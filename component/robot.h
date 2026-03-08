#ifndef ROBOT_H
#define ROBOT_H

#include "device.h"
#include "timer.h"
#include "wifi.h"
#include "parameter.h"
#include "matrix.h"
#include "config.h"

#include "robot_comm.pb.h"
#include "vision_detection.pb.h"
#include <yaml-cpp/yaml.h>
#include "zos/core.h"
#include "zss_cmd.pb.h"
#include <fstream>
#include <sstream>

static constexpr int COMM_TYPE_24L01 = 1;
static constexpr int COMM_TYPE_WIFI = 2;

static constexpr int CMD_TYPE_NONE = 0;
static constexpr int CMD_TYPE_WHEEL = 1;
static constexpr int CMD_TYPE_VEL = 2;
static constexpr int CMD_TYPE_POSE = 3;
static constexpr int CMD_TYPE_CHASE = 4;

class robot_
{
public:
    robot_();
    ~robot_() = default;

    uint8_t robot_id = config::robot_id;

    void send_status();
    void multicast_info();

    void control();

    void shoot();

    void device_run();

    double ms_since_last_cmd_pack();

    void stand();

    void update_imu();

    void update_expansion_board(const double dt);

    int debug_dt = 0;
    std::atomic<int> vision_err_count = 0;


private:
    void wifi_callback(const asio::ip::udp::endpoint &, const void *, size_t);
    void _wifi_vision(const asio::ip::udp::endpoint &, const void *, size_t);
    void _wifi_cmd_small(const asio::ip::udp::endpoint &, const void *, size_t);
    void cmd_callback(const zos::Data &);
    void _vision_cb(const zos::Data &);
    void cmd_small_cb(const zos::Data &);

    double get_vr(const double _dt, double fdb, bool have_fdb);
    uint16_t acc_r_set = 200;

    std::atomic<bool> imu_online;

    float Q = 200;
    float R = 0.01;

    float r = 10;
    float h = 0.01;

    typedef struct _filter_
    {
        std::vector<std::vector<float>> P_k;
        std::vector<std::vector<float>> X;

        float x1;
        float x2;
        float x1k;
        float x2k;
    } filter_;

    filter_ filter_X;
    filter_ filter_Y;

    std::atomic<int> vision2imu;

    std::atomic<bool> sendIsDebug = true;

    void getFdb(const double _dt, double imu, double vision, double &pose, double &vel, filter_ &filter);

    Vision_DetectionFrame pb_detectionFrame;

    void motion_planner(const double _dt, double fdb, bool have_fdb); // _dt in us

    void read_config_yaml();

    void save_pid();
    void save_id(int robot_id_new);

#ifndef motor_i2c_same_addr
    void set_pid();
#endif
    void save_freq(int nrf2401_freq_new);

    YAML::Node config_yaml;
    std::atomic_bool need_change_team = false;
    std::atomic_bool last_need_change_team = false;

    ZSS::New::Robot_Command pb_cmd;
    ZSS::New::Robot_Command_Mini pb_cmd_mini;

    std::atomic<double> _time_since_last_pack = 0; // ms

    uint8_t Robot_Status = 0, Last_Robot_Status = 0;
    uint8_t Kick_Count = 0;
    int8_t Left_Report_Package = 0;

    std::atomic_bool pid_busy = false; // TODO: ?

    int nrf2401_freq;

    std::vector<int> pid_pack = {0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> pid_real = {0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> vel_pack = {0, 0, 0, 0};
    std::vector<int16_t> all_vel_pack = {0, 0, 0};
    std::array<std::atomic<float>, 4> vel_read_temp = {0, 0, 0, 0};

    const double Vel_k2 = config::vel_ratio;
    double sin_angle[4] = {sin(config::car_angle_front), -sin(config::car_angle_front), -sin(config::car_angle_back), sin(config::car_angle_back)};
    double cos_angle[4] = {-cos(config::car_angle_front), -cos(config::car_angle_front), cos(config::car_angle_back), cos(config::car_angle_back)};

    float error = 0;
    float prev_error = 0;

    device_ device;

    zos::Data _cmd_data;
    zos::Data _vision_data;
    zos::Data _cmd_small_data;
    std::mutex _cmd_data_mutex;
    std::mutex _vision_data_mutex;
    std::mutex _cmd_small_data_mutex;
    zos::Subscriber<2> _cmd_subsciber;
    zos::Publisher _cmd_publisher;
    zos::Subscriber<2> _vision_subsciber;
    zos::Publisher _vision_publisher;
    zos::Subscriber<2> _cmd_small_subsciber;
    zos::Publisher _cmd_small_publisher;

    wifi_comm wifi;
    wifi_receive_ wifi_vision;
    wifi_receive_ wifi_cmd_small;
};

#endif
