cmake_minimum_required(VERSION 3.22)
project(cm4_motion LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Protobuf REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Threads REQUIRED)
add_executable(cm4_service main.cpp
    motion/cm4_motion_bridge.cpp motion/spi_transport.cpp motion/linux_spi_transport.cpp
    share/proto/zss_cmd.pb.cc share/proto/zss_cmd_type.pb.cc
    ../motion/src/protocol.cpp)
target_include_directories(cm4_service PRIVATE . motion share/proto ../motion/include)
target_link_libraries(cm4_service PRIVATE protobuf::libprotobuf yaml-cpp Threads::Threads)
target_compile_options(cm4_service PRIVATE -Wall -Wextra -Wpedantic)

# The locked protobuf shared library uses its NDEBUG metadata layout.
target_compile_definitions(cm4_service PRIVATE NDEBUG)
install(TARGETS cm4_service RUNTIME DESTINATION bin)
