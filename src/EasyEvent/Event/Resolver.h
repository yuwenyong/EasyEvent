//
// Created by yuwenyong on 2020/12/2.
//

#ifndef EASYEVENT_EVENT_RESOLVER_H
#define EASYEVENT_EVENT_RESOLVER_H

#include "EasyEvent/Event/EvtCommon.h"
#include "EasyEvent/Event/Address.h"
#include "EasyEvent/Common/Task.h"

namespace EasyEvent {

    class EASY_EVENT_API Resolver {
    public:
        static std::vector<Address> getAddresses(const std::string& host, unsigned short port, ProtocolSupport protocol,
                                                 bool preferIPv6, bool canBlock, std::error_code& ec);

        static std::vector<Address> getAddresses(const std::string& host, unsigned short port, ProtocolSupport protocol,
                                                 bool preferIPv6, bool canBlock) {
            std::error_code ec;
            auto res = getAddresses(host, port, protocol, preferIPv6, canBlock, ec);
            throwError(ec);
            return res;
        }

        static void sortAddresses(std::vector<Address>& addrs, ProtocolSupport protocol, bool preferIPv6);
    };

    class EASY_EVENT_API ResolveEntry {
    public:
        using CallbackType = Task<void(const std::vector<Address>&, const std::error_code&)>;

        ResolveEntry(std::string host, unsigned short port, ProtocolSupport protocol, bool preferIPv6,
                     CallbackType&& callback)
            : _host(std::move(host))
            , _port(port)
            , _protocol(protocol)
            , _preferIPv6(preferIPv6)
            , _callback(std::move(callback)) {

        }

        bool cancelled() const {
            return _cancelled;
        }

    protected:
        std::string _host;
        unsigned short _port;
        ProtocolSupport _protocol;
        bool _preferIPv6;
        CallbackType _callback;
        bool _cancelled{false};
        std::vector<Address> _addrs;
        std::error_code _ec;
    };

}

#endif //EASYEVENT_EVENT_RESOLVER_H
