#include "motion/cm4_motion_bridge.hpp"
#include "motion/linux_spi_transport.hpp"
#include "motion/runtime_config.hpp"
#include <yaml-cpp/yaml.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

namespace {
volatile std::sig_atomic_t running = 1;
void stop(int) { running = 0; }
motion::TimeUs nowUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
sockaddr_in endpoint(const char* host, int port) {
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port);
    if (::inet_pton(AF_INET, host, &a.sin_addr) != 1) throw std::runtime_error("invalid IPv4 address");
    return a;
}
void sendProto(int fd, const sockaddr_in& peer, const google::protobuf::MessageLite& p) {
    const auto bytes = p.SerializeAsString();
    ::sendto(fd, bytes.data(), bytes.size(), 0, reinterpret_cast<const sockaddr*>(&peer), sizeof(peer));
}
}

int main(int argc, char** argv) try {
    int command_port = 14234, feedback_port = 14134, id = 6, team = 1;
    const char* spi_device = "/dev/spidev1.0";
    const char* unix_path = nullptr;
    const char* config_path = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--help")) {
            std::puts("cm4_service [--config FILE] [--spi-device PATH | --unix-spi PATH] [--command-port N] [--feedback-port N] [--id N] [--team 1|2]");
            return 0;
        }
        if (i + 1 == argc) return 2;
        const char* key = argv[i++]; const char* value = argv[i];
        if (!std::strcmp(key, "--config")) config_path = value;
        else if (!std::strcmp(key, "--spi-device")) spi_device = value;
        else if (!std::strcmp(key, "--unix-spi")) unix_path = value;
        else if (!std::strcmp(key, "--command-port")) command_port = std::stoi(value);
        else if (!std::strcmp(key, "--feedback-port")) feedback_port = std::stoi(value);
        else if (!std::strcmp(key, "--id")) id = std::stoi(value);
        else if (!std::strcmp(key, "--team")) team = std::stoi(value);
        else return 2;
    }
    if (config_path) {
        const auto config = YAML::LoadFile(config_path);
        id = config["id"].as<int>(); team = config["team"].as<int>();
    }
    if (id < 0 || id >= 16 || (team != 1 && team != 2) || command_port <= 0 ||
        command_port > 65535 || feedback_port <= 0 || feedback_port > 65535) return 2;
    ::signal(SIGINT, stop); ::signal(SIGTERM, stop); ::signal(SIGPIPE, SIG_IGN);
    const int fd = ::socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    const auto bind_address = endpoint(unix_path ? "127.0.0.1" : "0.0.0.0", command_port);
    if (fd < 0 || ::bind(fd, reinterpret_cast<const sockaddr*>(&bind_address), sizeof(bind_address))) {
        std::perror("CM4 command socket"); return 1;
    }
    robot_cm4::Cm4MotionBridge bridge;
    bridge.configure(motion::runtimeMotionConfig(), static_cast<std::uint32_t>(nowUs()), nowUs());
    // Match execution tail extension is explicitly enabled by the shared-world
    // match session; the production default (env absent) keeps v3 bytes.
    const char* match_execution_env = std::getenv("KUN2_MUJOCO_MATCH_EXECUTION");
    const bool match_execution_enabled =
        match_execution_env != nullptr && std::strcmp(match_execution_env, "1") == 0;
    std::uint32_t match_world_session = 0;
    if (const char* session_env = std::getenv("KUN2_MUJOCO_MATCH_WORLD_SESSION")) {
        match_world_session =
            static_cast<std::uint32_t>(std::strtoul(session_env, nullptr, 10));
    }
    const bool use_hardware_spi = unix_path == nullptr;
    if (match_execution_enabled && use_hardware_spi) {
        bridge.setMatchExecutionEnabled(true, match_world_session, nowUs());
    }
    robot_cm4::UnixSocketSpiTransport unix_spi;
    robot_cm4::LinuxSpiTransport hardware_spi;
    if (!unix_path && !hardware_spi.openDevice(spi_device)) {
        std::perror("CM4 SPI device"); ::close(fd); return 1;
    }
    auto* spi = unix_path ? static_cast<robot_cm4::SpiTransport*>(&unix_spi) : &hardware_spi;
    auto feedback_peer = endpoint("127.0.0.1", feedback_port);
    const auto multicast_peer = endpoint("225.225.225.225", 13134);
    bool connected = !unix_path, have_peer = false;
    bool match_execution_ready_reported = false;
    motion::TimeUs next_multicast = 0;
    while (running) {
        const auto start = nowUs();
        for (int count = 0; count < 64; ++count) {
            char bytes[65536]; sockaddr_in sender{}; socklen_t length = sizeof(sender);
            const auto size = ::recvfrom(fd, bytes, sizeof(bytes), 0, reinterpret_cast<sockaddr*>(&sender), &length);
            if (size < 0) break;
            ZSS::New::Robot_Command command;
            if (!command.ParseFromArray(bytes, size) || command.robot_id() != id) continue;
            // Only the loopback simulator shares Athena's monotonic clock.
            const bool observation_only = unix_path && !command.has_motion_command() &&
                                          command.has_vision_observation();
            const auto received_us = nowUs();
            if (observation_only ? bridge.submitLocalVision(command.vision_observation(), received_us)
                                 : bridge.submitRobotCommand(command, received_us)) {
                feedback_peer.sin_addr = sender.sin_addr; have_peer = true;
            }
        }
        if (unix_path && !connected) {
            connected = unix_spi.connectTo(unix_path);
            if (connected) {
                timeval timeout{0, 40000};
                ::setsockopt(unix_spi.descriptor(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                // The capability handshake only starts once the SPI link exists,
                // so a slow session startup is not a false capability failure.
                if (match_execution_enabled) {
                    bridge.setMatchExecutionEnabled(true, match_world_session, nowUs());
                }
            }
        }
        if (connected) {
            const bool ok = bridge.exchange(*spi, nowUs());
            if (unix_path) connected = ok;
            if (ok && have_peer) {
                ZSS::New::Robot_Status status;
                status.set_robot_id(id); status.set_team(static_cast<ZSS::New::Team>(team));
                bridge.applyFeedbackToStatus(&status); sendProto(fd, feedback_peer, status);
            }
        }
        if (match_execution_enabled && bridge.matchExecutionEnabled()) {
            bridge.pollMatchExecution(nowUs());
            if (!match_execution_ready_reported && bridge.matchExecutionReady()) {
                // One-shot capability evidence consumed by the session launcher.
                std::fprintf(stderr,
                             "CM4: match_execution_ready team=%d id=%d world_session=%u "
                             "events=%u acks_forwarded=%u duplicates=%u old_generation=%u\n",
                             team, id, match_world_session,
                             bridge.executionEvents(), bridge.executionAcksForwarded(),
                             bridge.executionDuplicates(), bridge.executionOldGeneration());
                match_execution_ready_reported = true;
            }
        }
        if (match_execution_enabled && bridge.matchExecutionRejected()) {
            // Missing/mismatched capabilities must refuse automatic match play
            // instead of silently degrading to the v3 wire format.
            std::fprintf(stderr, "CM4: match_execution_not_supported\n");
            ::close(fd); return 1;
        }
        if (!unix_path && start >= next_multicast) {
            ZSS::New::Multicast_Status status; const auto feedback = bridge.lastFeedback();
            status.set_robot_id(id); status.set_team(static_cast<ZSS::New::Team>(team));
            status.set_battery(feedback.battery_voltage * 10);
            status.set_capacitance(feedback.capacitance_voltage * 10);
            status.set_have_imu(feedback.have_imu); status.set_infrared(feedback.infrared_age_ms);
            sendProto(fd, multicast_peer, status); next_multicast = start + 100000;
        }
        std::this_thread::sleep_until(std::chrono::steady_clock::time_point(std::chrono::microseconds(start + 10000)));
    }
    if (match_execution_enabled) {
        std::fprintf(stderr,
                     "CM4: match_execution events=%u acks_forwarded=%u duplicates=%u "
                     "old_generation=%u\n",
                     bridge.executionEvents(), bridge.executionAcksForwarded(),
                     bridge.executionDuplicates(), bridge.executionOldGeneration());
    }
    ::close(fd); return 0;
} catch (const std::exception& e) {
    std::fprintf(stderr, "CM4: %s\n", e.what()); return 1;
}
