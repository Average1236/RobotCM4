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

#include "zos/utils/threadpool.h"
#include "zos/utils/rate.h"
#include "zos/log.h"
#include "config.h"

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

const int spi_length = 32;

class device_stm32_
{

    // device
public:
    device_stm32_();
    ~device_stm32_()
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
        int16_t vel[3];
        int16_t angle_pid[3];
        int16_t wheel_pid[3];
        bool use_imu;
    };

    struct __attribute__((packed)) stm32_to_CM4_spi
    {
        int16_t imu_data[9];
        bool infrare_flag;
        bool getBall;
        bool imu_online;
        int8_t battery_vol;
        int16_t cap_vol;
        int16_t wheel[4];

    };

    // dribbler
public:
    void set_dribbler(int dribble_val){drib_power = dribble_val;};
private:
    std::atomic<int> drib_power;

    // vel
public:
    void set_vel(float vel_x,float vel_y,float vel_w){vel_all[0]=vel_x;vel_all[1]=vel_y;vel_all[2]=vel_w;};
    std::array<std::atomic<float>,4> wheel = {0};
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
    float shoot_chip(bool kick_mode, float kick_discharge_time);

private:
};

#endif
