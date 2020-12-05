//
// Created by yuwenyong on 2020/11/29.
//

#ifndef EASYEVENT_EVENT_SOCKETOPS_H
#define EASYEVENT_EVENT_SOCKETOPS_H

#include "EasyEvent/Event/EvtCommon.h"
#include "EasyEvent/Event/Address.h"


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

        static int Shutdown(SocketType s, int what, std::error_code& ec);

        static int Shutdown(SocketType s, int what) {
            std::error_code ec;
            auto result = Shutdown(s, what, ec);
            throwError(ec, "Shutdown");
            return result;
        }

        static int Close(SocketType s, bool destruction, std::error_code& ec);

        static int Close(SocketType s, bool destruction) {
            std::error_code ec;
            auto result = Close(s, destruction, ec);
            throwError(ec, "Close");
            return result;
        }

        static int SetSockOpt(SocketType s, int level, int optname, const void* optval, std::size_t optLen,
                              std::error_code& ec);

        static int SetSockOpt(SocketType s, int level, int optname, const void* optval, std::size_t optLen) {
            std::error_code ec;
            auto result = SetSockOpt(s, level, optname, optval, optLen, ec);
            throwError(ec, "SetSocketOpt");
            return result;
        }

        static int Ioctl(SocketType s, int cmd, IoctlArgType* arg, std::error_code& ec);

        static int Ioctl(SocketType s, int cmd, IoctlArgType* arg) {
            std::error_code ec;
            auto result = Ioctl(s, cmd, arg, ec);
            throwError(ec, "Ioctl");
            return result;
        }

        static int SetReuseAddress(SocketType s, bool reuse, std::error_code& ec) {
            int flag = reuse ? 1 : 0;
            return SetSockOpt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag), ec);
        }

        static int SetReuseAddress(SocketType s, bool reuse) {
            std::error_code ec;
            auto result = SetReuseAddress(s, reuse, ec);
            throwError(ec, "SetReuseAddress");
            return result;
        }

        static int Bind(SocketType s, const sockaddr* addr, size_t addrLen, std::error_code& ec);

        static int Bind(SocketType s, const sockaddr* addr, size_t addrLen) {
            std::error_code ec;
            auto result = Bind(s, addr, addrLen, ec);
            throwError(ec, "Bind");
            return result;
        }

        static int Bind(SocketType s, const Address& addr, std::error_code& ec) {
            return Bind(s, addr.getStoragePtr(), addr.getStorageSize(), ec);
        }

        static int Bind(SocketType s, const Address& addr) {
            std::error_code ec;
            auto result = Bind(s, addr,  ec);
            throwError(ec, "Bind");
            return result;
        }

        static int Listen(SocketType s, int backlog, std::error_code& ec);

        static int Listen(SocketType s, int backlog) {
            std::error_code ec;
            auto result = Listen(s, backlog,  ec);
            throwError(ec, "Listen");
            return result;
        }

        static int GetPeerName(SocketType s, sockaddr* addr, size_t *addrLen, std::error_code& ec);

        static int GetPeerName(SocketType s, sockaddr* addr, size_t *addrLen) {
            std::error_code ec;
            auto result = GetPeerName(s, addr, addrLen, ec);
            throwError(ec, "GetPeerName");
            return result;
        }

        static int GetPeerName(SocketType s, Address& addr, std::error_code& ec) {
            struct sockaddr_storage ss;
            size_t addrLen = sizeof(ss);
            auto result = GetPeerName(s, (sockaddr*)&ss, &addrLen, ec);
            throwError(ec, "GetPeerName");
            addr = Address((const sockaddr*)&ss, addrLen);
            return result;
        }

        static int GetPeerName(SocketType s, Address& addr) {
            std::error_code ec;
            auto result = GetPeerName(s, addr, ec);
            throwError(ec, "GetPeerName");
            return result;
        }

        static int GetSockName(SocketType s, sockaddr* addr, size_t* addrLen, std::error_code& ec);

        static int GetSockName(SocketType s, sockaddr* addr, size_t* addrLen) {
            std::error_code ec;
            auto result = GetSockName(s, addr, addrLen, ec);
            throwError(ec, "GetSockName");
            return result;
        }

        static int GetSockName(SocketType s, Address& addr, std::error_code& ec) {
            struct sockaddr_storage ss;
            size_t addrLen = sizeof(ss);
            auto result = GetSockName(s, (sockaddr*)&ss, &addrLen, ec);
            throwError(ec, "GetSockName");
            addr = Address((const sockaddr*)&ss, addrLen);
            return result;
        }

        static int GetSockName(SocketType s, Address& addr) {
            std::error_code ec;
            auto result = GetSockName(s, addr, ec);
            throwError(ec, "GetSockName");
            return result;
        }

        static const char* InetNtop(int af, const void* src, char* dest, size_t length, unsigned long scopeId,
                                    std::error_code& ec);

        static const char* InetNtop(int af, const void* src, char* dest, size_t length, unsigned long scopeId) {
            std::error_code ec;
            auto result = InetNtop(af, src, dest, length, scopeId, ec);
            throwError(ec, "InetNtop");
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
