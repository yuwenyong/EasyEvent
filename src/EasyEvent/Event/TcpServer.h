//
// Created by yuwenyong on 2020/12/19.
//

#ifndef EASYEVENT_EVENT_TCPSERVER_H
#define EASYEVENT_EVENT_TCPSERVER_H

#include "EasyEvent/Event/EventBase.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Event/Connection.h"
#include "EasyEvent/Common/Task.h"
#include "EasyEvent/Ssl/SslContext.h"


namespace EasyEvent {

    constexpr int DefaultBacklog = 128;

    class EASY_EVENT_API TcpListener: public Selectable, public std::enable_shared_from_this<TcpListener> {
    public:
        TcpListener(const TcpListener&) = delete;
        TcpListener& operator=(const TcpListener&) = delete;

        TcpListener(IOLoop* ioLoop, SocketType socket)
            : _ioLoop(ioLoop ? ioLoop : IOLoop::current())
            , _socket(socket) {

        }

        ~TcpListener() noexcept override;

        void handleEvents(IOEvents events) override;

        SocketType getFD() const override;

        void closeFD() override;

        void startListening(Task<void(SocketType, const Address&)>&& callback);

        void close();

        bool closed() const {
            return _socket == InvalidSocket;
        }

        bool listening() const {
            return (bool)_callback;
        }
    private:
        IOLoop* _ioLoop;
        SocketType _socket;
        Task<void(SocketType, const Address&)> _callback;
    };

    using TcpListenerPtr = std::shared_ptr<TcpListener>;
    using TcpListenerHolder = Holder<TcpListener>;

    class EASY_EVENT_API TcpServer: public std::enable_shared_from_this<TcpServer> {
    protected:
        struct MakeSharedTag {};
    public:
        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        TcpServer(IOLoop* ioLoop, SslContextPtr sslContext, size_t maxBufferSize, MakeSharedTag tag);

        virtual ~TcpServer() noexcept = default;

        Logger* getLogger()
        {
            return _ioLoop->getLogger();
        }

        void listen(unsigned short port, const std::string& address="") {
            auto sockets = bindSockets(port, address);
            addSockets(std::move(sockets));
        }

        void bind(unsigned short port, const std::string& address="", ProtocolSupport protocol=EnableBoth,
                  int backlog=DefaultBacklog);

        void start(Task<void(ConnectionPtr, const Address&)>&& callback= nullptr);

        void stop();

        virtual void handleConnection(ConnectionPtr connection, const Address& address);

        static std::shared_ptr<TcpServer> create(IOLoop* ioLoop, SslContextPtr sslContext= nullptr,
                                                 size_t maxBufferSize=0) {
            return std::make_shared<TcpServer>(ioLoop, std::move(sslContext), maxBufferSize, MakeSharedTag{});
        }
    protected:

        std::vector<SocketHolder> bindSockets(unsigned short port, const std::string& address="",
                                              ProtocolSupport protocol=EnableBoth, int backlog=DefaultBacklog);

        void addSockets(std::vector<SocketHolder>&& sockets);

        void addSocket(SocketHolder&& holder) {
            std::vector<SocketHolder> sockets;
            sockets.emplace_back(std::move(holder));
            addSockets(std::move(sockets));
        }

        void handleIncomingConnection(SocketType socket, const Address& address);

        IOLoop* _ioLoop;
        SslContextPtr _sslContext{nullptr};
        std::map<SocketType, TcpListenerHolder> _handlers;
        std::vector<SocketHolder> _pendingSockets;
        bool _started{false};
        bool _stopped{false};
        size_t _maxBufferSize;
        Task<void(ConnectionPtr, const Address&)> _callback;
    };

    using TcpServerPtr = std::shared_ptr<TcpServer>;

}

#endif //EASYEVENT_EVENT_TCPSERVER_H
