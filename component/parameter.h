#ifndef PARAMETER_H
#define PARAMETER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

#include <yaml-cpp/yaml.h>
#include "WorldDefine.h"

struct Robot_CMD
{
    int comm_type; // 1:24L01,2:WIFI
    int cmd_type;  // check zss_cmd.proto (1:CMD_WHEEL,2:CMD_VEL,3:CMD_POSE,4:CMD_CHASE)

    bool kick_en;              // if kick enabled
    bool kick_mode;            // chip:1  shoot:0
    float desire_power;        // speed(m/s) for flatkick or distance(m) for chip
    float kick_discharge_time; // us
    float drib_power;          // -1 ~ 1

    // cmd_wheel
    std::array<float, 4> motor_vel_target; // rad/s
    // cmd_vel
    float vx_target, vy_target, vr_target; // robot velocity: cm/s
    bool use_dir;
    //// use to store
    float vx_target_last, vy_target_last, vr_target_last;
    // cmd_pose
    float x_target;
    float y_target;
    float dir_target;
    float vx_target_trap, vy_target_trap, vr_target_trap;
    float acc_factor;
    float rot_acc_factor;
    PlayerCapabilityT capability;
    // cmd_chase
    //  TODO

    // old protocol
    uint8_t acc_set; // 加速度限制 16ms内合成加速度最大值，单位cm/s
    uint16_t acc_r_set;

    uint8_t DEC_FRAME;
    std::array<float, 4> pid_kp;
    std::array<float, 4> pid_ki;
    std::array<float, 4> pid_kd;
    bool imu_is_used;
    std::array<float,3> angle_pid;
    // store origin pb cmd
};

struct Robot_STATUS
{
    int id;
    int team; // 1 for blue,2 for yellow

    float robot_is_infrared;     // infrared detected time (ms) or -1
    float robot_is_boot_charged; // if capacitance charge finished (60v)
    float last_kick_time;        // ms [0 ~ 10000]
    float robot_is_shooted;      // ms
    float robot_is_chipped;      // ms

    float cap_vol;                      // (v)
    float bat_vol;                      // (v)
    std::array<float, 4> motor_encoder; // [-4096~4095]

    float imu_theta;               // suppose to be (-pi,pi]
    std::array<float, 6> imu_info; // TODO
    std::array<float, 12> imu_data;

    // 滤波值
    float robotX;
    float robotXd;
    float robotY;
    float robotYd;
    
    // uint8_t robot_status = 0, last_robot_status = 0;
};

struct Ball
{
    float vel_x;
    float vel_y;
    unsigned int area;
    float x;
    float y;
    float height;
    unsigned int ball_state;
    unsigned int last_touch;
    bool valid;
    float raw_x;
    float raw_y;
    float chip_predict_x;
    float chip_predict_y;
    float chip_best_predict_x;
    float chip_best_predict_y;
    float real_x;
    float real_y;
    float real_z;
    float chip_vel_x;
    float chip_vel_y;
    float chip_vel_z;
    float chip_second_pos_x;
    float chip_second_pos_y;
    float chip_fly_time;
    float chip_rest_fly_time;
    float raw_vel_x;
    float raw_vel_y;
};

struct Robot
{
    bool valid;
    unsigned int robot_id;
    float x;
    float y;
    float orientation;
    float vel_x;
    float vel_y;
    float rotate_vel;
    float accelerate_x;
    float accelerate_y;
    float raw_x;
    float raw_y;
    float raw_orientation;
    float raw_vel_x;
    float raw_vel_y;
    float raw_rotate_vel;
    // added by cxw
    float imu_dir;
    float imu_rotate_vel;
    float imu_bias;
};

class Vision
{
private:
    /* data */
    struct PlayerPoseT _our_robot_vision[16];
    struct PlayerPoseT _their_robot_vision[16];
    bool _updateFlag = false;

public:
    Vision(/* args */);
    ~Vision();

    struct Ball ball;

    bool isUpdated() { return _updateFlag; }
    void setUpdate() { _updateFlag = true; }
    void resetUpdate() { _updateFlag = false; }
    const struct Ball &getBallState() { return ball; }
    PlayerPoseT &ourRobot(int i) { return _our_robot_vision[i]; }
    PlayerPoseT &theirRobot(int i) { return _their_robot_vision[i]; }
};

// struct element_t
// {
//     element_t *next_p;
//     element_t *front_p;
//     Vision num;
// };

// typedef struct element_t element_;

// class queue_
// {

// public:
//     ~queue_(void);
//     void forwardInsert(Vision num);
//     void rearwardInsert(Vision num);
//     bool rearwardPop(Vision *num);
//     bool forwardPop(Vision *num);
//     bool getFirstElement(Vision *num);
//     bool getLastElement(Vision *num);
//     int getLength(void);
//     bool ifEmpty(void);

// private:
//     element_ *first_p = new element_;
//     element_ *now_p = new element_;
//     int length = 0;
// };

// extern std::atomic<Robot_STATUS> robot_status;
// extern std::atomic<Robot_CMD> robot_cmd;
// extern std::atomic<Vision> vision;

extern std::queue<Vision> Vision_queue;
extern Vision pVision;
extern Vision VisionTmp;
extern struct Robot_STATUS robot_status;
extern struct Robot_CMD robot_cmd;

extern std::mutex _robot_cmd_mutex;
extern std::mutex _robot_status_mutex;
extern std::mutex _pVision_mutex;
extern std::mutex _vision_queue_mutex;

#endif