#ifndef ROBOT_BASE_H
#define ROBOT_BASE_H

#include "device_base.h"
#include <memory>
#include <atomic>

class RobotBase
{
public:
    virtual ~RobotBase() = default;

    virtual void send_status() = 0;
    virtual void multicast_info() = 0;
    virtual void shoot() = 0;
    virtual void stand() = 0;
    virtual void update_expansion_board(const double dt) = 0;
    virtual double ms_since_last_cmd_pack() = 0;

    std::unique_ptr<DeviceBase> device_ptr_;
};

#endif
