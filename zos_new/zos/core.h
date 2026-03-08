#ifndef __ZOS_CORE_H__
#define __ZOS_CORE_H__

#include <string>
#include <set>
#include <typeinfo>
#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <stdexcept>

// Include necessary zos headers
#include "zos/utils/singleton.h"
#include "zos/config.h"
#include "zos/semadata.h"
#include "zos/log.h"
#include "zos/type.h"
#include "zos/socket.h"
#include "zos/data.h"
#include "zos/manager.h"

namespace zos {

// Forward declarations
class SubscriberBase;
class PublisherBase;
class Publisher;

// Simple Topic Manager using msg_name as key
class TopicManager : public Singleton<TopicManager> {
public:
    void addSubscriber(const std::string& topic, SubscriberBase* sub) {
        std::lock_guard<std::mutex> lock(_mutex);
        _subscribers[topic].push_back(sub);
    }

    void removeSubscriber(const std::string& topic, SubscriberBase* sub) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto& subs = _subscribers[topic];
        subs.erase(std::remove(subs.begin(), subs.end(), sub), subs.end());
    }

    std::vector<SubscriberBase*> getSubscribers(const std::string& topic) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_subscribers.find(topic) != _subscribers.end()) {
            return _subscribers[topic];
        }
        return {};
    }

private:
    std::mutex _mutex;
    std::map<std::string, std::vector<SubscriberBase*>> _subscribers;
};

using __callback_type = zos::type::callback_type;
using __socket_callback_type = zos::type::socket_callback_type;

class SubscriberBase {
public:
    SubscriberBase(const unsigned int buffer_size, const std::string& cs, const std::string& name, const std::string& msg_name, const __callback_type& f = {})
        : _callback(f), _name(name), _msg_name(msg_name) {
        init();
    }
    
    // Simplification: Constructor overloads matching original core.h
    SubscriberBase(const unsigned int buffer_size, const std::string& name, const std::string& msg_name, const __callback_type& f = {})
        : SubscriberBase(buffer_size, "", name, msg_name, f) {}
    
    virtual ~SubscriberBase() {
        TopicManager::_()->removeSubscriber(_msg_name, this);
        zos::manager::removeSub(this, _info);
    }
    
    // Called by Publisher
    void invoke(const zos::Data& data) {
        if (_callback) {
            _callback(data);
        }
    }
    
    std::string name() const { return _name; }
    std::string msg_name() const { return _msg_name; }

protected:
    void init() {
         // Register with TopicManager using msg_name as topic
        TopicManager::_()->addSubscriber(_msg_name, this);
        
        // Register with Manager
        _info.name = _name;
        _info.msg_name = _msg_name;
        _info.ptr = this;
        zos::manager::addSub(this, _info);
    }

    __callback_type _callback;
    std::string _name;
    std::string _msg_name;
    SubscriberInfo _info;
};

template<unsigned int buffer_size=2>
class Subscriber : public SubscriberBase {
public:
    Subscriber(const std::string& name, const __callback_type& f = {})
        : SubscriberBase(buffer_size, "", name + "_subcriber", name, f) {}
        
    Subscriber(const std::string& name, const std::string& msg_name, const __callback_type& f = {})
        : SubscriberBase(buffer_size, "", name, msg_name, f) {}
        
    Subscriber(const std::string& cs, const std::string& name, const std::string& msg_name, const __callback_type& f = {})
        : SubscriberBase(buffer_size, cs, name, msg_name, f) {}
        
    Subscriber(const int _buffer_size_arg, const std::string& name, const std::string& msg_name, const __callback_type& f = {})
        : SubscriberBase(_buffer_size_arg, "", name, msg_name, f) {}
        
    Subscriber(const int _buffer_size_arg, const std::string& cs, const std::string& name, const std::string& msg_name, const __callback_type& f = {})
        : SubscriberBase(_buffer_size_arg, cs, name, msg_name, f) {}
};

class PublisherBase {
public:
    PublisherBase(const std::string& cs, const std::string& name, const std::string& msg_name) 
        : _name(name), _msg_name(msg_name) {
        init();
    }
    
    PublisherBase(const std::string& name, const std::string& msg_name) 
        : _name(name), _msg_name(msg_name) {
        init();
    }
    
    virtual ~PublisherBase() {
        // zos::manager::removePub(this, _info);
    }
    
    virtual void publish(const void* data = nullptr, const unsigned long size = 0) {
        // Prepare data object
        zos::Data d;
        d.store(data, size);
        
        // 1. Get subscribers for this topic (msg_name)
        auto subs = TopicManager::_()->getSubscribers(_msg_name);
        
        // 2. Post to ASIO io_context
        auto& io = *zos::Singleton<asio::io_context>::_();
        
        // For shared data ownership, relying on Data copy/move
        for (auto* sub : subs) {
            asio::post(io, [sub, d]() {
                sub->invoke(d);
            });
        }
    }
    
    virtual void publish(const Data& data) {
        this->publish(data.data(), data.size());
    }
    
    virtual void publish(const std::string& data) {
        this->publish(data.data(), data.length());
    }
    
    std::string name() const { return _name; }
    std::string msg_name() const { return _msg_name; }
    
    bool setCommLevel(const CommLevel& level) { return true; }
    CommLevel commLevel() const { return CommLevel::Default; }

protected:
    void init() {
        _info.name = _name;
        _info.msg_name = _msg_name;
        _info.uuid = zos::manager::uuid();
        _info.ptr = this;
        // zos::manager::addPub(this, _info);
    }

    std::string _name;
    std::string _msg_name;
    PublisherInfo _info;
};

class Publisher : public PublisherBase {
public:
    Publisher(const std::string& name) : Publisher(name + "_publisher", name) {}
    Publisher(const std::string& name, const std::string& msg_name) : PublisherBase(name, msg_name) {}
    Publisher(const std::string& cs, const std::string& name, const std::string& msg_name) : PublisherBase(cs, name, msg_name) {}
};

} // namespace zos

#endif // __ZOS_CORE_H__
