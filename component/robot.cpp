#include "robot.h"

#define need_store_data

using PB = ZSS::New::Robot_Command;

auto now_time = std::chrono::steady_clock::now();
auto last_time = now_time - std::chrono::microseconds(1000000) / config::robot_freq;

// now_time = std::chrono::steady_clock::now();
// dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
// last_time = now_time;

std::atomic<double> visionX_raw = 0;
std::atomic<double> visionY_raw = 0;
std::atomic<double> velX_raw = 0;
std::atomic<double> velY_raw = 0;

std::atomic<bool> initFinish = false;

#ifdef need_store_data
std::string logcsv = "/home/pi/log.csv";
#endif

RobotCm::RobotCm() : device_{},
                   wifi_(std::bind(&RobotCm::wifi_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
                   wifi_vision_(config::receive_port_vision,std::bind(&RobotCm::wifi_vision_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
                   wifi_cmd_small_(config::receive_port_cmd_small, std::bind(&RobotCm::wifi_cmd_small_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
                   cmd_subscriber_("cmd", std::bind(&RobotCm::cmd_callback, this, std::placeholders::_1)),
                   cmd_publisher_("cmd"),
                   vision_subscriber_("vision", std::bind(&RobotCm::vision_callback, this, std::placeholders::_1)),
                   vision_publisher_("vision"),
                   cmd_small_subscriber_("cmd_small", std::bind(&RobotCm::cmd_small_callback, this, std::placeholders::_1)),
                   cmd_small_publisher_("cmd_small")
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    float visionX = -10000;
    float visionY = -10000;

    {
        std::scoped_lock lock{_robot_cmd_mutex};
        robot_cmd.motor_vel_target[0] = 0;
        robot_cmd.motor_vel_target[1] = 0;
        robot_cmd.motor_vel_target[2] = 0;
        robot_cmd.motor_vel_target[3] = 0;

        robot_cmd.use_dir = 0;
        robot_cmd.vx_target_last = 0;
        robot_cmd.vy_target_last = 0;
        robot_cmd.vr_target_last = 0;
        robot_cmd.x_target = 0;
        robot_cmd.y_target = 0;
        robot_cmd.dir_target = 0;
        robot_cmd.vx_target_trap = 0;
        robot_cmd.vy_target_trap = 0;
        robot_cmd.vr_target_trap = 0;
        robot_cmd.acc_factor = 0;
        robot_cmd.rot_acc_factor = 0;
        robot_cmd.acc_set = 150; // 加��度限制 16ms内合成加速度朢�大��，单位cm/s
        robot_cmd.acc_r_set = 200;

        robot_cmd.DEC_FRAME = 0;
        // robot_cmd.pid_kp[0] = 0;
        // robot_cmd.pid_kp[1] = 0;
        // robot_cmd.pid_kp[2] = 0;
        // robot_cmd.pid_kp[3] = 0;
        // robot_cmd.pid_ki[0] = 0;
        // robot_cmd.pid_ki[1] = 0;
        // robot_cmd.pid_ki[2] = 0;
        // robot_cmd.pid_ki[3] = 0;
        // robot_cmd.pid_kd[0] = 0;
        // robot_cmd.pid_kd[1] = 0;
        // robot_cmd.pid_kd[2] = 0;
        // robot_cmd.pid_kd[3] = 0;
        robot_cmd.imu_is_used = 0;
        robot_cmd.angle_pid[0] = 6.5;
        robot_cmd.angle_pid[1] = 0;
        robot_cmd.angle_pid[2] = 0.5;
    }
    {
        std::scoped_lock lock{_robot_status_mutex};
        robot_status.robot_is_infrared = -1;     // infrared detected time (ms) or -1
        robot_status.robot_is_boot_charged;      // if capacitance charge finished (60v)
        robot_status.last_kick_time = 100.0;   // ms [0 ~ 10000]
        robot_status.robot_is_shooted = 100.0; // ms
        robot_status.robot_is_chipped = 100.0; // ms

        robot_status.motor_encoder[0] = 0; // [-4096~4095]
        robot_status.motor_encoder[1] = 0; // [-4096~4095]
        robot_status.motor_encoder[2] = 0; // [-4096~4095]
        robot_status.motor_encoder[3] = 0; // [-4096~4095]

        robot_status.robotX = visionX / 1000;
        robot_status.robotX = visionY / 1000;
    }

    filter_x_.P_k = matrix::creatMatrix<float>(3, 3, 0);
    filter_x_.P_k[1][1] = q_ * ((1.0 / config::robot_freq) / 2) * ((1.0 / config::robot_freq) / 2);
    filter_x_.P_k[2][1] = q_ * ((1.0 / config::robot_freq) / 2);
    filter_x_.P_k[1][2] = q_ * ((1.0 / config::robot_freq) / 2);
    filter_x_.P_k[2][2] = q_ * 1;

    filter_x_.X = matrix::creatMatrix<float>(3, 1, 0);
    filter_x_.X[0][0] = visionX / 1000;

    filter_x_.x1 = visionX / 1000;
    filter_x_.x2 = visionX / 1000;
    filter_x_.x1k = visionX / 1000;
    filter_x_.x2k = visionX / 1000;

    filter_y_.P_k = matrix::creatMatrix<float>(3, 3, 0);
    filter_y_.P_k[1][1] = q_ * ((1.0 / config::robot_freq) / 2) * ((1.0 / config::robot_freq) / 2);
    filter_y_.P_k[2][1] = q_ * ((1.0 / config::robot_freq) / 2);
    filter_y_.P_k[1][2] = q_ * ((1.0 / config::robot_freq) / 2);
    filter_y_.P_k[2][2] = q_ * 1;

    filter_y_.X = matrix::creatMatrix<float>(3, 1, 0);
    filter_y_.X[0][0] = visionY / 1000;

    filter_y_.x1 = visionY / 1000;
    filter_y_.x2 = visionY / 1000;
    filter_y_.x1k = visionY / 1000;
    filter_y_.x2k = visionY / 1000;

    read_config_yaml();

#if !defined(motor_i2c_same_addr) && !defined(CM4_stm32)
    int motors_num = device_.motors_detect();
    if (motors_num == 0)
    {
        zos::log("NO motor detected\n");
    }
    else
    {
        zos::log("{} motor detected!\n", motors_num);
    }
    set_pid();

#endif

    
    {
        std::scoped_lock lock{_robot_status_mutex};
        if (robot_status.team == 1)
        {
            vision_to_imu_ = 1;
        }
        else if (robot_status.team == 2)
        {
            vision_to_imu_ = -1;
        }
        else
        {
            vision_to_imu_ = 0;
        }
    }

#ifdef need_store_data
    std::ofstream fileCsv("/home/pi/log.csv");
    if (!fileCsv.is_open())
    {
        zos::log("file open false\n");
    }

    fileCsv << "open" << std::endl;
    fileCsv.close();
#endif

    initFinish = true;
}

unsigned long receive_count = 0;
void RobotCm::wifi_callback(const asio::ip::udp::endpoint &ep, const void *ptr, size_t size)
{
    wifi_.set_master_ip(ep.address().to_string());
    {
        std::scoped_lock lock{cmd_data_mutex_};
        cmd_data_.resize(size);
        cmd_data_.store(ptr, size);
        // zos::log("cmd_size,{}\n",size);
        // zos::log("cmd_receive_count,{}\n",++receive_count);
        // zos::log("cmd_ip,{}\n",ep.address().to_string());
    }

    cmd_publisher_.publish();
}

void RobotCm::wifi_vision_callback(const asio::ip::udp::endpoint &ep, const void *ptr, size_t size)
{
    {
        std::scoped_lock lock{vision_data_mutex_};
        vision_data_.resize(size);
        vision_data_.store(ptr, size);
    }
    vision_publisher_.publish();
}

void RobotCm::wifi_cmd_small_callback(const asio::ip::udp::endpoint &ep, const void *ptr, size_t size)
{
    {
        std::scoped_lock lock{cmd_small_data_mutex_};
        cmd_small_data_.resize(size);
        cmd_small_data_.store(ptr, size);
    }
    cmd_small_publisher_.publish();
}

void RobotCm::cmd_callback(const zos::Data &data)
{

    if (initFinish)
    {
        // zos::log("cmd_callback\n");
        std::scoped_lock lock{cmd_data_mutex_};
        auto res = pb_cmd_.ParseFromArray(cmd_data_.data(), cmd_data_.size());
        if (res)
        {
            auto &_p = pb_cmd_;
            std::scoped_lock lock{_robot_cmd_mutex};
            auto &_c = robot_cmd;
            _time_since_last_pack.store(0);
            // zos::log("{}\n", _p.DebugString());
            //  TODO update
            
            // _c.id = _p.robot_id(); // maybe need to check if id correct
            _c.kick_en = (_p.kick_mode() != PB::NONE);
            _c.kick_mode = (_p.kick_mode() == PB::CHIP);
            // _c.desire_power = _p.desire_power();
            _c.kick_discharge_time = _p.kick_discharge_time();
            _c.drib_power = _p.dribble_spin();
            // _c.cmd_type = int(_p.cmd_type());
            // switch (_p.cmd_type())
            // {
            //     case PB::CMD_WHEEL:
            //     {
            //         _c.motor_vel_target[0] = _p.cmd_wheel().wheel1();
            //         _c.motor_vel_target[1] = _p.cmd_wheel().wheel2();
            //         _c.motor_vel_target[2] = _p.cmd_wheel().wheel3();
            //         _c.motor_vel_target[3] = _p.cmd_wheel().wheel4();
            //         break;
            //     }
            //     case PB::CMD_VEL:
            //     {
            //         _c.vx_target = _p.cmd_vel().velocity_x();
            //         _c.vy_target = _p.cmd_vel().velocity_y();
            //         _c.use_dir = _p.cmd_vel().use_imu();
            //         // _c.vr_target = _c.use_dir ? _p.cmd_vel().imu_theta() : _p.cmd_vel().velocity_r();
            //         _c.vr_target = _p.cmd_vel().velocity_r();
            //         zos::log("vel,{},{},{}\n",_c.vx_target,_c.vy_target,_c.vr_target);
            //         break;
            //     }
            //     default:
            //     {
            //         // zos::error("current cmd_type not supported:{}\n", _p.cmd_type());
            //         break;
            //     }
            // }
            _c.vx_target = ((float)(int32_t)_p.cmd_vel().velocity_x())/1000.0;
            _c.vy_target = ((float)(int32_t)_p.cmd_vel().velocity_y())/1000.0;
            _c.use_dir = _p.cmd_vel().use_imu();
            _c.vr_target = _c.use_dir ? ((float)(int32_t)_p.cmd_vel().imu_theta())/1000.0 : ((float)(int32_t)_p.cmd_vel().velocity_r())/1000.0;
            // _c.vr_target = _p.cmd_vel().velocity_r();
            // zos::log("vel,{},{},{}\n",_c.vx_target,_c.vy_target,_c.vr_target);
            if (_p.angle_pid_size() >= 3) {
                _c.angle_pid[0] = ((float)(int32_t)_p.angle_pid(0)/1000.0);
                _c.angle_pid[1] = ((float)(int32_t)_p.angle_pid(1)/1000.0);
                _c.angle_pid[2] = ((float)(int32_t)_p.angle_pid(2)/1000.0);
            }
            send_is_debug_ = _p.isdebug();

            last_need_change_team = (need_change_team)?true:false;
            need_change_team = _p.need_change_team();

            if(last_need_change_team == false && need_change_team == true)
            {
                Robot_STATUS _s;
                {
                    std::scoped_lock lock{_robot_status_mutex};
                    robot_status.team = (robot_status.team - 1)?1:2;
                    _s = robot_status;
                }
                save_freq((_s.team - 1)?6:8);
                // zos::log("{}\n",_s.team);
            }

            now_time = std::chrono::steady_clock::now();
            debug_dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
            last_time = now_time;
            
            // zos::log("debug,{}\n",debug_dt);

            // _c.angle_pid[0] = 4;
            // _c.angle_pid[1] = 3;
            // _c.angle_pid[2] = 0.5;
            // pid_pack_[0]=_p.kp(0);
            // pid_pack_[1]=_p.ki(0);
            // pid_pack_[2]=_p.kp(1);
            // pid_pack_[3]=_p.ki(1);
            // pid_pack_[4]=_p.kp(2);
            // pid_pack_[5]=_p.ki(2);
            // pid_pack_[6]=_p.kp(3);
            // pid_pack_[7]=_p.ki(3);
            // for(int i=0;i<8;i++){
            //     zos::status("------------{}------------\n",pid_pack_[i]);
            // }
            // _c.imu_is_used = _p.use_pid();
        }
    }
}

void RobotCm::update_imu()
{
    // std::vector<int> nano_pack = device_.read_nano();
    int length = device_.read_imu(device_.imu_uart);
    std::array<float, 12> imu_tmp = {0};
    imu_tmp[0] = device_.imu_status.acc_x;
    imu_tmp[1] = device_.imu_status.acc_y;
    imu_tmp[2] = device_.imu_status.acc_z;
    imu_tmp[3] = device_.imu_status.T_degree;
    imu_tmp[4] = device_.imu_status.omega_x;
    imu_tmp[5] = device_.imu_status.omega_y;
    imu_tmp[6] = device_.imu_status.omega_z;
    imu_tmp[7] = device_.imu_status.voltage;
    imu_tmp[8] = device_.imu_status.theta_x;
    imu_tmp[9] = device_.imu_status.theta_y;
    imu_tmp[10] = device_.imu_status.theta_z;
    imu_tmp[11] = device_.imu_status.version;

    imu_online = device_.imu_online();
    // imu_online = false;
    {
        std::scoped_lock lock{_robot_status_mutex};
        auto &_s = robot_status;
        // std::scoped_lock lock{_imu_queue_mutex};
        // imu_queue.push(imu_tmp);
    //     // zos::status("i----------------mu_data num: {}-----------------\n", length);
        _s.imu_data[0] = device_.imu_status.acc_x;
        _s.imu_data[1] = device_.imu_status.acc_y;
        _s.imu_data[2] = device_.imu_status.acc_z;
        _s.imu_data[3] = device_.imu_status.T_degree;
        _s.imu_data[4] = device_.imu_status.omega_x;
        _s.imu_data[5] = device_.imu_status.omega_y;
        _s.imu_data[6] = device_.imu_status.omega_z;
        _s.imu_data[7] = device_.imu_status.voltage;
        _s.imu_data[8] = device_.imu_status.theta_x;
        _s.imu_data[9] = device_.imu_status.theta_y;
        _s.imu_data[10] = device_.imu_status.theta_z;
        _s.imu_data[11] = device_.imu_status.version;
    }
}

void RobotCm::update_expansion_board(const double dt)
{

#ifndef CM4_stm32
    std::vector<int> expansion_board_pack = device_.read_nano();
#else
    std::vector<int> expansion_board_pack = device_.read_write_stm32();
#endif
    
    {
        std::scoped_lock lock{_robot_status_mutex};
        auto &_s = robot_status;
        // Arduino nano外接板 or stm32 拓展板获取一部分信息
        if (expansion_board_pack[0] == 0xaa)
        {
            int freq_temp = expansion_board_pack[1];
            int id_temp = expansion_board_pack[2];
            if (id_temp != _s.id)
            {
                _s.id = id_temp;
                save_id(id_temp);
                device_.buzzer_thread();
                zos::warning("change num: {}\n", id_temp);
            }
            if (freq_temp == 6 && _s.team != 2)
            {
                _s.team = 2;
                save_freq(freq_temp);
                device_.buzzer_thread();
            }
            else if (freq_temp == 8 && _s.team != 1)
            {
                _s.team = 1;
                save_freq(freq_temp);
                device_.buzzer_thread();
            }
        }
        else if ((expansion_board_pack[0] & 0xfa) == 0xfa)
        {
            // zos::status("expansion_board pack 0: {:#}\n", expansion_board_pack[0]);
            if ((expansion_board_pack[0] & 0x01))
            {
                if (_s.robot_is_infrared > 0)
                {
                    _s.robot_is_infrared = std::min(_s.robot_is_infrared + dt, 10000.0);
                }
                else
                {
                    _s.robot_is_infrared = 1;
                }
                // zos::status("robot_is_infrared: {}ms\n", _s.robot_is_infrared);
            }
            else if (_s.robot_is_infrared < 0)
            {
                _s.robot_is_infrared = std::max(_s.robot_is_infrared - dt, -10000.0);
            }
            else
            {
                _s.robot_is_infrared = -1;
            }
            _s.cap_vol = expansion_board_pack[1];
            _s.bat_vol = expansion_board_pack[2] / 10.0;
            // zos::status("infrare: {}, cap_vol : {}, bat_vol: {}\n", _s.robot_is_infrared, _s.cap_vol, _s.bat_vol);
        }

        // motors will be updated in control thread;
        for (int i = 0; i < MAX_MOTOR; i++)
        {
            _s.motor_encoder[i] = vel_read_temp_[i];
        }
        // zos::warning("motor get : {},{},{},{}\n", _s.motor_encoder[0], _s.motor_encoder[1], _s.motor_encoder[2], _s.motor_encoder[3]);
        //  kick status will be reset to 0 in control thread (no closed loop check)
        _s.last_kick_time = std::min(_s.last_kick_time + dt, 10000.0);
        _s.robot_is_shooted = std::min(_s.robot_is_shooted + dt, 100.0);
        _s.robot_is_chipped = std::min(_s.robot_is_chipped + dt, 100.0);
    }
}

void RobotCm::send_status()
{
    zos::Data data;
    ZSS::New::Robot_Status pb_status;

    // update_status(1000.0 / config::sendback_freq);

    if (wifi_.get_master_ip() == "")
    {
	    // zos::log("in send_status no send\n");
        return;
    }

    {        
        if(send_is_debug_)
        {
            // zos::log("send_ip,{}\n",wifi_.get_master_ip());
            std::scoped_lock lock{_robot_status_mutex, _pVision_mutex};
            auto &_s = robot_status;
            auto &_v = pVision;
            pb_status.set_robot_id(_s.id);
            pb_status.set_infrared((int32_t)_s.robot_is_infrared);
            // zos::status("id: {}, team: {}\n", _s.id, _s.team);
            // zos::status("send infrare status: {}\n", _s.robot_is_infrared);
            pb_status.set_flat_kick((int32_t)_s.robot_is_shooted);
            pb_status.set_chip_kick((int32_t)_s.robot_is_chipped);
            pb_status.set_battery((int32_t)(_s.bat_vol*10.0));
            pb_status.set_capacitance((int32_t)(_s.cap_vol*10.0));
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[0]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[1]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[2]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[3]);
            pb_status.set_team(ZSS::New::Team(_s.team));
            pb_status.add_imu_data(_s.imu_data[0]);
            pb_status.add_imu_data(_s.imu_data[1]);
            pb_status.add_imu_data(_s.imu_data[2]);
            pb_status.add_imu_data(_s.imu_data[3]);
            pb_status.add_imu_data(_s.imu_data[4]);
            pb_status.add_imu_data(_s.imu_data[5]);
            pb_status.add_imu_data(_s.imu_data[6]);
            pb_status.add_imu_data(_s.imu_data[7]);
            pb_status.add_imu_data(_s.imu_data[8]);
            pb_status.add_imu_data(_s.imu_data[9]);
            pb_status.add_imu_data(_s.imu_data[10]);
            pb_status.add_imu_data(_s.imu_data[11]);
            pb_status.add_real_pose(_s.robotX);
            pb_status.add_real_pose(_s.robotXd);
            pb_status.add_real_pose(_s.robotY);
            pb_status.add_real_pose(_s.robotYd);
            // pb_status.set_imu_theta_z((int32_t)(_s.imu_data[10]*100));
            // pb_status.set_imu_omega_z((int32_t)(_s.imu_data[6]*100));
            pb_status.set_have_imu(imu_online);
        }
        else
        {
            // zos::log("send_ip,{}\n",wifi_.get_master_ip());
            std::scoped_lock lock{_robot_status_mutex, _pVision_mutex};
            auto &_s = robot_status;
            auto &_v = pVision;
            pb_status.set_robot_id(_s.id);
            int32_t send_is_infrared = -1;
            if (_s.robot_is_infrared > 100)
            {
                send_is_infrared = 100;
            }
            else if (_s.robot_is_infrared < -100)
            {
                send_is_infrared = -100;
            }
            else 
            {
                send_is_infrared = _s.robot_is_infrared;
            }
            pb_status.set_infrared(send_is_infrared);
            // zos::status("id: {}, team: {}\n", _s.id, _s.team);
            // zos::status("send infrare status: {}\n", _s.robot_is_infrared);
            pb_status.set_flat_kick((int32_t)_s.robot_is_shooted);
            pb_status.set_chip_kick((int32_t)_s.robot_is_chipped);
            // pb_status.set_battery(_s.bat_vol);
            // pb_status.set_capacitance(_s.cap_vol);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[0]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[1]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[2]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[3]);
            pb_status.set_team(ZSS::New::Team(_s.team));
            // pb_status.add_imu_data(_s.imu_data[0]);
            // pb_status.add_imu_data(_s.imu_data[1]);
            // pb_status.add_imu_data(_s.imu_data[2]);
            // pb_status.add_imu_data(_s.imu_data[3]);
            // pb_status.add_imu_data(_s.imu_data[4]);
            // pb_status.add_imu_data(_s.imu_data[5]);
            // pb_status.add_imu_data(_s.imu_data[6]);
            // pb_status.add_imu_data(_s.imu_data[7]);
            // pb_status.add_imu_data(_s.imu_data[8]);
            // pb_status.add_imu_data(_s.imu_data[9]);
            // pb_status.add_imu_data(_s.imu_data[10]);
            // pb_status.add_imu_data(_s.imu_data[11]);
            // pb_status.add_real_pose(_s.robotX);
            // pb_status.add_real_pose(_s.robotXd);
            // pb_status.add_real_pose(_s.robotY);
            // pb_status.add_real_pose(_s.robotYd);
            pb_status.set_imu_theta_z((int32_t)(_s.imu_data[10]*100));
            pb_status.set_imu_omega_z((int32_t)(_s.imu_data[6]*100));
            pb_status.set_have_imu(imu_online);
        }


    }
    auto size = pb_status.ByteSizeLong();
    data.resize(size);
    pb_status.SerializeToArray(data.ptr(), size);
    wifi_.udp_sender_point(data.data(), size);
    // zos::log("send_size,{}\n",size);
    pb_status.Clear();
}

void RobotCm::multicast_info()
{

    zos::Data data;
    ZSS::New::Multicast_Status mc_status;
    Robot_STATUS _s;

    _time_since_last_pack.store(_time_since_last_pack.load() + 1000.0 / config::multicast_freq); // ms

    {
        std::scoped_lock lock{_robot_status_mutex};
        _s = robot_status;
    }

    // mc_status.set_ip(wifi_.get_ip()); 
    std::string ip = wifi_.get_ip();
    size_t last_dot = ip.rfind('.');
    int ip_value = -1;
    if (last_dot != std::string::npos) {
        // 截取末尾部分
        std::string last_part = ip.substr(last_dot + 1);
        
        // 转换为int
        ip_value = std::stoi(last_part);
    } 
    mc_status.set_ip(ip_value);
    // mc_status.set_uuid("0000-0000-0000-0000");
    mc_status.set_team(ZSS::New::Team(_s.team));
    mc_status.set_robot_id(_s.id); // ROBOT_ID change
    mc_status.set_battery((int32_t)(_s.bat_vol*10));
    mc_status.set_capacitance((int32_t)(_s.cap_vol*10));
    mc_status.set_infrared((int32_t)_s.robot_is_infrared);
    mc_status.set_have_imu(imu_online);
    auto size = mc_status.ByteSizeLong();
    data.resize(size);
    mc_status.SerializeToArray(data.ptr(), size);
    wifi_.udp_sender_multicast(data.data(), size);
    // zos::log("mul_size,{}\n",size);
    mc_status.Clear();
}

void RobotCm::device_run()
{

    Robot_CMD _c;
    Robot_STATUS _s;
    {
        std::scoped_lock lock{_robot_cmd_mutex};
        _c = robot_cmd;
    }

    // now_time = std::chrono::steady_clock::now();
    // dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
    // last_time = now_time;
    // zos::log("dt = {}\n",dt);
#ifndef CM4_stm32
#ifndef motor_i2c_same_addr
    device_.motor_run();
#else
    device_.all_motor_run();
#endif
#endif
    device_.set_dribbler((int)abs(_c.drib_power) | 0x0c);
}

void RobotCm::control()
{
    // now_time = std::chrono::steady_clock::now();
    // dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
    // last_time = now_time;

    // zos::log("debug\n");

    Vision _v;
    Robot_STATUS _s;

    {
        std::scoped_lock lock{_robot_status_mutex, _pVision_mutex};
        _s = robot_status;
        _v = pVision;
    }

    double poseX;
    double velX;
    double velX_imu;
    double poseY;
    double velY;
    double velY_imu;

    // zos::log("vision : {},{},{}\n",abs(_v.ourRobot(_s.id).X()),abs(_v.ourRobot(_s.id).Y()),vision_err_count);
    // zos::log("imu : {},vision : {}\n",_s.imu_data[10]*3.1415926/180,_v.ourRobot(_s.id).Dir());

    if (abs(_v.ourRobot(_s.id).X()) < 8000 && abs(_v.ourRobot(_s.id).Y()) < 8000 && vision_err_count < 20 && imu_online)
    {

        double angleZ = _v.ourRobot(_s.id).Dir();

        get_fdb(1.0 / config::robot_freq, _s.imu_data[0], vision_to_imu_ * _v.ourRobot(_s.id).X(), poseX, velX_imu, filter_x_);
        get_fdb(1.0 / config::robot_freq, _s.imu_data[1], vision_to_imu_ * _v.ourRobot(_s.id).Y(), poseY, velY_imu, filter_y_);

        velX = cos(angleZ) * velX_imu + sin(angleZ) * velY_imu;
        velY = -sin(angleZ) * velX_imu + cos(angleZ) * velY_imu;
        {
            std::scoped_lock lock{_robot_status_mutex};
            robot_status.robotX = poseX;
            robot_status.robotXd = velX;
            robot_status.robotY = poseY;
            robot_status.robotYd = velY;
        }
    }

    if (imu_online)
    {
        motion_planner(1000000.0 / config::robot_freq, _s.imu_data[10] * 3.1415926 / 180, true);
        // zos::log("imu_online\n");
    }
    else if (abs(_v.ourRobot(_s.id).X()) < 8000 && abs(_v.ourRobot(_s.id).Y()) < 8000 && vision_err_count < 20)
    {
        motion_planner(1000000.0 / config::robot_freq, _v.ourRobot(_s.id).Dir(), true);
        // zos::log("vision_online\n");
    }
    else
    {
        motion_planner(1000000.0 / config::robot_freq, _v.ourRobot(_s.id).Dir(), false);
        // zos::log("no_angle_pid\n");
    }

    // if (initFinish)
    // {
    //     std::ofstream file(logcsv, std::ios::app);
    //     if (!file.is_open())
    //     {
    //         zos::log("file open false\n");
    //     }
    //     file << poseX << "," << poseY << "," << _s.imu_data[10] * 3.1415926 / 180 << "," << _v.ourRobot(_s.id).Dir() << std::endl;
    //     file.close();
    // }

#if defined(motor_i2c_same_addr)||defined(CM4_stm32)
    device_.set_all_motor_vel(all_vel_pack_);
#else
    device_.set_motors_vel(vel_pack_);
#endif

    auto temp_read = device_.get_motors_vel();
    for (int i = 0; i < MAX_MOTOR; i++)
    {
        vel_read_temp_[i] = temp_read[i];
    }

    
}

void RobotCm::shoot()
{
    

    Robot_CMD _c;
    Robot_STATUS _s;
    {
        std::scoped_lock lock{_robot_cmd_mutex};
        _c = robot_cmd;
    }
    {
        std::scoped_lock lock{_robot_status_mutex};
        _s = robot_status;
    }
    // kick
    int kick_status = -1;
    if (_s.robot_is_infrared > 0 && _c.kick_en && _s.last_kick_time > 300)
    {
        if (initFinish)
        {
            std::ofstream file(logcsv, std::ios::app);
            if (!file.is_open())
            {
                zos::log("file open false\n");
            }
            file << _c.kick_en << "," << _c.kick_mode << "," << _c.kick_discharge_time << "," << _s.robot_is_infrared << "," <<_s.last_kick_time<< std::endl;
            file.close();
        }
        zos::log("kick info   en:{}, mode:{}, time:{}   ir: {}, last_time: {}\n", _c.kick_en, _c.kick_mode, _c.kick_discharge_time, _s.robot_is_infrared, _s.last_kick_time);
        kick_status = device_.shoot_chip(_c.kick_mode, _c.kick_discharge_time);
        // TODO: _s.robot_is_shooted/chipped
        {
            std::scoped_lock lock{_robot_status_mutex, _robot_cmd_mutex};
            auto &_status = robot_status;
            switch (kick_status)
            {
            case SHOOT_MODE:
                // zos::log("--kick status: shoot!\n");
                _status.robot_is_shooted = 0;
                break;
            case CHIP_MODE:
                // zos::log("--kick status: chip!\n");
                _status.robot_is_chipped = 0;
                break;
            }
            robot_status.last_kick_time = 0;
            robot_cmd.kick_en = false;
            robot_cmd.kick_discharge_time = 0;
        }
    }
}

// dt in s
void RobotCm::get_fdb(const double _dt, double imu, double vision, double &pose, double &vel, filter_ &filter)
{
    using namespace std;

    vector<vector<float>> A = matrix::creatMatrix<float>(3, 3, 0);
    A[0][0] = 1;
    A[0][1] = _dt;
    A[0][2] = _dt * _dt / 2;
    A[1][1] = 1;
    A[1][2] = _dt / 2;
    A[2][2] = 0;

    vector<vector<float>> B = matrix::creatMatrix<float>(3, 1, 0);
    B[1][0] = _dt / 2;
    B[2][0] = 1;

    vector<vector<float>> C = matrix::creatMatrix<float>(1, 3, 0);
    C[0][0] = 1;

    float tmp = filter.P_k[0][0] + r_measure_;

    std::vector<std::vector<float>> M = matrix::creatMatrix<float>(3, 3, 0);

    M = matrix::matrixMultiplyNum<float>(matrix::matrixMultiply<float>(filter.P_k, matrix::matrixTrans<float>(C)), 1 / tmp);

    filter.P_k = matrix::matrixSum<float>(
        matrix::matrixMultiply<float>(matrix::matrixMultiply<float>(A, filter.P_k), matrix::matrixTrans<float>(A)),
        matrix::matrixMultiply<float>(matrix::matrixMultiplyNum<float>(B, q_), matrix::matrixTrans<float>(B)));

    vector<vector<float>> eye = matrix::creatMatrix<float>(3, 3, 0);
    eye[0][0] = 1;
    eye[1][1] = 1;
    eye[2][2] = 1;

    filter.P_k = matrix::matrixMultiply<float>(matrix::matrixMinus(eye, matrix::matrixMultiply<float>(M, C)), filter.P_k);

    tmp = matrix::matrixMultiply<float>(matrix::matrixMultiply<float>(C, A), filter.X)[0][0];

    filter.X = matrix::matrixSum<float>(
        matrix::matrixMultiply<float>(A, filter.X),
        matrix::matrixMultiplyNum<float>(M, vision / 1000.0 - tmp));

    filter.X = matrix::matrixSum<float>(
        matrix::matrixMultiply<float>(A, filter.X),
        matrix::matrixMultiplyNum<float>(B, imu));

    pose = filter.X[0][0];

    float d = radius_ * time_step_;
    float d0 = d * time_step_;

    filter.x1k = filter.x1;
    filter.x2k = filter.x2;

    float tdy = filter.x1k - pose + time_step_ * filter.x2k;
    float a0 = sqrt(d * d + 8 * radius_ * abs(tdy));

    float a = 0;

    if (abs(tdy) <= d0)
    {
        a = filter.x2k + tdy / time_step_;
    }
    else
    {
        if (tdy > 0)
        {
            a = filter.x2k + 0.5 * (a0 - d);
        }
        else
        {
            a = filter.x2k - 0.5 * (a0 - d);
        }
    }

    float f = 0;
    if (abs(a) <= d)
    {
        f = -radius_ * a / d;
    }
    else
    {
        if (a > 0)
        {
            f = -radius_;
        }
        else
        {
            f = radius_;
        }
    }

    filter.x1 = filter.x1k + _dt * filter.x2k;
    filter.x2 = filter.x2k + _dt * f;

    vel = filter.x2;
}

double RobotCm::get_vr(const double _dt, double fdb, bool have_fdb)
{
    Robot_CMD _c;
    Robot_STATUS _s;
    {
        std::scoped_lock lock{_robot_cmd_mutex};
        _c = robot_cmd;
    }
    {
        std::scoped_lock lock{_robot_status_mutex};
        _s = robot_status;
    }
    if (!_c.use_dir || !have_fdb)
        return _c.vr_target;
    float current_angle = fdb;
    float target_angle = _c.vr_target;
    error_ = target_angle - current_angle;

    if (error_ < -3.1415926)
    {
        error_ = error_ + 2 * 3.1415926;
    }
    else if (error_ > 3.1415926)
    {
        error_ = error_ - 2 * 3.1415926;
    }

    // zos::log("err : {}\n", error_);

    float derivative = (error_ - prev_error_) / _dt;
    prev_error_ = error_;
    double output_vr = _c.angle_pid[0] * error_ + _c.angle_pid[2] * derivative;
    // zos::status("output_vr: {} error_: {} target_theta: {} current angle: {}\n", output_vr, error_, target_angle, current_angle);
    return output_vr;
}
 
void RobotCm::motion_planner(const double _dt, double fdb, bool have_fdb)
{ // dt in us
    Robot_CMD _c;
    {
        std::scoped_lock lock{_robot_cmd_mutex};
        _c = robot_cmd;
    }
    Robot_STATUS _s;
    {
        std::scoped_lock lock{_robot_status_mutex};
        _s = robot_status;
    }

    double acc_x = (_c.vx_target - _c.vx_target_last) / (_dt / 1000000);
    double acc_y = (_c.vy_target - _c.vy_target_last) / (_dt / 1000000);
    // double acc_whole = sqrt(acc_x * acc_x + acc_y * acc_y);
    // if (acc_whole > config::a_max)
    // {
    //     double sin_x = acc_x / acc_whole;
    //     double sin_y = acc_y / acc_whole;
    //     acc_whole = config::a_max;
    //     acc_x = acc_whole * sin_x;
    //     acc_y = acc_whole * sin_y;
    // }
    // double vx_target = 0, vy_target = 0;
    // vx_target = _c.vx_target_last + acc_x * _dt / 1000000.0;
    // vy_target = _c.vy_target_last + acc_y * _dt / 1000000.0;

    if(acc_x>16)
    {
        acc_x = 16;
    }
    if(acc_x<-16)
    {
        acc_x = -16;
    }

    if(acc_y>5)
    {
        acc_y = 5;
    }
    if(acc_y<-5)
    {
        acc_y = -5;
    }

    double vx_target = 0, vy_target = 0;
    vx_target = _c.vx_target_last + acc_x * _dt / 1000000.0;
    vy_target = _c.vy_target_last + acc_y * _dt / 1000000.0;

    double vr = get_vr(_dt, fdb, have_fdb);
    double acc_r = (vr - _c.vr_target_last) / (_dt / 1000000);
    if (acc_r > acc_r_set)
    {
        acc_r = acc_r_set;
    }
    vr = _c.vr_target_last + acc_r * _dt / 1000000.0;

    double vx = 0;
    double vy = 0;

    vx = vx_target;
    vy = vy_target;

    // zos::log("{},{},{}\n", vx, vy, vr);

    all_vel_pack_[0] = (int16_t)(vx * 3000.0);
    all_vel_pack_[1] = (int16_t)(vy * 3000.0);
    all_vel_pack_[2] = (int16_t)(vr * 1000.0);

    bool LimitVelXY = true;
    float VelXYStep = 0.1;

    while(LimitVelXY)
    {
        for (int i = 0; i < 4; i++)
        {
            vel_pack_[i] = ((sin_angle_[i]) * vx + (cos_angle_[i]) * vy - 8.2 * vr / 160) * vel_k2_ * 100; // TODO: *100
            // vel_pack_[i] = (vel_pack_[i] > 8000) ? 8000 : vel_pack_[i];
            // vel_pack_[i] = (vel_pack_[i] < -8000) ? -8000 : vel_pack_[i];
            // zos::log("{},{},{}\n",vx,vy,vr);
            
        }
        if( vel_pack_[0] < 4000 && vel_pack_[0] > -4000 && 
            vel_pack_[1] < 4000 && vel_pack_[1] > -4000 && 
            vel_pack_[2] < 4000 && vel_pack_[2] > -4000 && 
            vel_pack_[3] < 4000 && vel_pack_[3] > -4000)
        {
            LimitVelXY = false;
        }
        else
        {

            // if(vx > -1 && vx < 1)
            // {
            //     break;
            // }

            if(vx < -1)
            {
                vx = vx + VelXYStep;
            }

            if(vx > 1)
            {
                vx = vx - VelXYStep;
            }

            if(vy < -3)
            {
                vy = vy + VelXYStep;
            }

            if(vy > 3)
            {
                vy = vy - VelXYStep;
            }

            if(vr < -10)
            {
                vr = vr + VelXYStep;
            }

            if(vr > 10)
            {
                vr = vr - VelXYStep;
            }
        }
    }
    
    {
        std::scoped_lock lock{_robot_cmd_mutex};
        auto &_c = robot_cmd;
        _c.vx_target_last = vx;
        _c.vy_target_last = vy;
        _c.vr_target_last = vr;
    }

    // zos::info("vel_pack_: {} {} {} {}\n", vel_pack_[0], vel_pack_[1], vel_pack_[2], vel_pack_[3]);
}

void RobotCm::stand()
{
    std::scoped_lock lock{_robot_cmd_mutex};
    robot_cmd.cmd_type = CMD_TYPE_VEL;

    robot_cmd.kick_en = false;
    robot_cmd.kick_discharge_time = 0;

    robot_cmd.vy_target = 0;
    robot_cmd.vx_target = 0;
    robot_cmd.vr_target = 0;
    robot_cmd.use_dir = 0;
    if (_time_since_last_pack.load() > 1000)
    {
        robot_cmd.drib_power = 0;
    }
    

    std::fill_n(begin(vel_pack_), MAX_MOTOR, 0);

    wifi_.udp_restart();
}

void RobotCm::read_config_yaml()
{
    try
    {
        // zos::log("read_config_yaml\n");
        config_yaml_ = YAML::LoadFile("/home/pi/rpi_timer/robot/config.yaml");
        // id
        robot_id = config_yaml_["id"].as<int>();
        // zos::log("robot id: {}\n", robot_id);
        {
            std::scoped_lock lock{_robot_status_mutex};
            auto &_s = robot_status;
            _s.id = robot_id;
        }
        // freq
        nrf2401_freq_ = config_yaml_["freq"].as<int>();
        // zos::log("nrf2401 freq: {}\n", nrf2401_freq_);
        //  team: 1 for blue,2 for yellow
        {
            std::scoped_lock lock{_robot_status_mutex};
            auto &_s = robot_status;
            if (nrf2401_freq_ == 6)
            {
                _s.team = 2;
                vision_to_imu_ = -1;
            }
            else if (nrf2401_freq_ == 8)
            {
                _s.team = 1;
                vision_to_imu_ = 1;
            }
        }
        zos::warning("id: {}, team: {}\n", robot_status.id, robot_status.team);

        // motors pid
        for (auto motor : config_yaml_["motor_pid"])
        {
            int motor_id = motor["id"].as<int>();
            // zos::log("motor{}  p: {}  i: {}\n", motor_id, motor["p"].as<int>(), motor["i"].as<int>());
            pid_pack_[motor_id * 2] = motor["p"].as<int>();
            pid_pack_[motor_id * 2 + 1] = motor["i"].as<int>();
        }
    }
    catch (const YAML::BadFile &e)
    {
        std::cerr << e.msg << std::endl;
    }
    catch (const YAML::ParserException &e)
    {
        std::cerr << e.msg << std::endl;
    }
    catch (...)
    {
        zos::error("read config.yaml failed, use default id #{}\n", robot_id);
    }
}

void RobotCm::save_id(int robot_id_new)
{
    if (config_yaml_.IsNull())
    {
        config_yaml_ = YAML::LoadFile("/home/pi/rpi_timer/robot/config.yaml");
    }
    config_yaml_["id"] = robot_id_new;
    std::ofstream file_stream("/home/pi/rpi_timer/robot/config.yaml");
    file_stream << config_yaml_; // dump it back into the file
    file_stream.close();
    // zos::log("robot id save\n");
}

void RobotCm::save_pid()
{
    if (config_yaml_.IsNull())
    {
        config_yaml_ = YAML::LoadFile("/home/pi/rpi_timer/robot/config.yaml");
    }
    for (auto motor : config_yaml_["motor_pid"])
    {
        int motor_id = motor["id"].as<int>();
        if (pid_real_[motor_id * 2] > 0)
        {
            motor["p"] = pid_real_[motor_id * 2];
        }
        if (pid_real_[motor_id * 2 + 1] > 0)
        {
            motor["i"] = pid_real_[motor_id * 2 + 1];
        }
    }
    std::ofstream file_stream("/home/pi/rpi_timer/robot/config.yaml");
    file_stream << config_yaml_; // dump it back into the file
    file_stream.close();
    // zos::log("pid update\n");
    //  pid_read();
}

#ifndef motor_i2c_same_addr
void RobotCm::set_pid()
{
    stand();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pid_busy = true;
    device_.set_motors_pid(pid_pack_);
    pid_real_ = device_.get_motors_pid();
    save_pid();
    std::fill_n(begin(pid_pack_), 8, 0);
    pid_busy = false;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
#endif

void RobotCm::save_freq(int freq_new)
{
    if (config_yaml_.IsNull())
    {
        config_yaml_ = YAML::LoadFile("/home/pi/rpi_timer/robot/config.yaml");
    }
    config_yaml_["freq"] = freq_new;
    std::ofstream file_stream("/home/pi/rpi_timer/robot/config.yaml");
    file_stream << config_yaml_; // dump it back into the file
    file_stream.close();
    // zos::log("robot id save\n");
}

double RobotCm::ms_since_last_cmd_pack()
{
    return _time_since_last_pack.load();
}

void RobotCm::cmd_small_callback(const zos::Data &data)
{
    // zos::log("cmd_small_callback\n");
    if (initFinish)
    {

        std::scoped_lock lock{cmd_small_data_mutex_};
        auto res = pb_cmd_mini_.ParseFromArray(cmd_small_data_.data(), cmd_small_data_.size());
        if (res)
        {
            auto &_p = pb_cmd_mini_;
            std::scoped_lock lock{_robot_cmd_mutex};
            auto &_c = robot_cmd;

            // switch (_p.kick_mode())
            // {
            //     case (ZSS::New::Robot_Command_Mini::KickMode::Robot_Command_Mini_KickMode_KICK):
            //         _c.kick_mode = false;
            //         _c.kick_en = true;
            //         break;
            //     case (ZSS::New::Robot_Command_Mini::KickMode::Robot_Command_Mini_KickMode_CHIP):
            //         _c.kick_mode = true;
            //         _c.kick_en = true;
            //         break;    
            //     default:
            //         _c.kick_en = false;
            //         break;
            // }

            using PB_MINI = ZSS::New::Robot_Command_Mini;
            // _c.id = _p.robot_id(); // maybe need to check if id correct
            _c.kick_en = (_p.kick_mode() != PB_MINI::NONE);
            _c.kick_mode = (_p.kick_mode() == PB_MINI::CHIP);

            _c.kick_discharge_time = _p.kick_discharge_time();
            

            // _c.desire_power = _p.desire_power();
            
        }
    }
}

void RobotCm::vision_callback(const zos::Data &data)
{
    if (initFinish)
    {

        Robot_STATUS _s;

        {
            std::scoped_lock lock{_robot_status_mutex};
            _s = robot_status;
        }

        // zos::log("vision_callback\n");
        std::scoped_lock lock{vision_data_mutex_};
        auto res = pb_detection_frame_.ParseFromArray(vision_data_.data(), vision_data_.size());
        if (res)
        {
            std::scoped_lock lock{_vision_queue_mutex};
            auto &_p = pb_detection_frame_;
            auto &_v = VisionTmp;

            // zos::log("{}\n",_p.DebugString());
            // TODO update

            // update Ball vision
            _v.setUpdate();
            _v.ball.area = _p.balls().area();
            _v.ball.vel_x = _p.balls().vel_x();
            _v.ball.vel_y = _p.balls().vel_y();
            _v.ball.x = _p.balls().x();
            _v.ball.y = _p.balls().y();
            _v.ball.valid = _p.balls().valid();
            _v.ball.raw_x = _p.balls().raw_x();
            _v.ball.raw_y = _p.balls().raw_y();
            _v.ball.raw_vel_x = _p.balls().raw_vel_x();
            _v.ball.raw_vel_y = _p.balls().raw_vel_y();
            // update robot vision
            if (_s.team == 1)
            {
                for (int i = 0; i < 16; i++)
                {
                    auto robot_blue = _p.robots_blue(i);
                    auto robot_yellow = _p.robots_yellow(i);
                    if (!robot_blue.valid())
                    {
                        _v.ourRobot(i).SetValid(false);
                    }
                    else
                    {
                        _v.ourRobot(i).SetValid(true);
                        _v.ourRobot(i).SetDir(robot_blue.orientation());
                        _v.ourRobot(i).SetAcc(robot_blue.accelerate_x(), robot_blue.accelerate_y());
                        _v.ourRobot(i).SetPos(robot_blue.x(), robot_blue.y());
                        _v.ourRobot(i).SetVel(robot_blue.vel_x(), robot_blue.vel_y());
                        _v.ourRobot(i).SetRotVel(robot_blue.rotate_vel());
                        _v.ourRobot(i).SetRawPos(robot_blue.raw_x(), robot_blue.raw_y());
                        _v.ourRobot(i).SetRawVel(robot_blue.raw_vel_x(), robot_blue.raw_vel_y());
                        _v.ourRobot(i).SetRawRotVel(robot_blue.raw_rotate_vel());
                    }
                    if (!robot_yellow.valid())
                    {
                        _v.theirRobot(i).SetValid(false);
                    }
                    else
                    {
                        _v.theirRobot(i).SetValid(true);
                        _v.theirRobot(i).SetDir(robot_yellow.orientation());
                        _v.theirRobot(i).SetAcc(robot_yellow.accelerate_x(), robot_yellow.accelerate_y());
                        _v.theirRobot(i).SetPos(robot_yellow.x(), robot_yellow.y());
                        _v.theirRobot(i).SetVel(robot_yellow.vel_x(), robot_yellow.vel_y());
                        _v.theirRobot(i).SetRotVel(robot_yellow.rotate_vel());
                        _v.theirRobot(i).SetRawPos(robot_yellow.raw_x(), robot_yellow.raw_y());
                        _v.theirRobot(i).SetRawVel(robot_yellow.raw_vel_x(), robot_yellow.raw_vel_y());
                        _v.theirRobot(i).SetRawRotVel(robot_yellow.raw_rotate_vel());
                    }
                }
            }
            else if (_s.team == 2)
            {
                for (int i = 0; i < 16; i++)
                {
                    auto robot_blue = _p.robots_blue(i);
                    auto robot_yellow = _p.robots_yellow(i);
                    if (!robot_blue.valid())
                    {
                        _v.theirRobot(i).SetValid(false);
                    }
                    else
                    {
                        _v.theirRobot(i).SetValid(true);
                        _v.theirRobot(i).SetDir(robot_blue.orientation());
                        _v.theirRobot(i).SetAcc(robot_blue.accelerate_x(), robot_blue.accelerate_y());
                        _v.theirRobot(i).SetPos(robot_blue.x(), robot_blue.y());
                        _v.theirRobot(i).SetVel(robot_blue.vel_x(), robot_blue.vel_y());
                        _v.theirRobot(i).SetRotVel(robot_blue.rotate_vel());
                        _v.theirRobot(i).SetRawPos(robot_blue.raw_x(), robot_blue.raw_y());
                        _v.theirRobot(i).SetRawVel(robot_blue.raw_vel_x(), robot_blue.raw_vel_y());
                        _v.theirRobot(i).SetRawRotVel(robot_blue.raw_rotate_vel());
                    }
                    if (!robot_yellow.valid())
                    {
                        _v.ourRobot(i).SetValid(false);
                    }
                    else
                    {
                        _v.ourRobot(i).SetValid(true);
                        _v.ourRobot(i).SetDir(robot_yellow.orientation());
                        _v.ourRobot(i).SetAcc(robot_yellow.accelerate_x(), robot_yellow.accelerate_y());
                        _v.ourRobot(i).SetPos(robot_yellow.x(), robot_yellow.y());
                        _v.ourRobot(i).SetVel(robot_yellow.vel_x(), robot_yellow.vel_y());
                        _v.ourRobot(i).SetRotVel(robot_yellow.rotate_vel());
                        _v.ourRobot(i).SetRawPos(robot_yellow.raw_x(), robot_yellow.raw_y());
                        _v.ourRobot(i).SetRawVel(robot_yellow.raw_vel_x(), robot_yellow.raw_vel_y());
                        _v.ourRobot(i).SetRawRotVel(robot_yellow.raw_rotate_vel());
                    }
                }
            }

            Vision_queue.push(_v);
        }
    }
}
