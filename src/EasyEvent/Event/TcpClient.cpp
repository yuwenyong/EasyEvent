//
// Created by yuwenyong on 2020/12/22.
//

#include "EasyEvent/Event/TcpClient.h"


void EasyEvent::TcpConnector::start(Task<void(ConnectionPtr, const std::error_code &)> &&callback,
                                    const std::vector<Address> &addrs, Time connectTimeout, Time timeout) {
    Assert(!addrs.empty());
    splitAddresses(addrs);
    _callback = std::move(callback);
    tryConnect(true);
    if (connectTimeout) {
        setConnectTimeout(connectTimeout);
    }
    if (timeout && !_secondaryAddrs.empty()) {
        setTimeout(timeout);
    }
}

void EasyEvent::TcpConnector::splitAddresses(const std::vector<Address> &addrs) {
    _remaining = addrs.size();
    int af = addrs.front().getFamily();
    for (auto& addr: addrs) {
        if (addr.getFamily() == af) {
            _primaryAddrs.emplace_back(addr);
        } else {
            _secondaryAddrs.emplace_back(addr);
        }
    }
    _primaryIter = _primaryAddrs.begin();
    _secondaryIter = _secondaryAddrs.begin();
}

void EasyEvent::TcpConnector::tryConnect(bool primary) {
    if (!_callback) {
        return;
    }
    if (_remaining == 0) {
        runConnectCallback(nullptr, _lastError);
        return;
    }
    Address* addr = nullptr;
    if (primary) {
        if (_primaryIter == _primaryAddrs.end()) {
            return;
        }
        addr = &(*_primaryIter);
        ++_primaryIter;
    } else {
        if (_secondaryIter == _secondaryAddrs.end()) {
            return;
        }
        addr = &(*_secondaryIter);
        ++_secondaryIter;
    }
    std::error_code ec = {};
    try {
        auto connection = _connectionCallback(*addr);
        SocketType socket = connection->getFD();
        connection->connect(*addr, [this, self=shared_from_this(), primary, socket](std::error_code ec) {
            onConnectDone(primary, socket, ec);
        });
        _connections.insert(std::pair<SocketType, ConnectionHolder>(socket, connection));
    } catch (std::system_error& e) {
        ec = e.code();
    } catch (...) {
        ec = EventErrors::UnexpectedBehaviour;
    }
    if (ec) {
        _ioLoop->addCallback([this, self=shared_from_this(), primary, ec]() {
           onConnectDone(primary, InvalidSocket, ec);
        });
    }
}

void EasyEvent::TcpConnector::onConnectDone(bool primary, SocketType socket, const std::error_code &ec) {
    --_remaining;
    if (ec) {
        _lastError = ec;
        clearTimeout();
        if (socket != InvalidSocket) {
            auto iter = _connections.find(socket);
            if (iter != _connections.end()) {
                _connections.erase(iter);
            }
        }
        tryConnect(primary);
    } else {
        auto iter = _connections.find(socket);
        if (iter == _connections.end()) {
            return;
        }
        auto connection = iter->second.lock();
        _connections.erase(iter);
        clearTimeouts();
        closeConnections();
        runConnectCallback(std::move(connection), {});
    }
}

void EasyEvent::TcpConnector::runConnectCallback(ConnectionPtr connection, const std::error_code &ec) {
    auto callback = std::move(_callback);
    _callback = nullptr;
    callback(std::move(connection), ec);
}


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
                        onResolved(addresses, ec);
                    });
    if (_timeout) {
        _timer = _ioLoop->callLater(_timeout, [this, self=shared_from_this()]() {
           onTimeout();
        });
    }
}

void EasyEvent::TcpClient::onResolved(const std::vector<Address>& addresses, const std::error_code& ec) {
    if (!_timer.expired()) {
        _ioLoop->removeTimeout(_timer);
    }
    if (ec) {
        runConnectCallback(nullptr, ec);
    } else {
        Assert(!addresses.empty());
        auto connector = std::make_shared<TcpConnector>(_ioLoop, [this, self=shared_from_this()](const Address &address) {
            return createConnection(address);
        });
        connector->start([this, self=shared_from_this()](ConnectionPtr connection, const std::error_code &ec) {
           runConnectCallback(std::move(connection), ec);
        }, addresses, _timeout);
    }
}

void EasyEvent::TcpClient::runConnectCallback(ConnectionPtr connection, const std::error_code &ec) {
    auto callback = std::move(_callback);
    _callback = nullptr;
    _connecting = false;
    callback(std::move(connection), ec);
}

EasyEvent::ConnectionPtr EasyEvent::TcpClient::createConnection(const Address &address) {
    SocketHolder socket(SocketOps::Socket(address.getFamily(), SOCK_STREAM, IPPROTO_TCP));
    if (_sourcePort) {
        Address sourceAddr;
        if (_sourceIP.empty()) {
            sourceAddr = address.isIPv6() ? Address::loopbackAddressIPv6(_sourcePort) : Address::loopbackAddressIPv4(_sourcePort);
        } else {
            sourceAddr = Address::makeAddress(_sourceIP.c_str(), _sourcePort);
        }
        SocketOps::Bind(socket.get(), sourceAddr);
    }
    return std::make_shared<Connection>(_ioLoop, socket.release(), _maxBufferSize);
}