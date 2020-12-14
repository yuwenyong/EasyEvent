//
// Created by yuwenyong on 2020/11/29.
//

#include "EasyEvent/Event/SocketOps.h"

namespace EasyEvent {

    inline void clearLastError() {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
        WSASetLastError(0);
#else
        errno = 0;
#endif
    }

    inline void getLastError(std::error_code& ec, bool isError) {
        if (!isError) {
            ec.assign(0, ec.category());
        } else {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
            ec = std::error_code(WSAGetLastError(), getSocketErrorCategory());
#else
            ec = std::error_code(errno, getSocketErrorCategory());
#endif
        }
    }

    inline std::error_code translateAddrInfoError(int error) {
        switch (error) {
            case 0:
                return {};
            case EAI_AGAIN:
                return NetDBErrors::HostNotFoundTryAgain;
            case EAI_BADFLAGS:
                return SocketErrors::InvalidArgument;
            case EAI_FAIL:
                return NetDBErrors::NoRecovery;
            case EAI_FAMILY:
                return SocketErrors::AddressFamilyNotSupported;
            case EAI_MEMORY:
                return SocketErrors::NoMemory;
            case EAI_NONAME:
#if defined(EAI_ADDRFAMILY)
            case EAI_ADDRFAMILY:
#endif
#if defined(EAI_NODATA) && (EAI_NODATA != EAI_NONAME)
            case EAI_NODATA:
#endif
                return NetDBErrors::HostNotFound;
            case EAI_SERVICE:
                return AddrInfoErrors::ServiceNotFound;
            case EAI_SOCKTYPE:
                return AddrInfoErrors::SocketTypeNotSupported;
            default: // Possibly the non-portable EAI_SYSTEM.
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
                return std::error_code(WSAGetLastError(), getSocketErrorCategory());
#else
                return std::error_code(errno, getSocketErrorCategory());
#endif
        }
    }

}


SocketType EasyEvent::SocketOps::Socket(int af, int type, int protocol, std::error_code &ec) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    SocketType s = ::WSASocketW(af, type, protocol, 0, 0, WSA_FLAG_OVERLAPPED);
    getLastError(ec, s == InvalidSocket);
    if (s == InvalidSocket) {
        return s;
    }
    if (af == AF_INET6) {
        DWORD optval = 0;
        ::setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&optval), sizeof(optval));
    }
    return s;
#elif EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_APPLE || EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_UNIX
    SocketType s = ::socket(af, type, protocol);
    getLastError(ec, s < 0);

    int optval = 1;
    int result = ::setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
    getLastError(ec, result != 0);
    if (result != 0) {
        ::close(s);
        return InvalidSocket;
    }
    return s;
#else
    int s = ::socket(af, type, protocol);
    getLastError(ec, s < 0);
    return s;
#endif
}

int EasyEvent::SocketOps::Shutdown(SocketType s, int what, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    int result = ::shutdown(s, what);
    getLastError(ec, result != 0);
    return result;
}

int EasyEvent::SocketOps::Close(SocketType s, bool destruction, std::error_code &ec) {
    int result = 0;
    if (s != InvalidSocket) {
        if (destruction) {
            ::linger opt;
            opt.l_onoff = 0;
            opt.l_linger = 0;
            std::error_code ignoredError;
            SocketOps::SetSockOpt(s, SOL_SOCKET, SO_LINGER, &opt, sizeof(opt), ignoredError);
        }
    }
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    result = ::closesocket(s);
#else
    result = ::close(s);
#endif
    getLastError(ec, result != 0);
    if (result != 0 && (ec == SocketErrors::WouldBlock || ec == SocketErrors::TryAgain)) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
        IoctlArgType arg = 0;
        ::ioctlsocket(s, FIONBIO, &arg);
        result = ::closesocket(s);
#else
        IoctlArgType arg = 0;
        ::ioctl(s, FIONBIO, &arg);
        result = ::close(s);
#endif
        getLastError(ec, result != 0);
    }
    return result;
}

int EasyEvent::SocketOps::SetSockOpt(SocketType s, int level, int optname, const void *optval, std::size_t optlen,
                                     std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    int result = ::setsockopt(s, level, optname, (const char*)optval, (SockLenType)optlen);
    getLastError(ec, result != 0);
    return result;
}

