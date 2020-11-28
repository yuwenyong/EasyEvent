//
// Created by yuwenyong on 2020/11/28.
//

#include "EasyEvent/Event/Address.h"


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

std::vector<EasyEvent::Address> EasyEvent::Address::getLoopbackAddresses(
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

std::vector<EasyEvent::Address> EasyEvent::Address::getWildAddresses(ProtocolSupport protocol, unsigned short port) {
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