//
// Created by yuwenyong on 2020/12/2.
//

#include "EasyEvent/Event/Resolver.h"
#include "EasyEvent/Event/SocketOps.h"


std::vector<EasyEvent::Address> EasyEvent::Resolver::getAddresses(const std::string &host, unsigned short port,
                                                                  ProtocolSupport protocol, bool preferIPv6,
                                                                  bool canBlock, std::error_code &ec) {
    std::vector<Address> result;
    if (host.empty()) {
        result = Address::loopbackAddresses(protocol, port);
        sortAddresses(result, protocol, preferIPv6);
        ec.assign(0, ec.category());
        return result;
    }

    struct addrinfo* info = nullptr;
    struct addrinfo hints = {};
    std::shared_ptr<void> localFree(nullptr, [info](void*) {
        if (info) {
            SocketOps::FreeAddrInfo(info);
        }
    });

    int retry = 3;
    if (protocol == EnableIPv4) {
        hints.ai_family = AF_INET;
    } else if (protocol == EnableIPv6) {
        hints.ai_family = AF_INET6;
    } else {
        hints.ai_family = AF_UNSPEC;
    }

    if (!canBlock) {
        hints.ai_flags = AI_NUMERICHOST;
    }

    do {
        SocketOps::GetAddrInfo(host.c_str(), nullptr, &hints, &info, ec);
    } while (info == nullptr && ec == NetDBErrors::HostNotFoundTryAgain && --retry >= 0);

    if (!ec) {
        for (struct addrinfo* p = info; p != nullptr; p = p->ai_next) {
            Address addr(p->ai_addr, p->ai_addrlen, p->ai_family, port);
            bool found = false;
            for (size_t i = 0; i != result.size(); ++i) {
                if (result[i] == addr) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.emplace_back(addr);
            }
        }
        sortAddresses(result, protocol, preferIPv6);
    }
    return result;
}

void EasyEvent::Resolver::sortAddresses(std::vector<Address> &addrs, ProtocolSupport protocol, bool preferIPv6) {
    if (protocol == EnableBoth) {
        if (preferIPv6) {
            std::stable_partition(addrs.begin(), addrs.end(), [](const Address& ss) { return ss.isIPv6(); });
        } else {
            std::stable_partition(addrs.begin(), addrs.end(), [](const Address& ss) { return !ss.isIPv6(); });
        }
    }
}