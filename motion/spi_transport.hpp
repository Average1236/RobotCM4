#ifndef ROBOT_CM4_SPI_TRANSPORT_HPP
#define ROBOT_CM4_SPI_TRANSPORT_HPP

#include <cstddef>
#include <cstdint>

namespace robot_cm4 {

// Hardware boundary used by the CM4 production bridge.  The real adapter wraps
// mraa::Spi; FakeSpiTransport connects the host runner to the STM32 host core
// without bypassing the byte-level frame format.
class SpiTransport {
public:
    virtual ~SpiTransport() = default;
    virtual bool transfer(const std::uint8_t* tx, std::uint8_t* rx, std::size_t length) = 0;
};

class FakeSpiTransport final : public SpiTransport {
public:
    using Loopback = bool (*)(void* context, const std::uint8_t* tx, std::uint8_t* rx,
                              std::size_t length);

    FakeSpiTransport(void* context, Loopback loopback)
        : context_(context), loopback_(loopback) {}

    bool transfer(const std::uint8_t* tx, std::uint8_t* rx,
                  std::size_t length) override {
        if (loopback_ == nullptr) return false;
        return loopback_(context_, tx, rx, length);
    }

private:
    void* context_ = nullptr;
    Loopback loopback_ = nullptr;
};

// Optional byte-pipe transport for the two-process integration test.
class UnixSocketSpiTransport final : public SpiTransport {
public:
    UnixSocketSpiTransport() = default;
    ~UnixSocketSpiTransport() override;
    UnixSocketSpiTransport(const UnixSocketSpiTransport&) = delete;
    UnixSocketSpiTransport& operator=(const UnixSocketSpiTransport&) = delete;

    bool connectTo(const char* path);
    bool acceptOn(const char* path);
    bool transfer(const std::uint8_t* tx, std::uint8_t* rx,
                  std::size_t length) override;

    int descriptor() const { return fd_; }

private:
    int fd_ = -1;
};

}  // namespace robot_cm4

#endif  // ROBOT_CM4_SPI_TRANSPORT_HPP
