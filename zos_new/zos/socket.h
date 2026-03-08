#ifndef __ZOS_UDP_SOCKET_H__
#define __ZOS_UDP_SOCKET_H__

#include <asio.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <array>
#include "zos/utils/singleton.h"
#include "zos/type.h"
#include "zos/posix.h"
#include "zos/log.h"

namespace zos {
namespace udp {

using __callback_type = zos::type::socket_callback_type;
using __callback_ep_type = zos::type::socket_ep_callback_type;
using endpoint = asio::ip::udp::endpoint;
using address = asio::ip::address;
using address_v4 = asio::ip::address_v4;

inline void ensure_io_runner_started() {
    struct IoRunner {
        IoRunner()
            : _ctx(*Singleton<asio::io_context>::_())
            , _work_guard(asio::make_work_guard(_ctx))
            , _thread([this]() {
                try {
                    _ctx.run();
                } catch (const std::exception &e) {
                    zos::error("io_context runner exception: {}", e.what());
                } catch (...) {
                    zos::error("io_context runner unknown exception");
                }
            }) {
            _thread.detach();
        }

        asio::io_context &_ctx;
        asio::executor_work_guard<asio::io_context::executor_type> _work_guard;
        std::thread _thread;
    };

    static std::once_flag flag;
    std::call_once(flag, []() {
        static IoRunner *runner = new IoRunner();
        (void)runner;
    });
}

class socket {
    class Impl : public std::enable_shared_from_this<Impl> {
    public:
        Impl(asio::io_context& ctx) 
            : _socket(ctx, asio::ip::udp::v4()), _io_context(ctx) {
            asio::error_code ec;
            _socket.set_option(asio::socket_base::reuse_address(true), ec);
        }
        
        ~Impl() {
            close_internal();
        }

        void close_internal() {
            if (_socket.is_open()) {
                {
                    std::lock_guard<std::mutex> lock(_callback_mutex);
                    _callback_ep = nullptr;
                }
                asio::error_code ec;
                _socket.cancel(ec); 
                _socket.close(ec);
            }
        }

        bool bind(const asio::ip::udp::endpoint& ep, const __callback_ep_type& f) {
            {
                std::lock_guard<std::mutex> lock(_callback_mutex);
                _callback_ep = f;
            }
            asio::error_code ec;
            
            if (_socket.is_open()) {
                 _socket.close(ec);
            }
            
            _socket.open(ep.protocol(), ec);
            if (ec) { zos::error("open error: {}", ec.message()); return false; }
            
            _socket.set_option(asio::socket_base::reuse_address(true), ec);
            _socket.bind(ep, ec);
            
            if (ec) {
                zos::error("bind error: {}", ec.message());
                return false;
            }
            
            if (f) {
                start_receive();
            }
            return true;
        }

        void send_to(const void* data, size_t size, const asio::ip::udp::endpoint& ep) {
            asio::error_code ec;
            _socket.send_to(asio::buffer(data, size), ep, 0, ec);
            if (ec) {
                zos::error("send_to error: {}", ec.message());
            }
        }
        
        void join_multicast(const asio::ip::address& multicast_address, 
                            const asio::ip::address& if_address) {
            if (!multicast_address.is_multicast()) return;
            try {
                _socket.set_option(asio::ip::multicast::join_group(multicast_address.to_v4(), if_address.to_v4()));
            } catch (std::exception& e) {
                zos::error("join_multicast failed: {}", e.what());
            }
        }
        
        void set_interface(const asio::ip::address& if_address) {
            try {
                 _socket.set_option(asio::ip::multicast::outbound_interface(if_address.to_v4()));
            } catch(std::exception& e) {
                 zos::error("set_interface failed: {}", e.what());
            }
        }

        asio::ip::udp::endpoint get_bind_ep() {
            return _socket.local_endpoint();
        }
        
        bool is_running() { return _socket.is_open(); }

        void start_receive() {
            auto self = shared_from_this();
            _socket.async_receive_from(
                asio::buffer(_recv_buffer), _sender_endpoint,
                [self](std::error_code ec, std::size_t bytes_recvd) {
                    if (!ec && bytes_recvd > 0) {
                        {
                            std::lock_guard<std::mutex> lock(self->_callback_mutex);
                            if (self->_callback_ep) {
                                try {
                                    self->_callback_ep(self->_sender_endpoint, self->_recv_buffer.data(), bytes_recvd);
                                } catch(const std::exception& e) {
                                    zos::error("Uncaught exception in callback: {}", e.what());
                                } catch(...) {
                                    zos::error("Unknown exception in callback");
                                }
                            }
                        }
                        self->start_receive();
                    } else if (ec != asio::error::operation_aborted) {
                        self->start_receive(); 
                    }
                });
        }
        
    private:
        asio::ip::udp::socket _socket;
        asio::io_context& _io_context;
        asio::ip::udp::endpoint _sender_endpoint;
        std::array<char, 65536> _recv_buffer;
        __callback_ep_type _callback_ep;
        std::mutex _callback_mutex;
    };

    std::shared_ptr<Impl> _impl;

public:
    socket() {
        ensure_io_runner_started();
        _impl = std::make_shared<Impl>(*Singleton<asio::io_context>::_());
        // printf("Socket Context Address: %p\n", (void*)Singleton<asio::io_context>::_());
    }
    
    ~socket(){
        close();
    }
    
    bool bind(const asio::ip::udp::endpoint& ep, const __callback_ep_type& f = {}) {
        return _impl->bind(ep, f);
    }
    
     bool bind(const asio::ip::udp::endpoint& ep, const __callback_type& f = {}) {
        auto adapted = [f](const asio::ip::udp::endpoint&, const void* data, size_t size) {
            if(f) f(data, size);
        };
        return bind(ep, adapted);
    }

    bool bind_any(const std::string& ip, const __callback_ep_type& f = {}) {
        asio::ip::udp::endpoint ep(asio::ip::address::from_string(ip), 0);
        return bind(ep, f);
    }
    
    bool bind(const std::string& ip, int port, const __callback_ep_type& f = {}) {
        asio::ip::udp::endpoint ep(asio::ip::address::from_string(ip), port);
        return bind(ep, f);
    }

    void send_to(const void* data, size_t size, const asio::ip::udp::endpoint& ep) {
        _impl->send_to(data, size, ep);
    }
    
    void send_to(const std::string& data, const asio::ip::udp::endpoint& ep) {
        send_to(data.data(), data.size(), ep);
    }

    void join_multicast(const asio::ip::address& multicast_address, 
                        const asio::ip::address& if_address = asio::ip::address_v4::any()) {
        _impl->join_multicast(multicast_address, if_address);
    }
    
    void set_interface(const asio::ip::address& if_address) {
        _impl->set_interface(if_address);
    }

    asio::ip::udp::endpoint get_bind_ep() {
        return _impl->get_bind_ep();
    }
    
    bool is_running() { return _impl->is_running(); }

    void close() {
        if (_impl) _impl->close_internal();
    }
};

using __io = zos::Singleton<asio::io_context>;

} // namespace udp

using __io = udp::__io;

} // namespace zos

#endif // __ZOS_UDP_SOCKET_H__
