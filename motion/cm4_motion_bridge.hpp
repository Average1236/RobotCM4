#ifndef ROBOT_CM4_CM4_MOTION_BRIDGE_HPP
#define ROBOT_CM4_CM4_MOTION_BRIDGE_HPP

#include "motion/protocol.hpp"
#include "motion/time_mapper.hpp"
#include "motion/types.hpp"
#include "motion/sequence.hpp"
#include "spi_transport.hpp"
#include "zss_cmd.pb.h"

#include <array>
#include <cstdint>
#include <mutex>
#include <atomic>

namespace robot_cm4 {

// CM4-side production bridge:
//   * protocol v2 protobuf (Kun2 <-> CM4) conversion,
//   * Kun2 clock -> CM4 local clock mapping,
//   * command snapshot (single producer / single SPI consumer),
//   * SPI v2 256-byte command frames with CRC,
//   * feedback frame decoding and Robot_Status feedback population.
//
// The bridge performs no motion closed loop of its own; it is only a time and
// transport boundary.  A missing or expired command is sent as an explicit
// disabled command so the STM32 safety state is well defined.
class Cm4MotionBridge {
public:
    void configure(const motion::MotionConfigV2& config, std::uint32_t session_id,
                   motion::TimeUs now_us);

    bool submitRobotCommand(const ZSS::New::Robot_Command& command,
                            motion::TimeUs receive_us);
    // Production CM4 vision path (local time base).  Hosts may also use the
    // embedded vision field of submitRobotCommand.
    bool submitLocalVision(const ZSS::New::VisionObservationV2& observation,
                           motion::TimeUs receive_us);
    bool submitVision(const motion::VisionObservationV2& observation,
                      motion::TimeUs now_us);

    bool buildCommandFrame(motion::TimeUs now_us, std::uint8_t* frame);
    bool buildConfigFrame(motion::TimeUs now_us, std::uint8_t* frame);
    bool buildFrame(motion::TimeUs now_us, std::uint8_t* frame);
    bool processFeedbackFrame(const std::uint8_t* frame, motion::TimeUs now_us);
    bool exchange(SpiTransport& transport, motion::TimeUs now_us);

    void applyFeedbackToStatus(ZSS::New::Robot_Status* status) const;

    motion::MotionCommandV2 lastCommand() const;
    motion::MotionFeedbackV2 lastFeedback() const;
    bool hasCommand() const;
    bool feedbackFresh(motion::TimeUs now_us) const;
    const motion::MotionConfigV2& config() const { return config_; }

    // --- Match execution tail extension (docs/MUJOCO_MATCH_EXECUTION.md) ---
    // Disabled by default: v3 encoding/decoding stays byte-identical.  When
    // enabled the bridge takes part in the startup capability handshake, adds
    // the capability block to command frames, forwards execution ACKs and
    // decodes execution feedback.  A session that cannot complete the
    // handshake reports the fixed `match_execution_not_supported` error.
    void setMatchExecutionEnabled(bool enabled, std::uint32_t world_session_id,
                                  motion::TimeUs now_us);
    bool matchExecutionEnabled() const { return match_execution_.enabled(); }
    bool matchExecutionReady() const { return match_execution_.ready(); }
    bool matchExecutionRejected() const { return match_execution_.rejected(); }
    const char* matchExecutionError() const { return match_execution_.error(); }
    void pollMatchExecution(motion::TimeUs now_us) { match_execution_.poll(now_us); }
    motion::KickExecutionFeedback lastExecutionFeedback() const;
    std::uint32_t executionFeedbackFrames() const;
    std::uint32_t executionEvents() const;
    std::uint32_t executionDuplicates() const;
    std::uint32_t executionOldGeneration() const;
    std::uint32_t executionAcksForwarded() const;

    std::uint32_t commandsSubmitted() const;
    std::uint32_t protocolErrors() const;
    std::uint32_t spiTimeouts() const;
    std::uint32_t visionAttached() const;
    std::uint32_t feedbackFrames() const;
    std::uint32_t configFramesSent() const { return config_frames_sent_; }
    std::uint32_t sessionId() const { return session_id_; }
    bool spiTimedOut() const { return spi_timed_out_; }

private:
    void buildDisabledCommand(motion::TimeUs now_us, motion::MotionCommandV2* command);
    void noteSpiTimeout(motion::TimeUs now_us);
    void observeFeedbackExtension(const motion::MatchExecutionExtension& extension,
                                  motion::TimeUs now_us);
    void pushPendingAck(const motion::KickExecutionAck& ack);
    bool popPendingAck(motion::KickExecutionAck* ack);

    motion::MotionConfigV2 config_;
    motion::TimeMapper command_time_map_;
    motion::TimeMapper feedback_time_map_;
    motion::SequenceGate command_sequences_;
    std::uint32_t session_id_ = 1;
    std::uint32_t disabled_sequence_ = 1;

    mutable std::mutex command_mutex_;
    motion::MotionCommandV2 command_;
    bool has_command_ = false;
    motion::TimeUs command_receive_us_ = 0;
    motion::VisionObservationV2 vision_;
    bool has_vision_ = false;
    std::uint32_t last_attached_vision_sequence_ = 0;
    motion::TimeUs last_attached_vision_capture_ = 0;
    std::array<motion::KickExecutionAck, motion::kMatchMaxPendingKickExecutions>
        pending_acks_{};
    std::size_t pending_ack_count_ = 0;

    mutable std::mutex feedback_mutex_;
    motion::MotionFeedbackV2 feedback_;
    bool has_feedback_ = false;
    motion::TimeUs last_feedback_receive_us_ = 0;
    motion::TimeUs last_feedback_frame_us_ = 0;
    bool spi_timed_out_ = true;

    // Match execution handshake / status-pulse state (feedback_mutex_).
    motion::MatchExecutionNegotiator match_execution_;
    std::uint32_t match_world_session_id_ = 0;
    std::uint32_t current_world_session_ = 0;
    motion::KickExecutionFeedback last_execution_feedback_;
    bool pulse_valid_ = false;
    std::uint32_t pulse_world_session_ = 0;
    std::uint32_t pulse_producer_session_ = 0;
    std::uint32_t pulse_kick_sequence_ = 0;
    motion::KickEventType pulse_event_type_ = motion::KickEventType::kNone;
    motion::TimeUs pulse_start_us_ = 0;
    std::int32_t flat_kick_ms_ = 0;
    std::int32_t chip_kick_ms_ = 0;
    std::uint32_t execution_feedback_frames_ = 0;
    std::uint32_t execution_events_ = 0;
    std::uint32_t execution_duplicates_ = 0;
    std::uint32_t execution_old_generation_ = 0;
    std::uint32_t execution_acks_forwarded_ = 0;

    std::uint32_t commands_submitted_ = 0;
    std::uint32_t protocol_errors_ = 0;
    std::uint32_t spi_timeouts_ = 0;
    std::uint32_t vision_attached_ = 0;
    std::uint32_t feedback_frames_ = 0;
    std::uint32_t config_frames_sent_ = 0;
    bool config_sent_ = false;
};

}  // namespace robot_cm4

#endif  // ROBOT_CM4_CM4_MOTION_BRIDGE_HPP
