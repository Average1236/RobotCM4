cmake_minimum_required(VERSION 3.16)
project(zos_core LANGUAGES CXX)

# Check if spdlog is available, otherwise fetching it or assuming header only
# On Debian 13, libspdlog-dev provides config via cmake
find_package(spdlog REQUIRED)
# asio is usually header only, often via package 'dev'
# find_package(asio REQUIRED) might fail if no config is installed
find_path(ASIO_INCLUDE_DIR asio.hpp)
if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find asio headers")
endif()

add_library(zos_core INTERFACE)

# We want users to include "zos/log.h", so we add current dir as include dir.
# Files are in ${CMAKE_CURRENT_SOURCE_DIR}/zos/*.h
target_include_directories(zos_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ASIO_INCLUDE_DIR}
)

target_link_libraries(zos_core INTERFACE
    spdlog::spdlog
    # asio::asio  <-- Removed because it is header only and we linked include dir
)
add_library(zos::zos_core ALIAS zos_core)

# Also need to link with pthread usually for asio/spdlog
find_package(Threads REQUIRED)
target_link_libraries(zos_core INTERFACE Threads::Threads)
