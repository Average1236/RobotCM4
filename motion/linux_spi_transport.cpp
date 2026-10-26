#include "linux_spi_transport.hpp"
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>
namespace robot_cm4 {
LinuxSpiTransport::~LinuxSpiTransport() { if (fd_ >= 0) ::close(fd_); }
bool LinuxSpiTransport::openDevice(const char* path, std::uint32_t speed_hz) {
    if (fd_ >= 0) ::close(fd_);
    fd_ = ::open(path, O_RDWR | O_CLOEXEC);
    if (fd_ < 0) return false;
    std::uint8_t mode = SPI_MODE_0, bits = 8;
    speed_hz_ = speed_hz;
    if (::ioctl(fd_, SPI_IOC_WR_MODE, &mode) < 0 ||
        ::ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ::ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz_) < 0) {
        ::close(fd_); fd_ = -1; return false;
    }
    return true;
}
bool LinuxSpiTransport::transfer(const std::uint8_t* tx, std::uint8_t* rx, std::size_t length) {
    if (fd_ < 0 || !tx || !rx || length != 256) return false;
    spi_ioc_transfer transfer{};
    transfer.tx_buf = reinterpret_cast<std::uintptr_t>(tx);
    transfer.rx_buf = reinterpret_cast<std::uintptr_t>(rx);
    transfer.len = length; transfer.speed_hz = speed_hz_; transfer.bits_per_word = 8;
    return ::ioctl(fd_, SPI_IOC_MESSAGE(1), &transfer) == static_cast<int>(length);
}
}
