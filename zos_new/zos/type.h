#ifndef __ZOS_TYPE_H__
#define __ZOS_TYPE_H__

#include <functional>
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <fmt/core.h>
#include <asio.hpp>

namespace zos {
    class Data; // Forward declaration
namespace type {
    using callback_type = std::function<void(const zos::Data&)>;
    using socket_callback_type = std::function<void(const void*, size_t)>;
    using socket_ep_callback_type = std::function<void(const asio::ip::udp::endpoint&, const void*, size_t)>;
}

enum class InfoFrom { UNASSIGNED, SELF, OUTER };
enum class CommLevel { Manual = 0, Default = 100, Process = 100, Localhost = 200, LocalNetwork = 300 };

struct HandleResult {
    bool success = false;
    std::string msg = "";
    HandleResult() = default;
    HandleResult(bool success, const std::string& msg="") : success(success), msg(msg) {}
};

struct ProcessInfo {
    std::string uuid = "";
    int pid = 0;
    std::string name = "";
    std::string ns = "";
    std::string ip = "";
    int port = 0;
    std::string key = "";
    ProcessInfo() = default;
    ProcessInfo(const std::string& uuid, const int pid, const std::string& name, const std::string& ns, const std::string& address, const int port)
        : uuid(uuid), pid(pid), name(name), ns(ns), ip(address), port(port), key(fmt::format("PROC:{}({}:{})", name, uuid, pid)) {}
};

struct SubscriberInfo {
    std::string ip = "";
    int port = 0;
    std::string puuid = ""; // process uuid
    std::string uuid = "";  // unique id for subscriber
    int pid = 0;
    std::string ns = "default";
    std::string name = "";
    std::string msg_name = "";
    std::string node_name = "";
    std::string node_ns = "";
    // Using void* to avoid circular dependency with SubscriberBase
    void* ptr = nullptr; 
    InfoFrom origin = InfoFrom::UNASSIGNED;
    CommLevel commLevel = CommLevel::Default;
    
    SubscriberInfo() = default;
    SubscriberInfo(const std::string& ip, const int port, const std::string& puuid, const int pid,
                  const std::string& node_name, const std::string& node_ns, const std::string& ns,
                  const std::string& uuid, const std::string& name, const std::string& msg_name,
                  void* ptr = nullptr, const InfoFrom origin = InfoFrom::UNASSIGNED)
        : ip(ip), port(port), puuid(puuid), pid(pid), node_name(node_name), node_ns(node_ns),
          ns(ns), uuid(uuid), name(name), msg_name(msg_name), ptr(ptr), origin(origin) {}

    bool operator<(const SubscriberInfo& rhs) const {
        return uuid < rhs.uuid;
    }
};

struct PublisherInfo {
    std::string ip = "";
    std::string puuid = "";
    std::string uuid = "";
    int pid = 0;
    std::string ns = "default";
    std::string name = "";
    std::string msg_name = "";
    std::string node_name = "";
    std::string node_ns = "";
    void* ptr = nullptr;
    InfoFrom origin = InfoFrom::UNASSIGNED;
    CommLevel commLevel = CommLevel::Default;
    std::set<SubscriberInfo> subs;
    
    PublisherInfo() = default;
    PublisherInfo(const std::string& ip, const std::string& puuid, const int pid,
                 const std::string& node_name, const std::string& node_ns, const std::string& ns,
                 const std::string& uuid, const std::string& name, const std::string& msg_name,
                 void* const ptr = nullptr, const InfoFrom origin = InfoFrom::UNASSIGNED)
        : ip(ip), puuid(puuid), pid(pid), node_name(node_name), node_ns(node_ns),
          ns(ns), uuid(uuid), name(name), msg_name(msg_name), subs({}), ptr(ptr), origin(origin) {}

    bool addSubs(const SubscriberInfo& subs_info) {
        subs.insert(subs_info);
        return true;
    }
    bool removeSubs(const SubscriberInfo& subs_info) {
        subs.erase(subs_info);
        return true;
    }
    bool hasSub(const SubscriberInfo& sub_info) const {
        return subs.find(sub_info) != subs.end();
    }
};

template<typename TPub, typename TSub=TPub>
struct Info {
    std::map<TPub, PublisherInfo> pubs;
    std::map<TSub, SubscriberInfo> subs;
    
    HandleResult addPub(TPub ptr, const PublisherInfo& pub) {
        return updatePub(ptr, pub);
    }
    
    HandleResult addSub(TSub ptr, const SubscriberInfo& sub) {
        return updateSub(ptr, sub);
    }
    
    HandleResult removePub(TPub ptr, const PublisherInfo& pub) {
        if (pubs.find(ptr) == pubs.end()) {
            return {false, "publisher not found"};
        }
        pubs.erase(ptr);
        return {true, "success"};
    }
    
    HandleResult removeSub(TSub ptr, const SubscriberInfo& sub) {
        if (subs.find(ptr) == subs.end()) {
            return {false, "subscriber not found"};
        }
        subs.erase(ptr);
        return {true, "success"};
    }
    
    HandleResult updatePub(TPub ptr, const PublisherInfo& pub) {
        pubs[ptr] = pub;
        return {true, "success"};
    }
    
    HandleResult updateSub(TSub ptr, const SubscriberInfo& sub) {
        subs[ptr] = sub;
        return {true, "success"};
    }
    
    bool checkSub(TSub ptr) {
        return subs.find(ptr) != subs.end();
    }
    
    bool checkPub(TPub ptr) {
        return pubs.find(ptr) != pubs.end();
    }
    
    SubscriberInfo& getSub(TSub ptr) {
        return subs[ptr];
    }
    
    PublisherInfo& getPub(TPub ptr) {
        return pubs[ptr];
    }
};

} // namespace zos
#endif // __ZOS_TYPE_H__
