//
// Created by yuwenyong.vincent on 2021/6/27.
//

#ifndef EASYEVENT_EVENT_SSLCONNECTION_H
#define EASYEVENT_EVENT_SSLCONNECTION_H

#include "EasyEvent/Event/Event.h"
#include "EasyEvent/Event/TcpConnection.h"
#include "EasyEvent/Ssl/SslContext.h"
#include "EasyEvent/Ssl/SslSocket.h"


namespace EasyEvent {

    class EASY_EVENT_API SslConnection: public TcpConnection {
    public:
        SslConnection(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize, size_t maxWriteBufferSize,
                      MakeSharedTag tag);

        void closeFD() override;

        bool reading() const override;

        bool writing() const override;

        void startTls(bool serverSide, SslContextPtr context, const std::string& serverHostname={});

        void waitForHandshake(Task<void(std::error_code)>&& callback);

        static std::shared_ptr<SslConnection> create(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize=0,
                                                     size_t maxWriteBufferSize=0) {
            return std::make_shared<SslConnection>(ioLoop, socket, maxReadBufferSize, maxWriteBufferSize, MakeSharedTag{});
        }
    protected:
        void doSslHandshake();

        void handleRead() override;

        void handleWrite() override;

        void runSslConnectCallback(std::error_code ec);

        ssize_t writeToFd(const void* data, size_t size, std::error_code& ec) override;

        ssize_t readFromFd(void* buf, size_t size, std::error_code& ec) override;

        bool isConnReset(const std::error_code& ec) const override {
            if (ec == SslStreamErrors::ConnectionReset) {
                return true;
            }
            return TcpConnection::isConnReset(ec);
        }

        bool _sslAccepting{true};
        bool _handshakeReading{false};
        bool _handshakeWriting{false};
        std::unique_ptr<SslSocket> _sslSock;
        Task<void(std::error_code)> _sslConnectCallback;
    };

}

#endif //EASYEVENT_EVENT_SSLCONNECTION_H
