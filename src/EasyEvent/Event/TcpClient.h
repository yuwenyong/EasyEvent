//
// Created by yuwenyong on 2020/12/22.
//

#ifndef EASYEVENT_EVENT_TCPCLIENT_H
#define EASYEVENT_EVENT_TCPCLIENT_H

#include "EasyEvent/Event/Event.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Event/Connection.h"
#include "EasyEvent/Common/Task.h"

namespace EasyEvent {

    class EASY_EVENT_API TcpClient: public std::enable_shared_from_this<TcpClient> {
    public:
        TcpClient(const TcpClient&) = delete;
        TcpClient& operator=(const TcpClient&) = delete;

        explicit TcpClient(IOLoop* ioLoop)
            : _ioLoop(ioLoop ? ioLoop : IOLoop::current()) {

        }

        void connect(Task<void(ConnectionPtr, const std::error_code&)>&& callback,
                     std::string host, unsigned short port, ProtocolSupport protocol=EnableBoth,
                     Time timeout={}, size_t maxBufferSize=0,
                     std::string sourceIP="", unsigned short sourcePort=0);

    protected:
        void onResolved(std::vector<Address> addresses, std::error_code ec);

        void onTimeout() {
            _resolve.cancel();
        }

        void runConnectCallback(ConnectionPtr connection, const std::error_code& ec);

        IOLoop* _ioLoop;
        bool _connecting{false};

        Task<void(ConnectionPtr, const std::error_code&)> _callback;
        Time _timeout;
        size_t _maxBufferSize;
        std::string _sourceIP;
        unsigned short _sourcePort;

        TimerHandle _timer;
        ResolveHandle _resolve;
    };

}

#endif //EASYEVENT_EVENT_TCPCLIENT_H
