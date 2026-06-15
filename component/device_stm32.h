#ifndef DEVICE_STM32_H
#define DEVICE_STM32_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>
#include <cstring>
#include <atomic>

#include "zos/utils/threadpool.h"
#include "zos/utils/rate.h"
#include "zos/log.h"
#include "config.h"
#include "device_base.h"

#include <pigpio.h>
#include "mraa/common.hpp"
#include "mraa/uart.hpp"
#include "mraa/spi.hpp"
#include "mraa/gpio.hpp"

#define SHOOT_MODE 0
#define CHIP_MODE 1

#define GPIO_SHOOT 23
#define GPIO_CHIP 24
#define GPIO_BUZZER 12

#define GPIO_LED0 19
#define GPIO_LED1 20
#define GPIO_LED2 21

const int spi_length = 60;

class DeviceStm32 : public DeviceBase
{

    // device
public:
    DeviceStm32();
    ~DeviceStm32()
    {
        gpioTerminate();
    }

private:


    // nano or stm32
public:

    void read_write_stm32();
    mraa::Uart *stm32_uart;
    mraa::Spi *stm32_spi;
    mraa::Gpio* cs;

private:

    uint8_t tx_data_spi[spi_length] = {0};
    uint8_t rx_data_spi[spi_length] = {0};

    uint8_t tx_data_uart[8] = {0};
    uint8_t rx_data_uart[8] = {0};


    struct __attribute__((packed)) CM4_to_stm32_uart
    {
        
    };

    struct __attribute__((packed)) stm32_to_CM4_uart
    {
        
    };

    struct __attribute__((packed)) CM4_to_stm32_spi
    {
        uint8_t drib_power;
        uint8_t drib_mode;     // 1: torque, 2: speed, 3: hybrid
        int16_t vel[3];
        int16_t angle_pid[3];
        int16_t wheel_pid[3];
        bool use_imu;
        bool kick_mode;
        uint16_t kick_discharge_time;
        int16_t drib_velocity;  // turns/s * 100
        int16_t drib_torque_ff; // Nm * 1000
        int16_t xy_max_acc[2];   // vx, vy acceleration limits in mm/s^2
        int16_t xy_max_jerk[2];  // vx, vy jerk limits in (m/s^3) * 10
        int16_t xy_max_dec[2];   // vx, vy deceleration limits in mm/s^2
        int16_t yaw_max_vel;     // rad/s * 100
        int16_t yaw_max_acc;     // rad/s^2 * 100
        int16_t yaw_max_jerk;    // rad/s^3 * 100
    };

    static_assert(sizeof(CM4_to_stm32_spi) <= spi_length, "CM4_to_stm32_spi exceeds spi_length");

    struct __attribute__((packed)) stm32_to_CM4_spi
    {
        int16_t imu_data[9];
        bool infrare_flag;
        bool getBall;
        bool imu_online;
        int8_t battery_vol;
        int16_t cap_vol;
        int16_t wheel[4];
        int16_t odom_vel[2]; // vx, vy in mm/s
        int16_t xy_max_vel[2];   // vx, vy axis limits in mm/s
        int16_t xy_max_acc[2];   // vx, vy acceleration limits in mm/s^2
        int16_t xy_max_jerk[2];  // vx, vy jerk limits in (m/s^3) * 10
        int16_t xy_max_dec[2];   // vx, vy deceleration limits in mm/s^2
        int16_t yaw_max_vel;     // rad/s * 100
        int16_t yaw_max_acc;     // rad/s^2 * 100
        int16_t yaw_max_jerk;    // rad/s^3 * 100
        int16_t reserved;

    };

    static_assert(sizeof(stm32_to_CM4_spi) <= spi_length, "stm32_to_CM4_spi exceeds spi_length");