int EasyEvent::SocketOps::GetSockOpt(SocketType s, int level, int optname, void *optval, std::size_t *optlen,
                                     std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    SockLenType tmpOptlen = (SockLenType)*optlen;
    int result = ::getsockopt(s, level, optname, (char*)optval, &tmpOptlen);
    *optlen = (std::size_t)tmpOptlen;
    getLastError(ec, result != 0);
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    if (result != 0 && level == IPPROTO_IPV6 && optname == IPV6_V6ONLY
        && ec.value() == WSAENOPROTOOPT && *optlen == sizeof(DWORD)) {
        *static_cast<DWORD*>(optval) = 1;
        ec.assign(0, ec.category());
    }
#elif EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_LINUX
    if (result == 0 && level == SOL_SOCKET && *optlen == sizeof(int)
        && (optname == SO_SNDBUF || optname == SO_RCVBUF)) {
        *static_cast<int*>(optval) /= 2;
    }
#endif
    return result;
}

int EasyEvent::SocketOps::Ioctl(SocketType s, int cmd, IoctlArgType *arg, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    int result = ::ioctlsocket(s, cmd, arg);
#elif EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_APPLE || EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_UNIX
    int result = ::ioctl(s, static_cast<unsigned int>(cmd), arg);
#else
    int result = ::ioctl(s, static_cast<unsigned long>(cmd), arg);
#endif
    getLastError(ec, result < 0);
    return result;
}

int EasyEvent::SocketOps::Bind(SocketType s, const sockaddr *addr, size_t addrLen, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    int result = ::bind(s, addr, (SockLenType)addrLen);
    getLastError(ec, result != 0);
    return result;
}

int EasyEvent::SocketOps::Listen(SocketType s, int backlog, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    int result = ::listen(s, backlog);
    getLastError(ec, result != 0);
    return result;
}

SocketType EasyEvent::SocketOps::Accept(SocketType s, sockaddr *addr, size_t *addrLen, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return InvalidSocket;
    }
    SockLenType tmpAddrLen = addrLen ? (SockLenType)*addrLen : 0;
    SocketType newSock = ::accept(s, addr, addrLen ? &tmpAddrLen : nullptr);
    if (addrLen) {
        *addrLen = (size_t)tmpAddrLen;
    }
    getLastError(ec, newSock == InvalidSocket);
    if (newSock == InvalidSocket) {
        return newSock;
    }
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_APPLE || EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_UNIX
    int optval = 1;
    int result = ::setsockopt(newSock, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
    getLastError(ec, result != 0);
    if (result != 0) {
        ::close(newSock);
        return InvalidSocket;
    }
#endif
    ec.assign(0, ec.category());
    return newSock;
}

int EasyEvent::SocketOps::Connect(SocketType s, const sockaddr *addr, size_t addrLen, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    int result = ::connect(s, addr, (SockLenType)addrLen);
    getLastError(ec, result != 0);
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_LINUX
    if (result != 0 && ec == SocketErrors::TryAgain) {
        ec = SocketErrors::NoBufferSpace;
    }
#endif
    return result;
}

int EasyEvent::SocketOps::GetPeerName(SocketType s, sockaddr *addr, size_t *addrLen, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    SockLenType tmpAddrLen = (SockLenType)*addrLen;
    int result = ::getpeername(s, addr, &tmpAddrLen);
    *addrLen = (size_t)tmpAddrLen;
    getLastError(ec, result != 0);
    return result;
}

int EasyEvent::SocketOps::GetSockName(SocketType s, sockaddr *addr, size_t *addrLen, std::error_code &ec) {
    if (s == InvalidSocket) {
        ec = SocketErrors::BadDescriptor;
        return SocketErrorRetVal;
    }
    SockLenType tmpAddrLen = (SockLenType)*addrLen;
    int result = ::getsockname(s, addr, &tmpAddrLen);
    *addrLen = (size_t)tmpAddrLen;
    getLastError(ec, result != 0);
    return result;
}

