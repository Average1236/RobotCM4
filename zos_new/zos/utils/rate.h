#ifndef __ZOS_RATE_H__
#define __ZOS_RATE_H__

#include <chrono>
#include <thread>

namespace zos {

class Rate {
public:
    Rate(double frequency) : _period(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / frequency))) {
        _last_time = std::chrono::steady_clock::now();
    }
    
    void sleep() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - _last_time;
        if (elapsed < _period) {
            std::this_thread::sleep_for(_period - elapsed);
        }
        _last_time = std::chrono::steady_clock::now();
    }
    
    void reset() {
        _last_time = std::chrono::steady_clock::now();
    }
    
private:
    std::chrono::nanoseconds _period;
    std::chrono::steady_clock::time_point _last_time;
};

} // namespace zos

#endif // __ZOS_RATE_H__
