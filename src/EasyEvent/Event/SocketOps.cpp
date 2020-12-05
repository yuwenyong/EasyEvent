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