ssize_t EasyEvent::SocketOps::Send(SocketType s, const void *data, size_t size, int flags, std::error_code &ec) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    WSABUF buf;
    buf.buf = const_cast<char*>(static_cast<const char*>(data));
    buf.len = static_cast<ULONG>(size);
    DWORD bytesTransferred = 0;
    DWORD sendFlags = flags;
    int result = ::WSASend(s, &buf, 1, &bytesTransferred, sendFlags, 0, 0);
    getLastError(ec, true);
    if (ec.value() == ERROR_NETNAME_DELETED) {
        ec = SocketErrors::ConnectionReset;
    } else if (ec.value() == ERROR_PORT_UNREACHABLE) {
        ec = SocketErrors::ConnectionRefused;
    }
    if (result != 0) {
        return SocketErrorRetVal;
    }
    ec.assign(0, ec.category());
    return (ssize_t)bytesTransferred;
#else
#   if ASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_LINUX
    flags |= MSG_NOSIGNAL;
#   endif
    ssize_t result = ::send(s, static_cast<const char*>(data), size, flags);
    getLastError(ec, result < 0);
    return result;
#endif
}

ssize_t EasyEvent::SocketOps::Recv(SocketType s, void *data, size_t size, int flags, std::error_code &ec) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    WSABUF buf;
    buf.buf = const_cast<char*>(static_cast<const char*>(data));
    buf.len = static_cast<ULONG>(size);
    DWORD bytesTransferred = 0;
    DWORD recvFlags = flags;
    int result = ::WSARecv(s, &buf, 1, &bytesTransferred, &recvFlags, 0, 0);
    getLastError(ec, true);
    if (ec.value() == ERROR_NETNAME_DELETED) {
        ec = SocketErrors::ConnectionReset;
    } else if (ec.value() == ERROR_PORT_UNREACHABLE) {
        ec = SocketErrors::ConnectionRefused;
    } else if (ec.value() == WSAEMSGSIZE || ec.value() == ERROR_MORE_DATA) {
        result = 0;
    }
    if (result != 0) {
        return SocketErrorRetVal;
    }
    ec.assign(0, ec.category());
    return (ssize_t)bytesTransferred;
#else
    ssize_t result = ::recv(s, static_cast<char*>(data), size, flags);
    getLastError(ec, result < 0);
    return result;
#endif
}

const char* EasyEvent::SocketOps::InetNtop(int af, const void *src, char *dest, size_t length, unsigned long scopeId,
                                           std::error_code &ec) {
    clearLastError();
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    if (af != AF_INET && af != AF_INET6) {
        ec = SocketErrors::AddressFamilyNotSupported;
        return nullptr;
    }

    union {
        sockaddr base;
        sockaddr_storage storage;
        sockaddr_in v4;
        sockaddr_in6 v6;
    } address;
    DWORD addressLength;
    if (af == AF_INET) {
        addressLength = sizeof(sockaddr_in);
        address.v4.sin_family = AF_INET;
        address.v4.sin_port = 0;
        memcpy(&address.v4.sin_addr, src, sizeof(in_addr));
    }
    else {
        addressLength = sizeof(sockaddr_in6);
        address.v6.sin6_family = AF_INET6;
        address.v6.sin6_port = 0;
        address.v6.sin6_flowinfo = 0;
        address.v6.sin6_scope_id = scopeId;
        memcpy(&address.v6.sin6_addr, src, sizeof(in6_addr));
    }

    DWORD stringLength = static_cast<DWORD>(length);
    int result = ::WSAAddressToStringA(&address.base, addressLength, 0, dest, &stringLength);
    getLastError(ec, true);
    if (result != SocketErrorRetVal) {
        ec.assign(0, ec.category());
    } else if (result == SocketErrorRetVal && !ec) {
        ec = UserErrors::InvalidArgument;
    }
    return result == SocketErrorRetVal ? nullptr : dest;
#else
    const char* result = ::inet_ntop(af, src, dest, (socklen_t)length);
    getLastError(ec, true);
    if (result == nullptr && !ec) {
        ec = UserErrors::InvalidArgument;
    }
    if (result != nullptr && af == AF_INET6 && scopeId != 0) {
        char ifName[(IF_NAMESIZE > 21 ? IF_NAMESIZE : 21) + 1] = "%";
        const in6_addr* ipv6Addr = static_cast<const in6_addr*>(src);
        bool isLinkLocal =((ipv6Addr->s6_addr[0] == 0xfe) && ((ipv6Addr->s6_addr[1] & 0xc0) == 0x80));
        bool isMulticastLinkLocal = ((ipv6Addr->s6_addr[0] == 0xff) && ((ipv6Addr->s6_addr[1] & 0x0f) == 0x02));
        if ((!isLinkLocal && !isMulticastLinkLocal)
            || if_indextoname(static_cast<unsigned>(scopeId), ifName + 1) == nullptr) {
            std::sprintf(ifName + 1, "%lu", scopeId);
        }
        std::strcat(dest, ifName);
    }
    return result;
#endif
}

