//
// Created by yuwenyong on 2020/11/28.
//

#include "EasyEvent/Event/Address.h"
#include "EasyEvent/Event/SocketOps.h"


EasyEvent::Address::Address(const char *addr, unsigned short port, ProtocolSupport protocol)
    : Address() {
    if (protocol == EnableIPv4 || (protocol == EnableBoth && isAddressIPv4(addr))) {
        initAddressIPv4(addr, port);
    } else if (protocol == EnableIPv6 || (protocol == EnableBoth && isAddressIPv6(addr))) {
        initAddressIPv6(addr, port);
    } else {
        std::error_code ec = std::make_error_code(std::errc::invalid_argument);
        doThrowError(ec);
    }
}

bool EasyEvent::Address::operator<(const Address &rhs) const {
    if (_addr.saStorage.ss_family < rhs._addr.saStorage.ss_family) {
        return true;
    } else if (_addr.saStorage.ss_family > rhs._addr.saStorage.ss_family) {
        return false;
    }
    if (_addr.saStorage.ss_family == AF_UNSPEC) {
        return false;
    }
    if (_addr.saStorage.ss_family == AF_INET) {
        if (_addr.saIn.sin_port < rhs._addr.saIn.sin_port) {
            return true;
        } else if (_addr.saIn.sin_port > rhs._addr.saIn.sin_port) {
            return false;
        }

        if (_addr.saIn.sin_addr.s_addr < rhs._addr.saIn.sin_addr.s_addr) {
            return true;
        } else {
            return false;
        }
    } else {
        if (_addr.saIn6.sin6_scope_id < rhs._addr.saIn6.sin6_scope_id) {
            return true;
        } else if (_addr.saIn6.sin6_scope_id > rhs._addr.saIn6.sin6_scope_id) {
            return false;
        }
        if (_addr.saIn6.sin6_port < rhs._addr.saIn6.sin6_port) {
            return true;
        } else if (_addr.saIn6.sin6_port > rhs._addr.saIn6.sin6_port) {
            return false;
        }
        int res = memcmp(&_addr.saIn6.sin6_addr, &rhs._addr.saIn6.sin6_addr, sizeof(in6_addr));
        if (res < 0) {
            return true;
        } else {
            return false;
        }
    }
}

bool EasyEvent::Address::operator==(const Address &rhs) const {
    if (_addr.saStorage.ss_family != rhs._addr.saStorage.ss_family) {
        return false;
    }
    if (_addr.saStorage.ss_family == AF_UNSPEC) {
        return true;
    }
    if (_addr.saStorage.ss_family == AF_INET) {
        if (_addr.saIn.sin_port != rhs._addr.saIn.sin_port) {
            return false;
        }

        if (_addr.saIn.sin_addr.s_addr != rhs._addr.saIn.sin_addr.s_addr) {
            return false;
        }
        return true;
    } else {
        if (_addr.saIn6.sin6_scope_id != rhs._addr.saIn6.sin6_scope_id) {
            return false;
        }
        if (_addr.saIn6.sin6_port != rhs._addr.saIn6.sin6_port) {
            return false;
        }
        int res = memcmp(&_addr.saIn6.sin6_addr, &rhs._addr.saIn6.sin6_addr, sizeof(in6_addr));
        if (res != 0) {
            return false;
        }
        return true;
    }
}

std::string EasyEvent::Address::getAddrString(std::error_code &ec) const {
    if (_addr.saStorage.ss_family != AF_INET && _addr.saStorage.ss_family != AF_INET6) {
        ec = std::make_error_code(std::errc::not_supported);
        return {};
    }
    if (_addr.saStorage.ss_family == AF_INET) {
        char addrStr[MaxAddrV4StrLen];
        const char* addr = SocketOps::InetNtop(AF_INET, &_addr.saIn.sin_addr, addrStr, MaxAddrV4StrLen, 0, ec);
        if (addr == nullptr) {
            return {};
        }
        return addr;
    } else {
        char addrStr[MaxAddrV6StrLen];
        const char* addr = SocketOps::InetNtop(AF_INET6, &_addr.saIn6.sin6_addr, addrStr, MaxAddrV6StrLen, 0, ec);
        if (addr == nullptr) {
            return {};
        }
        return addr;
    }
}

