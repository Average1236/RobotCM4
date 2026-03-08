#ifndef __ZOS_LOG_H__
#define __ZOS_LOG_H__

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/chrono.h>
#include <string_view>
// #include <source_location> // spdlog handles location if needed, but for simplicity we rely on macros or basic logging

namespace zos {

// Using structs with deduction guides to mimic original zos behavior if needed,
// but since we want to be compatible with function-like usage zos::log("..."), functions are easier.
// However, zos::log might be used as a type in some templates? Unlikely based on usage.

template <typename... Args>
inline void log(fmt::format_string<Args...> fmt, Args &&...args) {
    spdlog::info(fmt, std::forward<Args>(args)...);
}
inline void log(std::string_view msg) {
    spdlog::info("{}", msg);
}

template <typename... Args>
inline void info(fmt::format_string<Args...> fmt, Args &&...args) {
    spdlog::info(fmt, std::forward<Args>(args)...);
}
inline void info(std::string_view msg) {
    spdlog::info("{}", msg);
}

template <typename... Args>
inline void status(fmt::format_string<Args...> fmt, Args &&...args) {
    spdlog::info(fmt, std::forward<Args>(args)...); // Map status to info
}
inline void status(std::string_view msg) {
    spdlog::info("{}", msg);
}

template <typename... Args>
inline void warning(fmt::format_string<Args...> fmt, Args &&...args) {
    spdlog::warn(fmt, std::forward<Args>(args)...);
}
inline void warning(std::string_view msg) {
    spdlog::warn("{}", msg);
}

template <typename... Args>
inline void error(fmt::format_string<Args...> fmt, Args &&...args) {
    spdlog::error(fmt, std::forward<Args>(args)...);
}
inline void error(std::string_view msg) {
    spdlog::error("{}", msg);
}

} // namespace zos

#endif // __ZOS_LOG_H__
