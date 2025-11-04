// net_compat.h
#pragma once
#include <string>
#include <system_error>

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm")

using socket_t = SOCKET;
inline constexpr socket_t INVALID_SOCKET_T = INVALID_SOCKET;
inline int last_net_error() { return WSAGetLastError(); }
inline void closesocket_wrap(socket_t s) { ::closesocket(s); }
inline bool net_init_once(std::string* err = nullptr) {
    WSADATA d{};
    if (int r = ::WSAStartup(MAKEWORD(2, 2), &d); r != 0) {
        if (err) *err = "WSAStartup failed: " + std::to_string(r);
        return false;
    }
    return true;
}
inline void net_cleanup() { ::WSACleanup(); }
inline int shutdown_wr(socket_t s) { return ::shutdown(s, SD_SEND); }
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#define SOCKET_ERROR (-1) 
#define INVALID_SOCKET (-1)
#define SD_RECEIVE SHUT_RD
#define SD_SEND    SHUT_WR
#define SD_BOTH    SHUT_RDWR
using socket_t = int;
inline constexpr socket_t INVALID_SOCKET_T = -1;
inline int last_net_error() { return errno; }
inline void closesocket_wrap(socket_t s) { ::close(s); }
inline bool net_init_once(std::string* /*err*/= nullptr) { return true; }
inline void net_cleanup() {}
inline int shutdown_wr(socket_t s) { return ::shutdown(s, SHUT_WR); }
#define closesocket close
#endif

inline std::error_code net_ec(int ev) {
#ifdef _WIN32
    return { ev, std::system_category() };
#else
    return { ev, std::generic_category() };
#endif
}

inline int set_nonblocking(socket_t s) {
#ifdef _WIN32
    unsigned long on = 1; return ::ioctlsocket(s, FIONBIO, &on);
#else
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
}
