#ifndef __ZOS_POSIX_H__
#define __ZOS_POSIX_H__
#include <string>
#include <vector>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>

namespace zos{

const int IPV4 = 0x01<<0;
const int IPV6 = 0x01<<1;

inline std::string exec(const char* cmd) {
    // Simple implementation using popen
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    while (fgets(buffer, 128, pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

inline std::string exec(const std::string& cmd) {
    return exec(cmd.c_str());
}

inline int getProcessId() {
    return getpid();
}

inline std::string getProcessName() {
    // Reading /proc/self/comm on Linux
    char buffer[256];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer); // Full path
    }
    return "unknown";
}
inline std::string generateUUID() {
    // Basic UUID fallback
    return "00000000-0000-0000-0000-000000000000"; 
}

inline std::vector<std::string> getNetworkInterfacesIP(const int ipType = IPV4) {
    std::vector<std::string> ips;
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;      

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (std::string(addressBuffer) != "127.0.0.1")
                ips.push_back(std::string(addressBuffer)); 
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            // ips.push_back(std::string(addressBuffer)); // Ignore IPv6 for now unless requested
        } 
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return ips;
}

inline bool checkPortUsage(const int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return true; // Assume used if error? No, assume available?
    struct sockaddr_in current;
    current.sin_family = AF_INET;
    current.sin_addr.s_addr = INADDR_ANY;
    current.sin_port = htons(port);
    int res = bind(sock, (struct sockaddr *)&current, sizeof(current));
    close(sock);
    return res < 0; // If bind failed, port is used
}

inline bool checkPortUsage(const std::string& address, const int port) {
    return checkPortUsage(port); // Simplified ignoring address
}

} // namespace zos
#endif // __ZOS_POSIX_H__
