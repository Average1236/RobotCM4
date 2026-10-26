#ifndef ROBOT_CM4_LINUX_SPI_TRANSPORT_HPP
#define ROBOT_CM4_LINUX_SPI_TRANSPORT_HPP
#include "spi_transport.hpp"
namespace robot_cm4 {
class LinuxSpiTransport final : public SpiTransport {
public:
    ~LinuxSpiTransport() override;
    LinuxSpiTransport() = default;
    LinuxSpiTransport(const LinuxSpiTransport&) = delete;
    LinuxSpiTransport& operator=(const LinuxSpiTransport&) = delete;
    bool openDevice(const char* path, std::uint32_t speed_hz = 1000000);
    bool transfer(const std::uint8_t* tx, std::uint8_t* rx, std::size_t length) override;
private:
    int fd_ = -1;
    std::uint32_t speed_hz_ = 1000000;
};
}
#endif
