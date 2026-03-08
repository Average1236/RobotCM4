#include "wifi.h"

wifi_comm::wifi_comm(const zos::udp::__callback_ep_type &f) : _context_manager(zos::__io::_())
{
    send2master_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string("127.0.0.1"), config::send_single_port);
    multicast_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(config::multicast_addr), config::send_multicast_port);
    receiver_endpoint = asio::ip::udp::endpoint(zos::udp::address_v4::any(), config::receive_port);

    // printf("Wifi Context Address: %p\n", (void*)zos::__io::_());

    if (f)
    {
        auto res = socket_receiver.bind(receiver_endpoint, f);
        zos::info("wifi_comm socket bind - res : {}\n", res);
        if (res)
        {
            udp_restart();
        }
    }
    else
    {
        zos::error("wifi_comm no callback function\n");
    }

    auto ips = zos::getNetworkInterfacesIP();
    zos::info("list all network interfaces({}):\n", ips.size());
    for (auto ip : ips)
    {
        zos::info("\t{}\n", ip);
        if (ip.rfind(config::multicast_if_prefix, 0) == 0)
        {
            socket_send_multicast.set_interface(asio::ip::address::from_string(ip));
            {
                std::scoped_lock lock{_endpoint_mutex};
                _self_ip = ip;
            }
            zos::status("choose interface : {}\n", ip);
            break;
        }
    }
}
wifi_comm::~wifi_comm()
{
    socket_send2master.close();
    socket_send_multicast.close();
    socket_receiver.close();

    _context_manager.stop();
}

void wifi_comm::udp_stop()
{
    _context_manager.stop();
}

void wifi_comm::udp_restart()
{

    _context_manager.start();
}

void wifi_comm::udp_sender_point(const void *p, const size_t size)
{
    asio::ip::udp::endpoint target_endpoint;
    {
        std::scoped_lock lock{_endpoint_mutex};
        target_endpoint = send2master_endpoint;
    }

    if (p == nullptr)
    {
        zos::log("in udp_sender_normal function\n");
        socket_send2master.send_to(fmt::format("normal udp test!!!"), target_endpoint);
    }
    else
    {
        socket_send2master.send_to(p, size, target_endpoint);
    }
}

void wifi_comm::udp_sender_multicast(const void *p, const size_t size)
{
    if (p == nullptr)
    {
        zos::log("in udp_sender_mc function\n");
        socket_send_multicast.send_to(fmt::format("multicast test!!!"), multicast_endpoint);
    }
    else
    {
        socket_send_multicast.send_to(p, size, multicast_endpoint);
    }
}

wifi_vision_::wifi_vision_(const zos::udp::__callback_ep_type &f) : _context_manager_vision(zos::__io::_())
{
    receiver_endpoint_vision = asio::ip::udp::endpoint(zos::udp::address_v4::any(), config::receive_port_vision);
    if (f)
    {
        auto res = socket_receiver_vision.bind(receiver_endpoint_vision, f);
        zos::info("wifi_vision socket bind - res : {}\n", res);
        if (res)
        {
            udp_restart();
        }
    }
    else
    {
        zos::error("wifi_vision no callback function\n");
    }
}

void wifi_vision_::udp_restart()
{
    _context_manager_vision.start();
}

wifi_vision_::~wifi_vision_()
{
    _context_manager_vision.stop();
}

wifi_cmd_small_::wifi_cmd_small_(const zos::udp::__callback_ep_type &f) : _context_manager_cmd_small(zos::__io::_())
{
    receiver_endpoint_cmd_small = asio::ip::udp::endpoint(zos::udp::address_v4::any(), config::receive_port_cmd_small);
    if (f)
    {
        auto res = socket_receiver_cmd_small.bind(receiver_endpoint_cmd_small, f);
        zos::info("wifi_cmd_small socket bind - res : {}\n", res);
        if (res)
        {
            udp_restart();
        }
    }
    else
    {
        zos::error("wifi_cmd_small no callback function\n");
    }
}

void wifi_cmd_small_::udp_restart()
{
    _context_manager_cmd_small.start();
}

wifi_cmd_small_::~wifi_cmd_small_()
{
    _context_manager_cmd_small.stop();
}


wifi_receive_::wifi_receive_(int receive_port, const zos::udp::__callback_ep_type &f) : _context_manager_receive(zos::__io::_())
{
    receiver_endpoint_receive = asio::ip::udp::endpoint(zos::udp::address_v4::any(), receive_port);
    if (f)
    {
        auto res = socket_receiver_receive.bind(receiver_endpoint_receive, f);
        zos::info("wifi_receive socket bind - res : {}\n", res);
        if (res)
        {
            udp_restart();
        }
    }
    else
    {
        zos::error("wifi_receive no callback function\n");
    }
}

void wifi_receive_::udp_restart()
{
    _context_manager_receive.start();
}

wifi_receive_::~wifi_receive_()
{
    _context_manager_receive.stop();
}