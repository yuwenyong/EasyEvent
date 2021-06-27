//
// Created by yuwenyong.vincent on 2021/6/27.
//

#include "EasyEvent/Event/SslConnection.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Logging/LogStream.h"

EasyEvent::SslConnection::SslConnection(IOLoop *ioLoop, SocketType socket, size_t maxReadBufferSize,
                                        size_t maxWriteBufferSize, MakeSharedTag tag)
    : TcpConnection(ioLoop, socket, maxReadBufferSize, maxWriteBufferSize, tag) {

}

void EasyEvent::SslConnection::closeFD() {
    if (_sslSock) {
        _sslSock.reset();
    }
    TcpConnection::closeFD();
}

bool EasyEvent::SslConnection::reading() const {
    return _handshakeReading || Connection::reading();
}

bool EasyEvent::SslConnection::writing() const {
    return _handshakeWriting || Connection::writing();
}

void EasyEvent::SslConnection::startTls(bool serverSide, SslContextPtr context, const std::string &serverHostname) {
    if (!_sslSock) {
        _sslSock = std::make_unique<SslSocket>(std::move(context), _socket, false,
                                               serverSide ? SslServerOrClient::Server : SslServerOrClient::Client);
        if (!serverHostname.empty()) {
            _sslSock->setVerifyHostName(serverHostname);
        }
        addIOState(IO_EVENT_WRITE);
    }
}

void EasyEvent::SslConnection::doSslHandshake() {
    std::error_code ec;
    _handshakeReading = false;
    _handshakeWriting = false;
    _sslSock->doHandshake(ec);
    if (!ec) {
        _sslAccepting = false;
        runSslConnectCallback(ec);
    } else if (ec == SslErrors::WantRead) {
        _handshakeReading = true;
    } else if (ec == SslErrors::WantWrite) {
        _handshakeWriting = true;
    } else if (ec == SslErrors::ErrorZeroReturn || ec == SslStreamErrors::ConnectionReset) {
        runSslConnectCallback(ec);
        close(ec);
    } else if (ec == SslStreamErrors::StreamTruncated) {
        LOG_WARN(getLogger()) << "SSL short read: " << ec << "(" << ec.message() << ")";
        runSslConnectCallback(ec);
        close(ec);
    } else if (ec.category() == getSslErrorCategory()) {
        LOG_WARN(getLogger()) << "SSL error: " << ec << "(" << ec.message() << ")";
        runSslConnectCallback(ec);
        close(ec);
    } else if (ec.category() == getSocketErrorCategory()) {
        runSslConnectCallback(ec);
        if (isConnReset(ec) || ec == SocketErrors::BadDescriptor || ec == SocketErrors::NotConnected) {
            close(ec);
            return;
        }
        throwError(ec, "SslConnection");
    } else {
        LOG_ERROR(getLogger()) << "Unexpected behavior: " << ec << "(" << ec.message() << ")";
        runSslConnectCallback(ec);
        close(ec);
    }
}

void EasyEvent::SslConnection::handleRead() {
    if (!_sslSock) {
        return;
    }
    if (_sslAccepting) {
        doSslHandshake();
        return;
    }
    TcpConnection::handleRead();
}

void EasyEvent::SslConnection::handleWrite() {
    if (!_sslSock) {
        return;
    }
    if (_sslAccepting) {
        doSslHandshake();
        return;
    }
    TcpConnection::handleWrite();
}

void EasyEvent::SslConnection::waitForHandshake(Task<void(std::error_code)> &&callback) {
    if (_sslConnectCallback) {
        std::error_code ec;
        ec = EventErrors::AlreadyWaiting;
        throwError(ec, "SslConnection");
    }
    _sslConnectCallback = std::move(callback);
    if (!_sslAccepting) {
        runSslConnectCallback(std::error_code());
    }
}

void EasyEvent::SslConnection::runSslConnectCallback(std::error_code ec) {
    if (_sslConnectCallback) {
        auto callback = std::move(_sslConnectCallback);
        _sslConnectCallback = nullptr;
        ++_pendingCallbacks;
        _ioLoop->addCallback([this, ec, self=shared_from_this(), callback=std::move(callback)]() {
            --_pendingCallbacks;
            LocalAddErrorListener addErrorListener(this);
            try {
                callback(ec);
            } catch (std::system_error& e) {
                LOG_ERROR(getLogger()) << "Uncaught system error in ssl connect callback: " << e.what();
                close(e.code());
                throw;
            } catch (std::exception& e) {
                LOG_ERROR(getLogger()) << "Uncaught exception in ssl connect callback: " << e.what();
                close(EventErrors::SslConnectCallbackFailed);
                throw;
            } catch (...) {
                LOG_ERROR(getLogger()) << "Uncaught exception in ssl connect callback: Unknown error.";
                close(EventErrors::SslConnectCallbackFailed);
                throw;
            }
        });
    }
}

ssize_t EasyEvent::SslConnection::writeToFd(const void *data, size_t size, std::error_code &ec) {
    auto result = _sslSock->write(data, size, ec);
    if (ec == SslErrors::WantWrite) {
        return 0;
    }
    return result;
}

ssize_t EasyEvent::SslConnection::readFromFd(void *buf, size_t size, std::error_code &ec) {
    if (_sslAccepting) {
        ec = SocketErrors::WouldBlock;
        return 0;
    }
    auto result = _sslSock->read(buf, size, ec);
    if (ec == SslErrors::WantRead) {
        ec = SocketErrors::WouldBlock;
        return 0;
    }
    return result;
}