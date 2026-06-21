#ifndef DEVICE_BASE_H
#define DEVICE_BASE_H

#include <atomic>

struct ImuData
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
};

class DeviceBase
{
public:
    virtual ~DeviceBase() = default;
    virtual void set_dribbler(int vel) = 0;
    virtual float shoot_chip(bool kick_mode, float kick_discharge_time) = 0;
    virtual void buzzer_start() = 0;
    virtual void buzzer_thread() = 0;
    virtual void set_vel(float vel_x, float vel_y, float vel_w) {}
    
    // common structs
    ImuData imu_status;
    std::atomic<bool> have_imu{false};
    std::atomic<int> have_infrare{0};
    std::atomic<float> battery_vol{0};
    std::atomic<float> cap_vol{0};
    
    bool imu_online() { return have_imu.load(); }
};

#endif
