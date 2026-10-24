#include <cstdio>
#include "spi_transport.hpp"

#include <cerrno>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace robot_cm4 {

UnixSocketSpiTransport::~UnixSocketSpiTransport() {
    if (fd_ >= 0) ::close(fd_);
}

bool UnixSocketSpiTransport::connectTo(const char* path) {
    if (path == nullptr) return false;
    if (fd_ >= 0) ::close(fd_);
    fd_ = ::socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (fd_ < 0) return false;
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::snprintf(address.sun_path, sizeof(address.sun_path), "%s", path);
    if (::connect(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        ::close(fd_);
        fd_ = -1;
        return false;
    }
    return true;
}

bool UnixSocketSpiTransport::acceptOn(const char* path) {
    if (path == nullptr) return false;
    if (fd_ >= 0) ::close(fd_);
    const int server = ::socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (server < 0) return false;
    ::unlink(path);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::snprintf(address.sun_path, sizeof(address.sun_path), "%s", path);
    if (::bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        ::close(server);
        return false;
    }
    if (::listen(server, 1) != 0) {
        ::close(server);
        ::unlink(path);
        return false;
    }
    fd_ = ::accept(server, nullptr, nullptr);
    ::close(server);
    ::unlink(path);
    return fd_ >= 0;
}

bool UnixSocketSpiTransport::transfer(const std::uint8_t* tx, std::uint8_t* rx,
                                      std::size_t length) {
    if (fd_ < 0 || tx == nullptr || rx == nullptr) return false;
    ssize_t sent = 0;
    do {
        sent = ::send(fd_, tx, length, 0);
    } while (sent < 0 && errno == EINTR);
    if (sent != static_cast<ssize_t>(length)) return false;

    ssize_t received = 0;
    do {
        received = ::recv(fd_, rx, length, 0);
    } while (received < 0 && errno == EINTR);
    return received == static_cast<ssize_t>(length);
}

}  // namespace robot_cm4
