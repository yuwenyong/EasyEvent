//
// Created by yuwenyong.vincent on 2021/6/26.
//

#include "EasyEvent/Event/TcpConnection.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Logging/LogStream.h"

EasyEvent::TcpConnection::TcpConnection(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize,
                                        size_t maxWriteBufferSize, MakeSharedTag tag)
    : Connection(ioLoop, maxReadBufferSize, maxWriteBufferSize, tag)
    , _socket(socket) {

    SocketOps::SetNonblock(_socket, true);

}

EasyEvent::TcpConnection::~TcpConnection() noexcept {
    if (_socket != InvalidSocket) {
        std::error_code ignoredError;
        SocketOps::Close(_socket, true, ignoredError);
    }
}

SocketType EasyEvent::TcpConnection::getFD() const {
    return _socket;
}

void EasyEvent::TcpConnection::closeFD() {
    std::error_code ec;
    SocketOps::Close(_socket, false, ec);
    if (!ec) {
        _socket = InvalidSocket;
    } else {
        throwError(ec, "TcpConnection");
    }
}

void EasyEvent::TcpConnection::connect(const Address &address, Task<void(std::error_code)> &&callback) {
    _connecting = true;
    _connectCallback = std::move(callback);

    std::error_code ec;
    SocketOps::Connect(_socket, address, ec);
    if (ec && !isInProgress(ec) && !isWouldBlock(ec)) {
        runConnectCallback(ec);
    } else {
        addIOState(IO_EVENT_WRITE);
    }
}

void EasyEvent::TcpConnection::setNoDelay(bool value) {
    SocketOps::SetTcpNoDelay(_socket, value);
}

std::string EasyEvent::TcpConnection::getLocalIP() const {
    Address address;
    SocketOps::GetSockName(_socket, address);
    return address.getAddrString();
}

unsigned short EasyEvent::TcpConnection::getLocalPort() const {
    Address address;
    SocketOps::GetSockName(_socket, address);
    return address.getPort();
}

EasyEvent::Address EasyEvent::TcpConnection::getLocalAddress() const {
    Address address;
    SocketOps::GetSockName(_socket, address);
    return address;
}

std::string EasyEvent::TcpConnection::getRemoteIP() const {
    Address address;
    SocketOps::GetPeerName(_socket, address);
    return address.getAddrString();
}

unsigned short EasyEvent::TcpConnection::getRemotePort() const {
    Address address;
    SocketOps::GetPeerName(_socket, address);
    return address.getPort();
}

EasyEvent::Address EasyEvent::TcpConnection::getRemoteAddress() const {
    Address address;
    SocketOps::GetPeerName(_socket, address);
    return address;
}

ssize_t EasyEvent::TcpConnection::writeToFd(const void *data, size_t size, std::error_code &ec) {
    return SocketOps::Send(_socket, data, size, 0, ec);
}

ssize_t EasyEvent::TcpConnection::readFromFd(void *buf, size_t size, std::error_code &ec) {
    return SocketOps::Recv(_socket, buf, size, 0, ec);
}

int EasyEvent::TcpConnection::getFdError(std::error_code &ec) {
    int error = 0;
    SocketOps::GetSockError(_socket, error, ec);
    return error;
}

void EasyEvent::TcpConnection::runConnectCallback(std::error_code ec) {
    if (_connectCallback) {
        auto callback = std::move(_connectCallback);
        _connectCallback = nullptr;
        ++_pendingCallbacks;
        _ioLoop->addCallback([this, ec, self=shared_from_this(), callback=std::move(callback)]() {
            --_pendingCallbacks;
            LocalAddErrorListener addErrorListener(this);
            try {
                callback(ec);
            } catch (std::system_error& e) {
                LOG_ERROR(getLogger()) << "Uncaught system error in connect callback: " << e.what();
                close(e.code());
                throw;
            } catch (std::exception& e) {
                LOG_ERROR(getLogger()) << "Uncaught exception in connect callback: " << e.what();
                close(EventErrors::ConnectCallbackFailed);
                throw;
            } catch (...) {
                LOG_ERROR(getLogger()) << "Uncaught exception in connect callback: Unknown error.";
                close(EventErrors::ConnectCallbackFailed);
                throw;
            }
        });
    }
    if (ec) {
        LOG_WARN(getLogger()) << "Connect error: " << ec << "(" << ec.message() << ")";
        close(ec);
    } else {
        _connecting = false;
    }
}

void EasyEvent::TcpConnection::handleConnect() {
    std::error_code ec;
    int err = getFdError(ec);
    if (ec == SocketErrors::NoProtocolOption) {
        err = 0;
    }
    if (!ec && err != 0) {
        ec = {err, getSocketErrorCategory()};
    }
    runConnectCallback(ec);
}

