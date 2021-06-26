//
// Created by yuwenyong.vincent on 2021/6/19.
//

#ifndef EASYEVENT_SSL_SSLSOCKET_H
#define EASYEVENT_SSL_SSLSOCKET_H

#include "EasyEvent/Ssl/Types.h"
#include "EasyEvent/Ssl/SslContext.h"
#include "EasyEvent/Event/Event.h"


namespace EasyEvent {

    class EASY_EVENT_API SslSocket: NonCopyable {
    public:
        SslSocket(SslContextPtr context, SocketType socket, bool owned, SslServerOrClient socketType);

        SslSocket(SslSocket&& other) noexcept {
            _context = std::move(other._context);
            _socket = other._socket;
            other._socket = InvalidSocket;
            _ssl = other._ssl;
            other._ssl = nullptr;
            _owned = other._owned;
            other._owned = false;
        }

        SslSocket& operator=(SslSocket&& other) noexcept {
            SslSocket tmp(std::move(*this));
            _context = std::move(other._context);
            _socket = other._socket;
            other._socket = InvalidSocket;
            _ssl = other._ssl;
            other._ssl = nullptr;
            _owned = other._owned;
            other._owned = false;
            return *this;
        }

        ~SslSocket() noexcept;

        SslContextPtr getContext() const {
            return _context;
        }

        SocketType getSocket() const {
            return _socket;
        }

        SSL* getNativeHandle() const {
            return _ssl;
        }

        void setVerifyMode(SslVerifyMode verifyMode);

        void setVerifyMode(SslVerifyMode verifyMode, std::error_code& ec);

        template<class VerifyCallback>
        void setVerifyCallback(VerifyCallback&& callback) {
            std::error_code ec;
            this->template setVerifyCallback(std::forward<VerifyCallback>(callback), ec);
            throwError(ec, "setVerifyCallback");
        }

        template<class VerifyCallback>
        void setVerifyCallback(VerifyCallback&& callback, std::error_code& ec) {
            doSetVerifyCallback(new SslVerifyCallback<VerifyCallback>(std::forward<VerifyCallback>(callback)), ec);
        }

        void setVerifyHostName(const std::string& hostName) {
            std::error_code ec;
            setVerifyHostName(hostName, ec);
            throwError(ec, "setVerifyHostName");
        }

        void setVerifyHostName(const std::string& hostName, std::error_code& ec) {
            setVerifyCallback(HostNameVerification(hostName), ec);
        }

        void doHandshake();

        void doHandshake(std::error_code& ec);

        ssize_t write(const void* data, size_t size);

        ssize_t write(const void* data, size_t size, std::error_code& ec);

        ssize_t read(void* data, size_t size);

        ssize_t read(void* data, size_t size, std::error_code& ec);

        void close();

        void close(std::error_code& ec);
    protected:
        void doSetVerifyCallback(SslVerifyCallbackBase* callback, std::error_code& ec);

        static int verifyCallbackFunction(int preverified, X509_STORE_CTX* ctx);

        void setErrorCode(int result, std::error_code& ec);

        SslContextPtr _context;
        SocketType _socket;
        SSL* _ssl{nullptr};
        bool _owned;
    };

}

#endif //EASYEVENT_SSL_SSLSOCKET_H
