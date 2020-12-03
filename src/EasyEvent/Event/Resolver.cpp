//
// Created by yuwenyong on 2020/12/2.
//

#include "EasyEvent/Event/Resolver.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Event/IOLoop.h"


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


bool EasyEvent::ResolveQuery::doResolve() {
    std::error_code ec;
    _addrs = Resolver::getAddresses(_host, _port, _protocol, _preferIPv6, false, ec);
    if (!ec) {
        onResolved();
    };
    return !ec;
}

bool EasyEvent::ResolveQuery::doResolveBackground() {
    if (_cancelled) {
        return false;
    }
    _addrs = Resolver::getAddresses(_host, _port, _protocol, _preferIPv6, true, _error);
    if (_cancelled) {
        return false;
    }
    onResolved();
    return !_error;
}

bool EasyEvent::ResolveQuery::cancel() {
    if (_cancelled || !_callback) {
        return false;
    }
    _cancelled = true;
    onResolved();
    return true;
}

void EasyEvent::ResolveQuery::onResolved() {
    _ioLoop->addCallback([this, self=shared_from_this()]() {
        onCallback();
    });
}

void EasyEvent::ResolveQuery::onCallback() {
    if (_callback) {
        CallbackType callback = std::move(_callback);
        _callback = nullptr;
        if (_cancelled) {
            callback({}, UserErrors::OperationCanceled);
        } else {
            callback(std::move(_addrs), _error);
        }
    }
}