int EasyEvent::SocketOps::InetPton(int af, const char *src, void *dest, unsigned long* scopeId, std::error_code &ec) {
    clearLastError();
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    if (af != AF_INET && af != AF_INET6) {
        ec = SocketErrors::AddressFamilyNotSupported;
        return -1;
    }

    union {
        sockaddr base;
        sockaddr_storage storage;
        sockaddr_in v4;
        sockaddr_in6 v6;
    } address;
    int addressLength = sizeof(sockaddr_storage);
    int result = ::WSAStringToAddressA(const_cast<char*>(src), af, 0, &address.base, &addressLength);
    getLastError(ec, true);
    if (af == AF_INET) {
        if (result != SocketErrorRetVal) {
            std::memcpy(dest, &address.v4.sin_addr, sizeof(in_addr));
            ec.assign(0, ec.category());
        } else if (std::strcmp(src, "255.255.255.255") == 0) {
            static_cast<in_addr*>(dest)->s_addr = INADDR_NONE;
            ec.assign(0, ec.category());
        }
    } else {
        if (result != SocketErrorRetVal) {
            std::memcpy(dest, &address.v6.sin6_addr, sizeof(in6_addr));
            if (scopeId) {
                *scopeId = address.v6.sin6_scope_id;
            }
            ec.assign(0, ec.category());
        }
    }
    if (result == SocketErrorRetVal && !ec) {
        ec = UserErrors::InvalidArgument;
    }
    if (result != SocketErrorRetVal) {
        ec.assign(0, ec.category());
    }
    return result == SocketErrorRetVal ? -1 : 1;
#else
    const bool isV6 = (af == AF_INET6);
    const char* ifName = isV6 ? std::strchr(src, '%') : nullptr;
    char srcBuf[MaxAddrV6StrLen + 1];
    const char* srcPtr = src;
    if (ifName != nullptr) {
        if (ifName - src > MaxAddrV6StrLen) {
            ec = UserErrors::InvalidArgument;
            return 0;
        }
        std::memcpy(srcBuf, src, (size_t)(ifName - src));
        srcBuf[ifName - src] = '\0';
        srcPtr = srcBuf;
    }

    int result = ::inet_pton(af, srcPtr, dest);
    getLastError(ec, true);
    if (result <= 0 && !ec) {
        ec = UserErrors::InvalidArgument;
    }
    if (result > 0 && isV6 && scopeId) {
        *scopeId = 0;
        if (ifName != nullptr)
        {
            in6_addr* ipv6Addr = static_cast<in6_addr*>(dest);
            bool isLinkLocal = ((ipv6Addr->s6_addr[0] == 0xfe) && ((ipv6Addr->s6_addr[1] & 0xc0) == 0x80));
            bool isMulticastLinkLocal = ((ipv6Addr->s6_addr[0] == 0xff) && ((ipv6Addr->s6_addr[1] & 0x0f) == 0x02));
            if (isLinkLocal || isMulticastLinkLocal) {
                *scopeId = if_nametoindex(ifName + 1);
            }

            if (*scopeId == 0) {
                *scopeId = (unsigned long)std::atoi(ifName + 1);
            }
        }
    }
    return result;
#endif
}

int EasyEvent::SocketOps::GetAddrInfo(const char *host, const char *service, const addrinfo *hints, addrinfo **result,
                                      std::error_code &ec) {
    host = (host && *host) ? host : nullptr;
    service = (service && *service) ? service : nullptr;
    clearLastError();
    int error = ::getaddrinfo(host, service, hints, result);
    ec = translateAddrInfoError(error);
    return error;
}