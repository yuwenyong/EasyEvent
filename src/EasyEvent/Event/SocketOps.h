//
// Created by yuwenyong on 2020/11/29.
//

#ifndef EASYEVENT_EVENT_SOCKETOPS_H
#define EASYEVENT_EVENT_SOCKETOPS_H

#include "EasyEvent/Event/EvtCommon.h"


namespace EasyEvent {

    class EASY_EVENT_API SocketOps {
    public:
        static SocketType Socket(int af, int type, int protocol, std::error_code& ec);

        static SocketType Socket(int af, int type, int protocol) {
            std::error_code ec;
            auto result = Socket(af, type, protocol, ec);
            throwError(ec, "Socket");
            return result;
        }

        static const char* InetNtop(int af, const void* src, char* dest, size_t length, unsigned long scopeId,
                                    std::error_code& ec);

        static const char* InetNtop(int af, const void* src, char* dest, size_t length, unsigned long scopeId) {
            std::error_code ec;
            auto result = InetNtop(af, src, dest, length, scopeId, ec);
            throwError(ec, "InetNto");
            return result;
        }

        static int InetPton(int af, const char* src, void* dest, unsigned long* scopeId, std::error_code& ec);

        static int InetPton(int af, const char* src, void* dest, unsigned long* scopeId) {
            std::error_code ec;
            auto result = InetPton(af, src, dest, scopeId, ec);
            throwError(ec, "InetPton");
            return result;
        }

        static int GetAddrInfo(const char* host, const char* service, const addrinfo* hints, addrinfo** result,
                               std::error_code& ec);

        static int GetAddrInfo(const char* host, const char* service, const addrinfo* hints, addrinfo** result) {
            std::error_code ec;
            auto ret = GetAddrInfo(host, service, hints, result, ec);
            throwError(ec, "GetAddrInfo");
            return ret;
        }

        static void FreeAddrInfo(struct addrinfo* ai) {
            ::freeaddrinfo(ai);
        }
    };

}

#endif //EASYEVENT_EVENT_SOCKETOPS_H
