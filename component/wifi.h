#ifndef WIFI_H
#define WIFI_H

#include "zos/socket.h"
#include "zos/log.h"
#include <fmt/core.h>
#include "config.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <memory>

class ContextManager
{
public:
    ContextManager(asio::io_context *const context) : _context(context) { (void)_context; zos::udp::ensure_io_runner_started(); }
    ~ContextManager() = default;
    void start()
    {
        zos::udp::ensure_io_runner_started();
    }
    void stop()
    {
        // shared io runner is process-wide; no-op by design
    }

private:
    asio::io_context *_context;
};

//

class wifi_comm
{
public:
    // std::thread receiveThread;
    wifi_comm(const zos::udp::__callback_ep_type &f = {});
    ~wifi_comm();
    void udp_stop();
    void udp_restart();
    void udp_receiver();
    void udp_sender_point(const void *p, const size_t size);
    void udp_sender_multicast(const void *p, const size_t size);
    void set_master_ip(const std::string &ip)
    {
        std::scoped_lock lock{_endpoint_mutex};
        send2master_endpoint.address(asio::ip::address::from_string(ip));
        _master_ip = ip;
    }
    std::string get_master_ip() { std::scoped_lock lock{_endpoint_mutex}; return _master_ip; }
    std::string get_ip() { std::scoped_lock lock{_endpoint_mutex}; return _self_ip; }

private:
    
    void _cb(const void *, size_t);
    asio::ip::udp::endpoint receiver_endpoint;
    asio::ip::udp::endpoint multicast_endpoint;
    asio::ip::udp::endpoint send2master_endpoint;

    zos::udp::socket socket_send2master;
    zos::udp::socket socket_send_multicast;
    zos::udp::socket socket_receiver;

    ContextManager _context_manager;
    mutable std::mutex _endpoint_mutex;

    std::string _self_ip = "";
    std::string _master_ip = "";
};

class wifi_vision_
{
public:
    // std::thread receiveThread;
    wifi_vision_(const zos::udp::__callback_ep_type &f = {});
    ~wifi_vision_();
    void udp_restart();
    void udp_receiver();

private:
    ContextManager _context_manager_vision;
    void _cb(const void *, size_t);
    asio::ip::udp::endpoint receiver_endpoint_vision;

    zos::udp::socket socket_receiver_vision;

};

class wifi_cmd_small_
{
public:
    // std::thread receiveThread;
    wifi_cmd_small_(const zos::udp::__callback_ep_type &f = {});
    ~wifi_cmd_small_();
    void udp_restart();
    void udp_receiver();

private:
    ContextManager _context_manager_cmd_small;
    void _cb(const void *, size_t);
    asio::ip::udp::endpoint receiver_endpoint_cmd_small;

    zos::udp::socket socket_receiver_cmd_small;

};

class wifi_receive_
{
public:
    // std::thread receiveThread;
    wifi_receive_(int receive_port, const zos::udp::__callback_ep_type &f = {});
    ~wifi_receive_();
    void udp_restart();
    void udp_receiver();

private:
    ContextManager _context_manager_receive;
    void _cb(const void *, size_t);
    asio::ip::udp::endpoint receiver_endpoint_receive;

    zos::udp::socket socket_receiver_receive;

};

#endif