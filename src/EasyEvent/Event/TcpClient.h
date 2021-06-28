//
// Created by yuwenyong on 2020/12/22.
//

#ifndef EASYEVENT_EVENT_TCPCLIENT_H
#define EASYEVENT_EVENT_TCPCLIENT_H

#include "EasyEvent/Event/Event.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Event/Connection.h"
#include "EasyEvent/Common/Task.h"
#include "EasyEvent/Ssl/SslContext.h"


namespace EasyEvent {

    class EASY_EVENT_API TcpConnector: public std::enable_shared_from_this<TcpConnector> {
    public:
        TcpConnector(const TcpConnector&) = delete;
        TcpConnector& operator=(const TcpConnector&) = delete;

        explicit TcpConnector(IOLoop* ioLoop, Task<ConnectionPtr(const Address& address)>&& connectionCallback)
            : _ioLoop(ioLoop ? ioLoop : IOLoop::current())
            , _connectionCallback(std::move(connectionCallback)) {

        }

        void start(Task<void(ConnectionPtr, const std::error_code&)>&& callback, const std::vector<Address>& addrs,
                   Time connectTimeout={}, Time timeout=Time::milliSeconds(300));
    private:
        void splitAddresses(const std::vector<Address>& addrs);

        void tryConnect(bool primary);

        void onConnectDone(bool primary, SocketType socket, const std::error_code& ec);

        void setTimeout(Time timeout) {
            _timer = _ioLoop->callLater(timeout, [this, self=shared_from_this()]() {
                onTimeout();
            });
        }

        void onTimeout() {
            tryConnect(false);
        }

        void clearTimeout() {
            if (!_timer.expired()) {
                _ioLoop->removeTimeout(_timer);
            }
        }

        void setConnectTimeout(Time connectTimeout) {
            _connectTimer = _ioLoop->callLater(connectTimeout, [this, self=shared_from_this()]() {
                onConnectTimeout();
            });
        }

        void onConnectTimeout() {
            closeConnections();
            runConnectCallback(nullptr, EventErrors::ConnectTimeout);
        }

        void clearConnectTimeout() {
            if (!_connectTimer.expired()) {
                _ioLoop->removeTimeout(_connectTimer);
            }
        }

        void clearTimeouts() {
            clearTimeout();
            clearConnectTimeout();
        }

        void closeConnections() {
            for (auto& conn: _connections) {
                conn.second->close();
            }
        }

        void runConnectCallback(ConnectionPtr connection, const std::error_code& ec);

        IOLoop* _ioLoop;
        Task<ConnectionPtr(const Address& address)> _connectionCallback;
        Task<void(ConnectionPtr, const std::error_code&)> _callback;
        TimerHandle _timer;
        TimerHandle _connectTimer;
        std::error_code _lastError;
        size_t _remaining;
        std::vector<Address> _primaryAddrs;
        std::vector<Address> _secondaryAddrs;
        std::vector<Address>::iterator _primaryIter;
        std::vector<Address>::iterator _secondaryIter;
        std::map<SocketType, ConnectionHolder> _connections;
    };


    class EASY_EVENT_API TcpClient: public std::enable_shared_from_this<TcpClient> {
    private:
        struct MakeSharedTag {};
    public:
        TcpClient(const TcpClient&) = delete;
        TcpClient& operator=(const TcpClient&) = delete;

        explicit TcpClient(IOLoop* ioLoop, MakeSharedTag tag)
            : TcpClient(ioLoop) {
            UnusedParameter(tag);
        }

        void connect(Task<void(ConnectionPtr, const std::error_code&)>&& callback,
                     std::string host, unsigned short port, SslContextPtr sslContext= nullptr,
                     ProtocolSupport protocol=EnableBoth, Time timeout={}, size_t maxBufferSize=0,
                     std::string sourceIP="", unsigned short sourcePort=0);

        static std::shared_ptr<TcpClient> create(IOLoop* ioLoop) {
            return std::make_shared<TcpClient>(ioLoop, MakeSharedTag{});
        }
    protected:
        explicit TcpClient(IOLoop* ioLoop)
                : _ioLoop(ioLoop ? ioLoop : IOLoop::current()) {

        }

        void onResolved(const std::vector<Address>& addresses, const std::error_code& ec);

        void onTimeout() {
            _resolve.cancel();
        }

        void runConnectCallback(ConnectionPtr connection, const std::error_code& ec);

        ConnectionPtr createConnection(const Address& address);

        IOLoop* _ioLoop;
        bool _connecting{false};

        Task<void(ConnectionPtr, const std::error_code&)> _callback;
        std::string _host;
        SslContextPtr _sslContext;
        Time _timeout;
        size_t _maxBufferSize{0};
        std::string _sourceIP;
        unsigned short _sourcePort{0};

        TimerHandle _timer;
        ResolveHandle _resolve;
    };

    using TcpClientPtr = std::shared_ptr<TcpClient>;

}

#endif //EASYEVENT_EVENT_TCPCLIENT_H
