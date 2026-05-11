#include "robot.h"
#include "robot_stm32.h"
#include "timer.h"
#include <asio.hpp>
#include <csignal>
#include <functional>
#include <iostream>

#define CLOCKID CLOCK_REALTIME

#define CM4_stm32

#ifdef CM4_stm32
using ActiveRobot = RobotStm32;
using ActiveRobotPtr = RobotStm32 *;
#else
using ActiveRobot = RobotCm;
using ActiveRobotPtr = RobotCm *;
#endif

ActiveRobotPtr g_robot = nullptr;

std::atomic<bool> g_stop = false;

namespace
{
void schedule_periodic(asio::io_context &io,
                       const std::chrono::steady_clock::duration period,
                       const std::function<void()> &task)
{
    auto timer = std::make_shared<asio::steady_timer>(io);
    auto next_tick = std::make_shared<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now() + period);
    auto runner = std::make_shared<std::function<void()>>();

    *runner = [timer, next_tick, period, task, runner]() {
        if (g_stop.load())
            return;

        task();

        auto now = std::chrono::steady_clock::now();
        do
        {
            *next_tick += period;
        } while (*next_tick <= now);

        timer->expires_at(*next_tick);
        timer->async_wait([runner](const std::error_code &ec) {
            if (ec || g_stop.load())
                return;
            (*runner)();
        });
    };

    timer->expires_at(*next_tick);
    timer->async_wait([runner](const std::error_code &ec) {
        if (ec || g_stop.load())
            return;
        (*runner)();
    });
}
}
#ifndef CM4_stm32
void callback_device(void)
{
    auto now_time = std::chrono::steady_clock::now();
    auto last_time = now_time - std::chrono::microseconds(1000000) / 1000.0;
    int dt = 0;
    while (!g_stop.load())
    {
        if (!g_robot)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        g_robot->device_run();
        now_time = std::chrono::steady_clock::now();
        dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
        last_time = now_time;
        // zos::log("dt = {}\n", dt);
        // std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
}
#endif

void callback_update_expansion_board(void)
{
    constexpr auto period = std::chrono::milliseconds(10); // 100 Hz
    auto last_time = std::chrono::steady_clock::now();
    auto next_tick = last_time;
    int dt = 0;
    while (!g_stop.load())
    {
        next_tick += period;
        std::this_thread::sleep_until(next_tick);

        auto now_time = std::chrono::steady_clock::now();
        dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
        last_time = now_time;
        if (g_robot)
            g_robot->update_expansion_board(dt / 1000.0);
    }
}

int main()
{
    ActiveRobot robot{};
    g_robot = &robot;
    g_stop = false;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    asio::io_context scheduler_io;
    asio::signal_set signals(scheduler_io, SIGINT, SIGTERM);
    signals.async_wait([&scheduler_io](const std::error_code &, int) {
        g_stop = true;
        scheduler_io.stop();
    });

#ifndef CM4_stm32
    std::jthread motor_thread(callback_device);
#endif
    std::jthread update_expansion_board_thread(callback_update_expansion_board);

#ifndef CM4_stm32
    schedule_periodic(scheduler_io,
                      std::chrono::nanoseconds(1000000000 / config::robot_freq),
                      []() {
                          if (g_robot)
                              g_robot->control();
                      });

    schedule_periodic(scheduler_io,
                      std::chrono::milliseconds(5),
                      []() {
                          if (g_robot)
                              g_robot->update_imu();
                      });
#endif

    schedule_periodic(scheduler_io,
                      std::chrono::nanoseconds(1000000000 / config::sendback_freq),
                      []() {
                          if (g_robot)
                              g_robot->send_status();
                      });

    schedule_periodic(scheduler_io,
                      std::chrono::nanoseconds(1000000000 / config::multicast_freq),
                      []() {
                          if (g_robot)
                              g_robot->multicast_info();
                      });

    schedule_periodic(scheduler_io,
                      std::chrono::nanoseconds(1000000000 / config::vision_freq),
                      []() {
                          if (!g_robot)
                              return;
                          std::scoped_lock lock{_vision_queue_mutex, _pVision_mutex};
                          if (!Vision_queue.empty())
                          {
                              pVision = Vision_queue.front();
                              Vision_queue.pop();
                              g_robot->vision_err_count = 0;
                          }
                          else
                          {
                              g_robot->vision_err_count += 1;
                          }
                      });

    schedule_periodic(scheduler_io,
                      std::chrono::nanoseconds(1000000000 / config::control_cmd_freq),
                      []() {
                          if (g_robot)
                              g_robot->shoot();
                      });

    schedule_periodic(scheduler_io,
                      std::chrono::milliseconds(100),
                      [&robot]() {
                          if (robot.ms_since_last_cmd_pack() > 100)
                          {
                              robot.stand();
                          }
                      });

    scheduler_io.run();

    g_stop = true;
    g_robot = nullptr;

    return 0;
}
