#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>

#include "zos/utils/threadpool.h"
#include "zos/utils/rate.h"
#include "zos/log.h"
#include "config.h"

#include <pigpio.h>
#include "mraa/common.hpp"
#include "mraa/uart.hpp"

#define SHOOT_MODE 0
#define CHIP_MODE 1

#define UART_BUFF_SIZE 3
#define UART_TXDATA_SIZE 7
#define IMU_DATA_LENGTH 256

#define GPIO_SHOOT 23
#define GPIO_CHIP 24
#define GPIO_BUZZER 12

#define GPIO_LED0 19
#define GPIO_LED1 20
#define GPIO_LED2 21

// #define motor_i2c_same_addr


class device_
{

    // device
public:
    device_();
    ~device_()
    {
        gpioTerminate();
    }

private:
    std::mutex mutex_i2c;
    std::mutex mutex_uart;

    // nano or stm32
public:
    std::vector<int> read_nano();
    mraa::Uart *nano_uart;

    // std::mutex expansion_board_buff_mutex;
    std::array<std::atomic<int>, UART_BUFF_SIZE> expansion_board_buff;

private:
    // motor
public:
    void motor_run();
    void all_motor_run();
    int motors_detect();
    void set_motors_vel(std::vector<int> &vel_pack);
    void set_motors_pid(std::vector<int> &pid_pack);
    void set_all_motor_vel(std::vector<int16_t> &vel_pack);
    std::array<float, MAX_MOTOR> get_motors_vel();
    std::vector<int> get_motors_pid();

private:
    int device_num = MAX_MOTOR;
    std::vector<int> vel_encoder = {0, 0, 0, 0};
    std::array<std::atomic_int, MAX_MOTOR> vel_target_atomic;   // 0.1 rad/s
    std::array<std::atomic_int, MAX_MOTOR> vel_encoders_atomic; // 0.1 rad/s

#ifdef motor_i2c_same_addr
    uint8_t motors_i2c_addr;
    int motors_i2c_handle;
#else
    uint8_t motors_i2c_addr[4];
    int motors_i2c_handle[MAX_MOTOR];
#endif

#ifdef motor_i2c_same_addr
    typedef union _all_vel_
    {
        int16_t vel[3];
        uint8_t tx[UART_TXDATA_SIZE-1];
    } all_vel_;
    all_vel_ all_vel = {0};
    std::mutex vel_tx_mutex;
#endif

    // imu
public:
    struct imu_data
    {
        float acc_x;
        float acc_y;
        float acc_z;
        float T_degree;
        float omega_x; // degree/s
        float omega_y;
        float omega_z;
        float voltage;
        float theta_x;
        float theta_y;
        float theta_z;
        float version;
    } imu_status;
    int read_imu(mraa::Uart *uart);
    bool imu_online() { return have_imu; }
    void set_imu_config(mraa::Uart *uart);

    mraa::Uart *imu_uart;

private:
    bool sumcrc(uint8_t *data);
    int read_imu_raw(mraa::Uart *uart);
    uint8_t buffer[IMU_DATA_LENGTH] = {0};
    bool have_imu;
    int imu_err;

    // dribbler
public:
    void set_dribbler(int vel);

private:
    int dribbler_i2c_handle;

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