    // dribbler
public:
    void set_dribbler(int dribble_val){drib_power = dribble_val;};
    void set_dribbler(int dribble_val, float dribble_velocity, float dribble_torque_ff, int dribble_mode)
    {
        drib_power = dribble_val;
        drib_velocity = dribble_velocity;
        drib_torque_ff = dribble_torque_ff;
        drib_mode = dribble_mode;
    };
    void set_motion_limits(const std::array<float, 2>& xy_acc,
                           const std::array<float, 2>& xy_jerk,
                           const std::array<float, 2>& xy_dec,
                           float yaw_vel,
                           float yaw_acc,
                           float yaw_jerk)
    {
        for (int i = 0; i < 2; ++i)
        {
            xy_cmd_acc[i] = xy_acc[i];
            xy_cmd_jerk[i] = xy_jerk[i];
            xy_cmd_dec[i] = xy_dec[i];
        }
        yaw_cmd_vel = yaw_vel;
        yaw_cmd_acc = yaw_acc;
        yaw_cmd_jerk = yaw_jerk;
    };
private:
    std::atomic<int> drib_power;
    std::atomic<int> drib_mode = 3;
    std::atomic<float> drib_velocity = -50.0f;
    std::atomic<float> drib_torque_ff = 0.1f;
    std::array<std::atomic<float>,2> xy_cmd_acc = {7.0f, 7.0f};
    std::array<std::atomic<float>,2> xy_cmd_jerk = {1000.0f, 1000.0f};
    std::array<std::atomic<float>,2> xy_cmd_dec = {10.0f, 10.0f};
    std::atomic<float> yaw_cmd_vel = 25.0f;
    std::atomic<float> yaw_cmd_acc = 40.0f;
    std::atomic<float> yaw_cmd_jerk = 200.0f;

    // vel
public:
    void set_vel(float vel_x,float vel_y,float vel_w){vel_all[0]=vel_x;vel_all[1]=vel_y;vel_all[2]=vel_w;};
    std::array<std::atomic<float>,4> wheel = {0};
    std::atomic<float> odom_vx = 0.0f;
    std::atomic<float> odom_vy = 0.0f;
    std::array<std::atomic<float>,2> xy_max_vel = {0.0f, 0.0f};
    std::array<std::atomic<float>,2> xy_max_acc = {0.0f, 0.0f};
    std::array<std::atomic<float>,2> xy_max_jerk = {0.0f, 0.0f};
    std::array<std::atomic<float>,2> xy_max_dec = {0.0f, 0.0f};
    std::atomic<float> yaw_max_vel = 0.0f;
    std::atomic<float> yaw_max_acc = 0.0f;
    std::atomic<float> yaw_max_jerk = 0.0f;
private:
    std::array<std::atomic<float>,3> vel_all = {0};

    //imu
public:
    struct imu_data
    {
        float acc_x;
        float acc_y;
        float acc_z;
        float omega_x; // degree/s
        float omega_y;
        float omega_z;
        float theta_x;
        float theta_y;
        float theta_z;
    } imu_status;

    std::mutex imu_mutex;
    std::atomic<bool> have_imu;
    std::atomic<bool> use_imu;

    // other
public:
    std::atomic<int> have_infrare;
    std::atomic<float> battery_vol;
    std::atomic<float> cap_vol;
    void set_angle_pid(float kp ,float ki,float kd){angle_pid[0] = kp;angle_pid[1] = ki;angle_pid[2] = kd;};
    std::array<std::atomic<float>,3> angle_pid = {0};

    // buzzer
public:
    void buzzer_thread();

    void buzzer_start();

private:
    void buzzer_once(int freq);

    // shoot
public:
    float shoot_chip(bool kick_mode, float kick_discharge_time) {
        this->kick_mode.store(kick_mode);
        this->kick_discharge_time.store(kick_discharge_time);
        return (float)kick_mode;
    };

private:
    std::atomic<bool> kick_mode;
    std::atomic<float> kick_discharge_time;

private:
};

#endif
