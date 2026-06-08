#include "device_stm32.h"

static auto now_time = std::chrono::steady_clock::now();
static auto last_time = now_time - std::chrono::microseconds(1000000) / config::robot_freq;

DeviceStm32::DeviceStm32()
{
    // 检测gpio初始化完成
    gpioCfgSetInternals(gpioCfgGetInternals() | PI_CFG_NOSIGHANDLER);
    auto _code_ = gpioInitialise();
    if (_code_ < 0)
    {
        zos::error("pigpio error!{}\n", _code_);
    }
    else
    {
        zos::log("pigpio init\n");
    }

    gpioSetMode(GPIO_SHOOT, PI_OUTPUT);
    gpioSetMode(GPIO_CHIP, PI_OUTPUT);
    gpioWrite(GPIO_SHOOT, PI_LOW);
    gpioWrite(GPIO_CHIP, PI_LOW);
    gpioSetMode(GPIO_BUZZER, PI_OUTPUT);

    try
    {
        stm32_uart = new mraa::Uart("/dev/ttyAMA3");
    }
    catch (std::exception &e)
    {
        std::cerr << "Error while setting up raw UART, do you have a uart?" << std::endl;
        std::terminate();
    }

    if (stm32_uart->setBaudRate(115200) != mraa::SUCCESS)
    {
        std::cerr << "Error setting parity on UART" << std::endl;
    }

    stm32_spi = new mraa::Spi(1,0);
    if(stm32_spi == NULL)
    {
        zos::error("no spi");
    }

    stm32_spi->mode(mraa::SPI_MODE0);
    stm32_spi->frequency(1000000);

    buzzer_start();

    have_imu = true;
}

void DeviceStm32::read_write_stm32()
{
    CM4_to_stm32_spi tx = {0};
    stm32_to_CM4_spi rx = {0};

    tx.drib_power = drib_power;
    tx.use_imu = true;
    tx.kick_mode = kick_mode.load();
    tx.kick_discharge_time = (uint16_t)kick_discharge_time.load();
    tx.drib_velocity = (int16_t)(drib_velocity.load() * 100);
    tx.drib_torque_ff = (int16_t)(drib_torque_ff.load() * 1000);
    // zos::log("kick_mode: {}, kick_discharge_time: {}\n", kick_mode.load(), kick_discharge_time.load());

    for(int i=0;i<3;i++)
    {
        if(i==2)
        {
            tx.vel[i] = (int16_t)(vel_all[i]*100);
        }
        else
        {
            tx.vel[i] = (int16_t)(vel_all[i]*1000);
        }
        

        tx.angle_pid[i] = (int16_t)(angle_pid[i]*1000);
        // zos::log("{},{}\n",vel_all[i],angle_pid[i]);
    }

    // zos::log("{},{},{}\n",vel_all[0],vel_all[1],vel_all[2]);
    // zos::log("{},{},{}\n",tx.vel[0],tx.vel[1],tx.vel[2]);
    memcpy(tx_data_spi,&tx,sizeof(tx));
    // zos::log("tx_data_spi: {},{}, {}\n",tx_data_spi[0],tx_data_spi[1],tx_data_spi[2]);

    int result = stm32_spi->transfer(tx_data_spi,rx_data_spi,spi_length);
    // zos::log("tmp :{},rx:{},{},{},{},{}\n",tmp,rx_data_spi[0],rx_data_spi[1],rx_data_spi[2],rx_data_spi[3],rx_data_spi[4]);
    // zos::log("stm32 spi transfer result: {}\n", result);

    memcpy(&rx,rx_data_spi,sizeof(rx));

    // zos::log("size = {},{}\n",sizeof(tx),sizeof(rx));

    have_imu = rx.imu_online;
    {
        std::scoped_lock lock{imu_mutex};
        imu_status.acc_x = rx.imu_data[0]/100.0;
        imu_status.acc_y = rx.imu_data[1]/100.0;
        imu_status.acc_z = rx.imu_data[2]/100.0;
        imu_status.omega_x = rx.imu_data[3]/100.0;
        imu_status.omega_y = rx.imu_data[4]/100.0;
        imu_status.omega_z = rx.imu_data[5]/100.0;
        imu_status.theta_x = rx.imu_data[6]/100.0;
        imu_status.theta_y = rx.imu_data[7]/100.0;
        imu_status.theta_z = rx.imu_data[8]/100.0;
    }

    have_infrare = rx.infrare_flag;
    battery_vol = rx.battery_vol/5.0;
    cap_vol = rx.cap_vol/100.0;

    for(int i =0;i<4;i++)
    {
        wheel[i] = rx.wheel[i]/10.0;
    }

    // int dt = 0;

    // now_time = std::chrono::steady_clock::now();
    // dt = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time).count();
    // last_time = now_time;
    // if(dt/1000.0>100)
    // {
    //     zos::log("dt = {}\n", dt);
    // }
    
}

void DeviceStm32::buzzer_start()
{
    std::thread _buzzer = std::thread([this]
                                      {
        buzzer_once(500);
        buzzer_once(1600); });
    _buzzer.join();
}

void DeviceStm32::buzzer_once(int freq)
{
    // gpioSetPWMfrequency(GPIO_BUZZER, freq);
    // gpioPWM(GPIO_BUZZER, 20);
    // std::this_thread::sleep_for(std::chrono::milliseconds(300));
    // gpioPWM(GPIO_BUZZER, 0);
}

void DeviceStm32::buzzer_thread()
{
    std::thread _buzzer = std::thread([this]
                                      { buzzer_once(1600); });
    _buzzer.join();
}

// float DeviceStm32::shoot_chip(bool kick_mode, float kick_discharge_time)
// {
//     int kick_flag = -1;
//     int kick_gpio;
//     if (kick_mode == SHOOT_MODE)
//     {
//         kick_gpio = GPIO_SHOOT;
//     }
//     else if (kick_mode == CHIP_MODE)
//     {
//         kick_gpio = GPIO_CHIP;
//     }
//     else
//     {
//         zos::error("wrong kick mode\n");
//         return -1;
//     }

//     if (kick_discharge_time > 0)
//     {
//         // std::jthread th_shoot([this, kick_gpio, kick_discharge_time] {
//         // 获取时间，用于时间间隔
//         auto time_pre = std::chrono::steady_clock::now();
//         // 设置时间间隔
//         std::chrono::duration<int, std::nano> _step = std::chrono::microseconds(1);
//         gpioWrite(kick_gpio, PI_HIGH);
//         zos::log("shoot: {}     ", _step * kick_discharge_time);
//         std::this_thread::sleep_for(_step * (int)kick_discharge_time);
//         gpioWrite(kick_gpio, PI_LOW);
//         auto time_now = std::chrono::steady_clock::now();
//         auto step = (time_now - time_pre);
//         // zos::status("kick power: {}, period time: {}, vol remain: {}\n", (int)kick_discharge_time, step.count() / 1000, cap_vol);
//         // });
//         // th_shoot.detach();
//         kick_flag = kick_mode;
//     }
//     else
//     {
//         // zos::log("kick power: {}, low voltage: {}, boot power: {}\n", (int)kick_discharge_time, cap_vol, (int)kick_discharge_time);
//     }
//     // gpioWrite(kick_gpio, PI_LOW);

//     zos::status("shoot\n");
//     return kick_flag;
// }
