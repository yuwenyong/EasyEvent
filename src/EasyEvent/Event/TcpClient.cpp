//
// Created by yuwenyong on 2020/12/22.
//

#include "EasyEvent/Event/TcpClient.h"


void EasyEvent::TcpClient::connect(Task<void(ConnectionPtr, const std::error_code &)> &&callback,
                                   std::string host, unsigned short port, ProtocolSupport protocol,
                                   Time timeout, size_t maxBufferSize,
                                   std::string sourceIP, unsigned short sourcePort) {
    if (_connecting) {
        std::error_code ec = EventErrors::AlreadyConnecting;
        throwError(ec, "TcpClient");
    }
    _connecting = true;
    _callback = std::move(callback);
    _timeout = timeout;
    _maxBufferSize = maxBufferSize;
    _sourceIP = std::move(sourceIP);
    _sourcePort = sourcePort;

    _ioLoop->resolve(std::move(host), port, protocol, false,
                     [this, self=shared_from_this()](std::vector<Address> addresses, std::error_code ec) {
                        onResolved(std::move(addresses), ec);
                    });
    if (_timeout) {
        _timer = _ioLoop->callLater(_timeout, [this, self=shared_from_this()]() {
           onTimeout();
        });
    }
}

void EasyEvent::TcpClient::onResolved(std::vector<Address> addresses, std::error_code ec) {
    if (!_timer.expired()) {
        _ioLoop->removeTimeout(_timer);
    }
    if (ec) {
        runConnectCallback(nullptr, ec);
    } else {

    }
}

void EasyEvent::TcpClient::runConnectCallback(ConnectionPtr connection, const std::error_code &ec) {
    auto callback = std::move(_callback);
    _callback = nullptr;
    _connecting = false;
    callback(std::move(connection), ec);
}