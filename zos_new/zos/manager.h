#ifndef __ZOS_MANAGER_H__
#define __ZOS_MANAGER_H__

#include <atomic>
#include <memory>
#include <string>
#include <mutex>
#include <map>
#include <thread>
// #include <chrono> // implicitly likely needed

#include "zos/type.h"
#include "zos/utils/singleton.h"
#include "zos/socket.h"
#include "zos/posix.h"
#include "zos/log.h" 
// #include "zos/config.h" 

namespace zos {

class PublisherBase;
class SubscriberBase;

class InfoHandler : public Info<PublisherBase*, SubscriberBase*> {
public:
    InfoHandler() = default;
    ~InfoHandler() = default;

    void addPub(PublisherBase* ptr, const PublisherInfo& pub) {
        std::unique_lock<std::mutex> lock(_self_info_mutex);
        Info::addPub(ptr, pub); 
    }
    
    void addSub(SubscriberBase* ptr, const SubscriberInfo& sub) {
        std::unique_lock<std::mutex> lock(_self_info_mutex);
        Info::addSub(ptr, sub);
    }
    
    void removePub(PublisherBase* ptr, const PublisherInfo& pub) {
        std::unique_lock<std::mutex> lock(_self_info_mutex);
        Info::removePub(ptr, pub);
    }
    
    void removeSub(SubscriberBase* ptr, const SubscriberInfo& sub) {
        std::unique_lock<std::mutex> lock(_self_info_mutex);
        Info::removeSub(ptr, sub);
    }
    
    void updatePub(PublisherBase* ptr, const PublisherInfo& pub) {
        std::unique_lock<std::mutex> lock(_self_info_mutex);
        Info::updatePub(ptr, pub);
    }
    
    void updateSub(SubscriberBase* ptr, const SubscriberInfo& sub) {
        std::unique_lock<std::mutex> lock(_self_info_mutex);
        Info::updateSub(ptr, sub);
    }
    
    // Outer/Process info methods stubs as per zos_old usage, though implemented simply
    // based on available context.
    
private:
    std::mutex _self_info_mutex;
    // std::mutex _outer_infos_mutex;
    // std::map<std::string, ProcessInfo> _process_infos;
};

// Manager implementation
class Manager : public Singleton<Manager> {
public:
    Manager(const std::string& name="", const std::string& ns="") 
        : _name(name), _namespace(ns), _pid(getpid()), _uuid("UUID-" + std::to_string(_pid) + "-" + name) { // Simplified UUID generation
    }
    
    // Static helper for singleton access for convenience if typical pattern
    // But Singleton<Manager> likely provides _() or instance()
    // For compatibility with previous `manager.h` replacement in core.h step:
    static void init(const std::string& name = "", const std::string& ns = "") {
        if (!Manager::instance(name, ns)) {
            // Already initialized or error
        }
    }

    static std::string uuid() { return Manager::_()->_uuid; }
    static int pid() { return Manager::_()->_pid; }
    static std::string node_name() { return Manager::_()->_name; }
    static std::string node_ns() { return Manager::_()->_namespace; }
    
    // Pub/Sub management
    static void addPub(PublisherBase* pub, const PublisherInfo& info) {
        Manager::_()->_info_handler.addPub(pub, info);
    }
    
    static void addSub(SubscriberBase* sub, const SubscriberInfo& info) {
        Manager::_()->_info_handler.addSub(sub, info);
    }
    
    static void removePub(PublisherBase* pub, const PublisherInfo& info) {
        Manager::_()->_info_handler.removePub(pub, info);
    }
    
    static void removeSub(SubscriberBase* sub, const SubscriberInfo& info) {
        Manager::_()->_info_handler.removeSub(sub, info);
    }
    
    static void updatePub(PublisherBase* pub, const PublisherInfo& info) {
        Manager::_()->_info_handler.updatePub(pub, info);
    }
    
    static void updateSub(SubscriberBase* sub, const SubscriberInfo& info) {
        Manager::_()->_info_handler.updateSub(sub, info);
    }

private:
   const std::string _name;
   const std::string _namespace;
   const int _pid;
   const std::string _uuid;
   
   InfoHandler _info_handler;
};

// Compatibility namespace
namespace manager {
    inline void init(const std::string& name, const std::string& ns) { Manager::init(name, ns); }
    inline std::string uuid() { return Manager::uuid(); }
    inline int pid() { return Manager::pid(); }
    inline std::string node_name() { return Manager::node_name(); }
    inline std::string node_ns() { return Manager::node_ns(); }
    
    // Template wrappers to handle pointer types automatically
    template<typename T>
    inline void addSub(T* sub, const SubscriberInfo& info) {
        Manager::addSub(sub, info);
    }
    
    template<typename T>
    inline void removeSub(T* sub, const SubscriberInfo& info) {
        Manager::removeSub(sub, info);
    }
    
    template<typename T>
    inline void addPub(T* pub, const PublisherInfo& info) {
        Manager::addPub(pub, info);
    }
    
    template<typename T>
    inline void removePub(T* pub, const PublisherInfo& info) {
        Manager::removePub(pub, info);
    }
     
    template<typename T>
    inline void updatePub(T* pub, const PublisherInfo& info) {
        Manager::updatePub(pub, info);
    }

} // namespace manager

inline void init(const std::string& name = "", const std::string& ns = "") {
    Manager::init(name, ns);
}

} // namespace zos

#endif // __ZOS_MANAGER_H__
