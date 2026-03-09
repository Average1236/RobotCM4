#ifndef ROBOT_H
#define ROBOT_H

#include "device.h"
#include "robot_base.h"
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

static constexpr int CMD_TYPE_NONE = 0;
static constexpr int CMD_TYPE_WHEEL = 1;
static constexpr int CMD_TYPE_VEL = 2;
static constexpr int CMD_TYPE_POSE = 3;
static constexpr int CMD_TYPE_CHASE = 4;

class RobotCm : public RobotBase
{
public:
    RobotCm();
    ~RobotCm() = default;

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
    void wifi_vision_callback(const asio::ip::udp::endpoint &, const void *, size_t);
    void wifi_cmd_small_callback(const asio::ip::udp::endpoint &, const void *, size_t);
    void cmd_callback(const zos::Data &);
    void vision_callback(const zos::Data &);
    void cmd_small_callback(const zos::Data &);

    double get_vr(const double _dt, double fdb, bool have_fdb);
    uint16_t acc_r_set = 200;

    std::atomic<bool> imu_online;

    float q_ = 200;
    float r_measure_ = 0.01;

    float radius_ = 10;
    float time_step_ = 0.01;

    typedef struct _filter_
    {
        std::vector<std::vector<float>> P_k;
        std::vector<std::vector<float>> X;

        float x1;
        float x2;
        float x1k;
        float x2k;
    } filter_;

    filter_ filter_x_;
    filter_ filter_y_;

    std::atomic<int> vision_to_imu_;

    std::atomic<bool> send_is_debug_ = true;

    void get_fdb(const double _dt, double imu, double vision, double &pose, double &vel, filter_ &filter);

    Vision_DetectionFrame pb_detection_frame_;

    void motion_planner(const double _dt, double fdb, bool have_fdb); // _dt in us

    void read_config_yaml();

    void save_pid();
    void save_id(int id_new);
    void save_team(ZSS::New::Team team_new);

    void change_team(ZSS::New::Team team_new);
    void change_id(int id_new);

#ifndef motor_i2c_same_addr
    void set_pid();
#endif    

    YAML::Node config_yaml_;
    std::atomic_bool need_change_team = false;
    std::atomic_bool need_change_id = false;
    std::atomic<std::chrono::_V2::steady_clock::time_point> last_change_time;

    ZSS::New::Robot_Command pb_cmd_;
    ZSS::New::Robot_Command_Mini pb_cmd_mini_;

    std::atomic<double> _time_since_last_pack = 0; // ms

    uint8_t robot_status_flag_ = 0, last_robot_status_flag_ = 0;
    uint8_t kick_count_ = 0;
    int8_t left_report_package_ = 0;

    std::atomic_bool pid_busy = false; // TODO: ?

    std::vector<int> pid_pack_ = {0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> pid_real_ = {0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> vel_pack_ = {0, 0, 0, 0};
    std::vector<int16_t> all_vel_pack_ = {0, 0, 0};
    std::array<std::atomic<float>, 4> vel_read_temp_ = {0, 0, 0, 0};

    const double vel_k2_ = config::vel_ratio;
    double sin_angle_[4] = {sin(config::car_angle_front), -sin(config::car_angle_front), -sin(config::car_angle_back), sin(config::car_angle_back)};
    double cos_angle_[4] = {-cos(config::car_angle_front), -cos(config::car_angle_front), cos(config::car_angle_back), cos(config::car_angle_back)};

    float error_ = 0;
    float prev_error_ = 0;

    DeviceCm device_;

    zos::Data cmd_data_;
    zos::Data vision_data_;
    zos::Data cmd_small_data_;
    std::mutex cmd_data_mutex_;
    std::mutex vision_data_mutex_;
    std::mutex cmd_small_data_mutex_;
    zos::Subscriber<2> cmd_subscriber_;
    zos::Publisher cmd_publisher_;
    zos::Subscriber<2> vision_subscriber_;
    zos::Publisher vision_publisher_;
    zos::Subscriber<2> cmd_small_subscriber_;
    zos::Publisher cmd_small_publisher_;

    wifi_comm wifi_;
    wifi_receive_ wifi_vision_;
    wifi_receive_ wifi_cmd_small_;
};

#endif
