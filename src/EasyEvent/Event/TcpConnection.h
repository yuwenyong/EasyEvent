//
// Created by yuwenyong.vincent on 2021/6/26.
//

#ifndef EASYEVENT_EVENT_TCPCONNECTION_H
#define EASYEVENT_EVENT_TCPCONNECTION_H

#include "EasyEvent/Event/EventBase.h"
#include "EasyEvent/Event/Connection.h"


namespace EasyEvent {

    class EASY_EVENT_API TcpConnection: public Connection {
    public:
        TcpConnection(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize, size_t maxWriteBufferSize,
                      MakeSharedTag tag);

        ~TcpConnection() noexcept override;

        SocketType getFD() const override;

        void closeFD() override;

        void connect(const Address& address, Task<void(std::error_code)>&& callback) override;

        void setNoDelay(bool value) override;

        std::string getLocalIP() const override;

        unsigned short getLocalPort() const override;

        Address getLocalAddress() const override;

        std::string getRemoteIP() const override;

        unsigned short getRemotePort() const override;

        Address getRemoteAddress() const override;

        static std::shared_ptr<TcpConnection> create(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize=0,
                                                     size_t maxWriteBufferSize=0) {
            return std::make_shared<TcpConnection>(ioLoop, socket, maxReadBufferSize, maxWriteBufferSize, MakeSharedTag{});
        }
    protected:
        ssize_t writeToFd(const void* data, size_t size, std::error_code& ec) override;

        ssize_t readFromFd(void* buf, size_t size, std::error_code& ec) override;

        int getFdError(std::error_code& ec) override;

        void runConnectCallback(std::error_code ec);

        void handleConnect() override;

        SocketType _socket;
    };

}


#endif //EASYEVENT_EVENT_TCPCONNECTION_H
