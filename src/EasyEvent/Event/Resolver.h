//
// Created by yuwenyong on 2020/12/2.
//

#ifndef EASYEVENT_EVENT_RESOLVER_H
#define EASYEVENT_EVENT_RESOLVER_H

#include "EasyEvent/Event/Event.h"
#include "EasyEvent/Event/Address.h"
#include "EasyEvent/Common/Task.h"

namespace EasyEvent {

    class IOLoop;

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


    class EASY_EVENT_API ResolveQuery: public std::enable_shared_from_this<ResolveQuery> {
    public:
        using CallbackType = Task<void(std::vector<Address>, std::error_code)>;

        ResolveQuery(IOLoop* ioLoop, std::string host, unsigned short port, ProtocolSupport protocol, bool preferIPv6,
                     CallbackType&& callback)
            : _ioLoop(ioLoop)
            , _host(std::move(host))
            , _port(port)
            , _protocol(protocol)
            , _preferIPv6(preferIPv6)
            , _callback(std::move(callback)) {
            Assert(_callback);
        }

        bool doResolve();

        bool doResolveBackground();

        bool cancel();

    protected:
        void onResolved();

        void onCallback();

        IOLoop* _ioLoop;
        std::string _host;
        unsigned short _port;
        ProtocolSupport _protocol;
        bool _preferIPv6;
        CallbackType _callback;
        bool _cancelled{false};
        std::vector<Address> _addrs;
        std::error_code _error;
    };

    using ResolveQueryPtr = std::shared_ptr<ResolveQuery>;

    class ResolveHandle {
    public:
        explicit ResolveHandle(const ResolveQueryPtr& query): _query(query) {}

        void cancel() {
            auto query = _query.lock();
            if (query) {
                query->cancel();
            }
        }
    private:
        std::weak_ptr<ResolveQuery> _query;
    };

}

#endif //EASYEVENT_EVENT_RESOLVER_H
