
#include "robot_stm32.h"

#define need_store_data

using PB = ZSS::New::Robot_Command;

static auto now_time = std::chrono::steady_clock::now();
static auto last_time = now_time - std::chrono::microseconds(1000000) / config::robot_freq;

// now_time = std::chrono::steady_clock::now();
// dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
// last_time = now_time;

static std::atomic<bool> initFinish = false;

#ifdef need_store_data
static std::string logcsv = "/home/pi/log.csv";
#endif

robot_stm32_::robot_stm32_() : device{},
                   wifi(std::bind(&robot_stm32_::wifi_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
                   wifi_vision(config::receive_port_vision,std::bind(&robot_stm32_::_wifi_vision, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
                   wifi_cmd_small(config::receive_port_cmd_small, std::bind(&robot_stm32_::_wifi_cmd_small, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
                   _cmd_subsciber("cmd", std::bind(&robot_stm32_::cmd_callback, this, std::placeholders::_1)),
                   _cmd_publisher("cmd"),
                   _vision_subsciber("vision", std::bind(&robot_stm32_::_vision_cb, this, std::placeholders::_1)),
                   _vision_publisher("vision"),
                   _cmd_small_subsciber("cmd_small", std::bind(&robot_stm32_::cmd_small_cb, this, std::placeholders::_1)),
                   _cmd_small_publisher("cmd_small")
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
        robot_cmd.acc_set = 150; 
        robot_cmd.acc_r_set = 200;

        robot_cmd.DEC_FRAME = 0;
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

    filter_X.P_k = matrix::creatMatrix<float>(3, 3, 0);
    filter_X.P_k[1][1] = Q * ((1.0 / config::robot_freq) / 2) * ((1.0 / config::robot_freq) / 2);
    filter_X.P_k[2][1] = Q * ((1.0 / config::robot_freq) / 2);
    filter_X.P_k[1][2] = Q * ((1.0 / config::robot_freq) / 2);
    filter_X.P_k[2][2] = Q * 1;

    filter_X.X = matrix::creatMatrix<float>(3, 1, 0);
    filter_X.X[0][0] = visionX / 1000;

    filter_X.x1 = visionX / 1000;
    filter_X.x2 = visionX / 1000;
    filter_X.x1k = visionX / 1000;
    filter_X.x2k = visionX / 1000;

    filter_Y.P_k = matrix::creatMatrix<float>(3, 3, 0);
    filter_Y.P_k[1][1] = Q * ((1.0 / config::robot_freq) / 2) * ((1.0 / config::robot_freq) / 2);
    filter_Y.P_k[2][1] = Q * ((1.0 / config::robot_freq) / 2);
    filter_Y.P_k[1][2] = Q * ((1.0 / config::robot_freq) / 2);
    filter_Y.P_k[2][2] = Q * 1;

    filter_Y.X = matrix::creatMatrix<float>(3, 1, 0);
    filter_Y.X[0][0] = visionY / 1000;

    filter_Y.x1 = visionY / 1000;
    filter_Y.x2 = visionY / 1000;
    filter_Y.x1k = visionY / 1000;
    filter_Y.x2k = visionY / 1000;

    read_config_yaml();
    
    {
        std::scoped_lock lock{_robot_status_mutex};
        if (robot_status.team == 1)
        {
            vision2imu = 1;
        }
        else if (robot_status.team == 2)
        {
            vision2imu = -1;
        }
        else
        {
            vision2imu = 0;
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

void robot_stm32_::wifi_callback(const asio::ip::udp::endpoint &ep, const void *ptr, size_t size)
{
    wifi.set_master_ip(ep.address().to_string());
    {
        std::scoped_lock lock{_cmd_data_mutex};
        _cmd_data.resize(size);
        _cmd_data.store(ptr, size);
    }

    _cmd_publisher.publish();
}

void robot_stm32_::_wifi_vision(const asio::ip::udp::endpoint &ep, const void *ptr, size_t size)
{
    {
        std::scoped_lock lock{_vision_data_mutex};
        _vision_data.resize(size);
        _vision_data.store(ptr, size);
    }
    _vision_publisher.publish();
}

void robot_stm32_::_wifi_cmd_small(const asio::ip::udp::endpoint &ep, const void *ptr, size_t size)
{
    {
        std::scoped_lock lock{_cmd_small_data_mutex};
        _cmd_small_data.resize(size);
        _cmd_small_data.store(ptr, size);
    }
    _cmd_small_publisher.publish();
}

void robot_stm32_::cmd_callback(const zos::Data &data)
{

    if (initFinish)
    {

        std::scoped_lock lock{_cmd_data_mutex};
        auto res = pb_cmd.ParseFromArray(_cmd_data.data(), _cmd_data.size());
        if (res)
        {
            auto &_p = pb_cmd;
            std::scoped_lock lock{_robot_cmd_mutex};
            auto &_c = robot_cmd;
            _time_since_last_pack = 0;
            _c.kick_en = (_p.kick_mode() != PB::NONE);
            _c.kick_mode = (_p.kick_mode() == PB::CHIP);
            _c.kick_discharge_time = _p.kick_discharge_time();
            _c.drib_power = _p.dribble_spin();
            _c.vx_target = ((float)(int32_t)_p.cmd_vel().velocity_x())/1000.0;
            _c.vy_target = ((float)(int32_t)_p.cmd_vel().velocity_y())/1000.0;
            _c.use_dir = _p.cmd_vel().use_imu();
            _c.vr_target = _c.use_dir ? ((float)(int32_t)_p.cmd_vel().imu_theta())/1000.0 : ((float)(int32_t)_p.cmd_vel().velocity_r())/1000.0;
            _c.angle_pid[0] = ((float)(int32_t)_p.angle_pid(0)/1000.0);
            _c.angle_pid[1] = ((float)(int32_t)_p.angle_pid(1)/1000.0);
            _c.angle_pid[2] = ((float)(int32_t)_p.angle_pid(2)/1000.0);
            sendIsDebug = _p.isdebug();

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
        }
    }
}

void robot_stm32_::update_expansion_board(const double dt)
{

    device.read_write_stm32();
    
    {
        std::scoped_lock lock{_robot_status_mutex};
        auto &_s = robot_status;
        auto &_c = robot_cmd;
        // stm32 拓展板获取信息
        if(device.have_infrare)
        {
            if (_s.robot_is_infrared > 0)
            {
                _s.robot_is_infrared = std::min(_s.robot_is_infrared + dt, 10000.0);
            }
            else
            {
                _s.robot_is_infrared = 1;
            }
        }
        else
        {
            _s.robot_is_infrared = std::max(_s.robot_is_infrared - dt, -10000.0);
        }
        _s.cap_vol = device.cap_vol;
        _s.bat_vol = device.battery_vol;

        for(int i=0;i<4;i++)
        {
            _s.motor_encoder[i] = device.wheel[i];
        }

        _s.last_kick_time = std::min(_s.last_kick_time + dt, 10000.0);
        _s.robot_is_shooted = std::min(_s.robot_is_shooted + dt, 100.0);
        _s.robot_is_chipped = std::min(_s.robot_is_chipped + dt, 100.0);

        imu_online = device.have_imu.load();

        {
            std::scoped_lock lock{device.imu_mutex};
            _s.imu_data[0] = device.imu_status.acc_x;
            _s.imu_data[1] = device.imu_status.acc_y;
            _s.imu_data[2] = device.imu_status.acc_z;
            _s.imu_data[3] = 0;
            _s.imu_data[4] = device.imu_status.omega_x;
            _s.imu_data[5] = device.imu_status.omega_y;
            _s.imu_data[6] = device.imu_status.omega_z;
            _s.imu_data[7] = 0;
            _s.imu_data[8] = device.imu_status.theta_x;
            _s.imu_data[9] = device.imu_status.theta_y;
            _s.imu_data[10] = device.imu_status.theta_z;
            _s.imu_data[11] = 0;
        }

        device.set_angle_pid(_c.angle_pid[0],_c.angle_pid[1],_c.angle_pid[2]);
        device.set_dribbler(_c.drib_power);
        device.use_imu = _c.use_dir;
        device.set_vel(_c.vx_target,_c.vy_target,_c.vr_target);
    }
}

void robot_stm32_::send_status()
{
    zos::Data data;
    ZSS::New::Robot_Status pb_status;

    if (wifi.get_master_ip() == "")
    {
	    // zos::log("in send_status no send\n");
        return;
    }

    {        
        if(sendIsDebug)
        {
            // zos::log("send_ip,{}\n",wifi.get_master_ip());
            std::scoped_lock lock{_robot_status_mutex, _pVision_mutex};
            auto &_s = robot_status;
            auto &_v = pVision;
            pb_status.set_robot_id(_s.id);
            pb_status.set_infrared((int32_t)_s.robot_is_infrared);
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
            pb_status.set_have_imu(imu_online);
        }
        else
        {
            // zos::log("send_ip,{}\n",wifi.get_master_ip());
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
            pb_status.set_flat_kick((int32_t)_s.robot_is_shooted);
            pb_status.set_chip_kick((int32_t)_s.robot_is_chipped);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[0]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[1]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[2]);
            pb_status.add_wheel_encoder((int32_t)_s.motor_encoder[3]);
            pb_status.set_team(ZSS::New::Team(_s.team));
            pb_status.set_imu_theta_z((int32_t)(_s.imu_data[10]*100));
            pb_status.set_imu_omega_z((int32_t)(_s.imu_data[6]*100));
            pb_status.set_have_imu(imu_online);
        }


    }
    auto size = pb_status.ByteSizeLong();
    data.resize(size);
    pb_status.SerializeToArray(data.ptr(), size);
    wifi.udp_sender_point(data.data(), size);
    pb_status.Clear();
}

void robot_stm32_::multicast_info()
{

    zos::Data data;
    ZSS::New::Multicast_Status mc_status;
    Robot_STATUS _s;

    _time_since_last_pack += 1000.0 / config::multicast_freq; // ms

    {
        std::scoped_lock lock{_robot_status_mutex};
        _s = robot_status;
    }

    // mc_status.set_ip(wifi.get_ip()); 
    std::string ip = wifi.get_ip();
    size_t last_dot = ip.rfind('.');
    int ip_value = -1;
    if (last_dot != std::string::npos) {
        std::string last_part = ip.substr(last_dot + 1);
        ip_value = std::stoi(last_part);
    } 
    mc_status.set_ip(ip_value);
    mc_status.set_team(ZSS::New::Team(_s.team));
    mc_status.set_robot_id(_s.id); // ROBOT_ID change
    mc_status.set_battery((int32_t)(_s.bat_vol*10));
    mc_status.set_capacitance((int32_t)(_s.cap_vol*10));
    mc_status.set_infrared((int32_t)_s.robot_is_infrared);
    mc_status.set_have_imu(imu_online);
    auto size = mc_status.ByteSizeLong();
    data.resize(size);
    mc_status.SerializeToArray(data.ptr(), size);
    wifi.udp_sender_multicast(data.data(), size);
    mc_status.Clear();
}

void robot_stm32_::shoot()
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
        kick_status = device.shoot_chip(_c.kick_mode, _c.kick_discharge_time);
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
void robot_stm32_::getFdb(const double _dt, double imu, double vision, double &pose, double &vel, filter_ &filter)
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

    float tmp = filter.P_k[0][0] + R;

    std::vector<std::vector<float>> M = matrix::creatMatrix<float>(3, 3, 0);

    M = matrix::matrixMultiplyNum<float>(matrix::matrixMultiply<float>(filter.P_k, matrix::matrixTrans<float>(C)), 1 / tmp);

    filter.P_k = matrix::matrixSum<float>(
        matrix::matrixMultiply<float>(matrix::matrixMultiply<float>(A, filter.P_k), matrix::matrixTrans<float>(A)),
        matrix::matrixMultiply<float>(matrix::matrixMultiplyNum<float>(B, Q), matrix::matrixTrans<float>(B)));

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

    float d = r * h;
    float d0 = d * h;

    filter.x1k = filter.x1;
    filter.x2k = filter.x2;

    float tdy = filter.x1k - pose + h * filter.x2k;
    float a0 = sqrt(d * d + 8 * r * abs(tdy));

    float a = 0;

    if (abs(tdy) <= d0)
    {
        a = filter.x2k + tdy / h;
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
        f = -r * a / d;
    }
    else
    {
        if (a > 0)
        {
            f = -r;
        }
        else
        {
            f = r;
        }
    }

    filter.x1 = filter.x1k + _dt * filter.x2k;
    filter.x2 = filter.x2k + _dt * f;

    vel = filter.x2;
}
 
void robot_stm32_::stand()
{
    std::scoped_lock lock{_cmd_data_mutex};
    robot_cmd.cmd_type = CMD_TYPE_VEL;

    robot_cmd.kick_en = false;
    robot_cmd.kick_discharge_time = 0;

    robot_cmd.vy_target = 0;
    robot_cmd.vx_target = 0;
    robot_cmd.vr_target = 0;
    robot_cmd.use_dir = 0;
    if (_time_since_last_pack > 1000)
    {
        robot_cmd.drib_power = 0;
    }
    

    std::fill_n(begin(vel_pack), MAX_MOTOR, 0);

    wifi.udp_restart();
}

void robot_stm32_::read_config_yaml()
{
    try
    {
        // zos::log("read_config_yaml\n");
        config_yaml = YAML::LoadFile("/home/pi/rpi_timer/robot/config.yaml");
        // id
        robot_id = config_yaml["id"].as<int>();
        // zos::log("robot id: {}\n", robot_id);
        {
            std::scoped_lock lock{_robot_status_mutex};
            auto &_s = robot_status;
            _s.id = robot_id;
        }
        // freq
        nrf2401_freq = config_yaml["freq"].as<int>();
        // zos::log("nrf2401 freq: {}\n", nrf2401_freq);
        //  team: 1 for blue,2 for yellow
        {
            std::scoped_lock lock{_robot_status_mutex};
            auto &_s = robot_status;
            if (nrf2401_freq == 6)
            {
                _s.team = 2;
                vision2imu = -1;
            }
            else if (nrf2401_freq == 8)
            {
                _s.team = 1;
                vision2imu = 1;
            }
        }
        zos::warning("id: {}, team: {}\n", robot_status.id, robot_status.team);

        // motors pid
        for (auto motor : config_yaml["motor_pid"])
        {
            int motor_id = motor["id"].as<int>();
            // zos::log("motor{}  p: {}  i: {}\n", motor_id, motor["p"].as<int>(), motor["i"].as<int>());
            pid_pack[motor_id * 2] = motor["p"].as<int>();
            pid_pack[motor_id * 2 + 1] = motor["i"].as<int>();
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

void robot_stm32_::save_id(int robot_id_new)
{
    if (config_yaml.IsNull())
    {
        config_yaml = YAML::LoadFile("/home/pi/rpi_timer/robot/config.yaml");
    }
    config_yaml["id"] = robot_id_new;
    std::ofstream file_stream("/home/pi/rpi_timer/robot/config.yaml");
    file_stream << config_yaml; // dump it back into the file
    file_stream.close();
    // zos::log("robot id save\n");
}

void robot_stm32_::save_freq(int nrf2401_freq_new)
{
    if (config_yaml.IsNull())
    {
        config_yaml = YAML::LoadFile("/home/pi/rpi_timer/robot/config.yaml");
    }
    config_yaml["freq"] = nrf2401_freq_new;
    std::ofstream file_stream("/home/pi/rpi_timer/robot/config.yaml");
    file_stream << config_yaml; // dump it back into the file
    file_stream.close();
    // zos::log("robot id save\n");
}

double robot_stm32_::ms_since_last_cmd_pack()
{
    return _time_since_last_pack;
}

void robot_stm32_::cmd_small_cb(const zos::Data &data)
{
    zos::log("cmd_small_cb\n");
    if (initFinish)
    {

        std::scoped_lock lock{_cmd_small_data_mutex};
        auto res = pb_cmd_mini.ParseFromArray(_cmd_small_data.data(), _cmd_small_data.size());
        if (res)
        {
            auto &_p = pb_cmd_mini;
            std::scoped_lock lock{_robot_cmd_mutex};
            auto &_c = robot_cmd;           
            using PB_MINI = ZSS::New::Robot_Command_Mini;
            _c.kick_en = (_p.kick_mode() != PB_MINI::NONE);
            _c.kick_mode = (_p.kick_mode() == PB_MINI::CHIP);
            _c.kick_discharge_time = _p.kick_discharge_time();
          
        }
    }
}

void robot_stm32_::_vision_cb(const zos::Data &data)
{
    if (initFinish)
    {

        Robot_STATUS _s;

        {
            std::scoped_lock lock{_robot_status_mutex};
            _s = robot_status;
        }

        // zos::log("vision_callback\n");
        std::scoped_lock lock{_vision_data_mutex};
        auto res = pb_detectionFrame.ParseFromArray(_vision_data.data(), _vision_data.size());
        if (res)
        {
            std::scoped_lock lock{_vision_queue_mutex};
            auto &_p = pb_detectionFrame;
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
