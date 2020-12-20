//
// Created by yuwenyong on 2020/12/19.
//

#ifndef EASYEVENT_EVENT_TCPSERVER_H
#define EASYEVENT_EVENT_TCPSERVER_H

#include "EasyEvent/Event/Event.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Event/Connection.h"
#include "EasyEvent/Common/Task.h"


namespace EasyEvent {

    class EASY_EVENT_API TcpListener: public Selectable, public std::enable_shared_from_this<TcpListener> {
    public:
        TcpListener(const TcpListener&) = delete;
        TcpListener& operator=(const TcpListener&) = delete;

        void handleEvents(IOEvents events) override;

        SocketType getFD() const override;

        void closeFD() override;
    private:
        IOLoop* ioLoop;
        SocketType _socket;
        Task<void(SocketType, const Address&)> _callback;
    };

    class EASY_EVENT_API TcpServer: public std::enable_shared_from_this<TcpServer> {
    public:
        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        explicit TcpServer(IOLoop* ioLoop, size_t maxBufferSize=0)
            : _ioLoop(ioLoop ? ioLoop : IOLoop::current())
            , _logger(_ioLoop->getLogger())
            , _maxBufferSize(maxBufferSize) {

        }

        void listen(unsigned short port, const std::string& address="") {
        }

        static constexpr int DefaultBacklog = 128;
    protected:
        std::vector<SocketHolder> bindSockets(unsigned short port, const std::string& address, int family=AF_UNSPEC,
                                              int backlog=0);

        IOLoop* _ioLoop;
        Logger* _logger;
        std::map<SocketType, int> _handlers;
        std::vector<SocketHolder> _pendingSockets;
        bool _started{false};
        bool _stopped{false};
        size_t _maxBufferSize;
    };

}

#endif //EASYEVENT_EVENT_TCPSERVER_H