std::vector<EasyEvent::Address> EasyEvent::Address::loopbackAddresses(
        ProtocolSupport protocol, unsigned short port) {

    std::vector<Address> result;
    Address addr;
    if (protocol != EnableIPv4) {
        addr._addr.saIn6.sin6_family = AF_INET6;
        addr._addr.saIn6.sin6_port = htons(port);
        addr._addr.saIn6.sin6_addr = in6addr_loopback;
        result.push_back(addr);
    }
    if (protocol != EnableIPv6) {
        addr._addr.saIn.sin_family = AF_INET;
        addr._addr.saIn.sin_port = htons(port);
        addr._addr.saIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        result.push_back(addr);
    }
    return result;
}

std::vector<EasyEvent::Address> EasyEvent::Address::anyAddresses(ProtocolSupport protocol, unsigned short port) {
    std::vector<Address> result;
    Address addr;
    if (protocol != EnableIPv4) {
        addr._addr.saIn6.sin6_family = AF_INET6;
        addr._addr.saIn6.sin6_port = htons(port);
        addr._addr.saIn6.sin6_addr = in6addr_any;
        result.push_back(addr);
    }
    if (protocol != EnableIPv6) {
        addr._addr.saIn.sin_family = AF_INET;
        addr._addr.saIn.sin_port = htons(port);
        addr._addr.saIn.sin_addr.s_addr = htonl(INADDR_ANY);
        result.push_back(addr);
    }
    return result;
}

bool EasyEvent::Address::isAddressIPv4(const char* addr) {
    std::error_code ec;
    in_addr addr4;
    return SocketOps::InetPton(AF_INET, addr, &addr4, nullptr, ec) > 0;
}

bool EasyEvent::Address::isAddressIPv6(const char* addr) {
    std::error_code ec;
    in6_addr addr6;
    return SocketOps::InetPton(AF_INET6, addr, &addr6, nullptr, ec) > 0;
}

void EasyEvent::Address::initAddressIPv4(const char *addr, unsigned short port) {
    _addr.saIn.sin_family = AF_INET;
    _addr.saIn.sin_port = htons(port);
    if (addr != nullptr) {
        SocketOps::InetPton(AF_INET, addr, &_addr.saIn.sin_addr, nullptr);
    } else {
        _addr.saIn.sin_addr.s_addr = htonl(INADDR_ANY);
    }
}

void EasyEvent::Address::initAddressIPv6(const char *addr, unsigned short port) {
    unsigned long scopeId;
    _addr.saIn6.sin6_family = AF_INET6;
    _addr.saIn6.sin6_port = htons(port);
    if (addr != nullptr) {
        SocketOps::InetPton(AF_INET6, addr, &_addr.saIn6.sin6_addr, &scopeId);
        _addr.saIn6.sin6_scope_id = (uint32_t)scopeId;
    } else {
        _addr.saIn6.sin6_addr = in6addr_any;
    }
}

EasyEvent::Address EasyEvent::Address::makeAddressIPv4(const char *str, unsigned short port, std::error_code &ec) {
    IPv4Bytes bytes;
    if (SocketOps::InetPton(AF_INET, str, &bytes[0], 0, ec) <= 0) {
        return {};
    }
    return Address(bytes, port);
}

EasyEvent::Address EasyEvent::Address::makeAddressIPv6(const char *str, unsigned short port, std::error_code &ec) {
    IPv6Bytes bytes;
    unsigned long scopeId = 0;
    if (SocketOps::InetPton(AF_INET6, str, &bytes[0], &scopeId, ec) <= 0) {
        return {};
    }
    return Address(bytes, scopeId, port);
}

EasyEvent::Address EasyEvent::Address::makeAddress(const char *str, unsigned short port, std::error_code &ec) {
    auto addr6 = makeAddressIPv6(str, port, ec);
    if (!ec) {
        return addr6;
    }
    auto addr4 = makeAddressIPv4(str, port, ec);
    if (!ec) {
        return addr4;
    }
    return {};
}

