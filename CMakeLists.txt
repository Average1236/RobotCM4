cmake_minimum_required(VERSION 3.16)


project(timer LANGUAGES CXX)

# Valgrind requires debug symbols and no ASan
# Ensure you build with -DCMAKE_BUILD_TYPE=Debug or uncomment the line below
# SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

set(LIBS pigpio)

find_package(Threads REQUIRED)
# find_package(fmt REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Protobuf REQUIRED)
# find_library(RF24 rf24 REQUIRED)
# message(STATUS "using RF24 library: ${RF24}")

# find_package(zos_core REQUIRED)
# message("zos_core_FOUND : " ${zos_core_FOUND})
# message("zos_core_INCLUDE_DIRS : " ${zos_core_INCLUDE_DIRS})
# message("zos_core_LIBRARIES : " ${zos_core_LIBRARIES})
add_subdirectory(zos_new)

include_directories(
    # ${pigpio_INCLUDE_DIRS}
    ${fmt_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${YAML_CPP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/share/proto
    # ${CMAKE_CURRENT_SOURCE_DIR}/share/zos/project/core/include
    # ${CMAKE_CURRENT_SOURCE_DIR}/share/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/component
    ${CMAKE_CURRENT_SOURCE_DIR}/control
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(LibMRAA /usr/local/lib/libmraa.so)

set(LIBS
    ${LIBS}
    ${LibMRAA}
    ${Protobuf_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
    atomic                # <--- 修改这里：去掉绝对路径，直接写库名
    Threads::Threads
    # fmt::fmt
    ${RF24}
    pigpio                # <--- 建议顺便加上这个，防止接下来的链接报错
)

file(GLOB SHARED_SOURCE_FILES
    ${SHARED_SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/share/proto/*.pb.cc
    # ${CMAKE_CURRENT_SOURCE_DIR}/control/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/share/utils/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/component/robot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/component/wifi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/component/timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/component/device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/component/parameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/control/CMmotion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/control/controlal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/control/geometry.cpp

)

file(GLOB TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp


)

foreach(test_file ${TEST_FILES})
    file(RELATIVE_PATH test_file_name ${CMAKE_CURRENT_SOURCE_DIR} ${test_file})
    string(REPLACE ".cpp" "" test_name ${test_file_name})
    string(REPLACE "test/" "" test_name ${test_name})
    message("test file: " ${test_file} " -- " ${test_name})

    message(${test_file_name})

    add_executable (${test_name} ${test_file} ${SHARED_SOURCE_FILES})
    target_link_libraries (${test_name} PRIVATE ${LIBS} zos::zos_core)
    target_compile_definitions(${test_name} PRIVATE CM4_VERSION)

endforeach()
