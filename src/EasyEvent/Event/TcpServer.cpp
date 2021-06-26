//
// Created by yuwenyong on 2020/12/19.
//

#include "EasyEvent/Event/TcpServer.h"
#include "EasyEvent/Event/Resolver.h"
#include "EasyEvent/Event/TcpConnection.h"
#include "EasyEvent/Logging/LogStream.h"


EasyEvent::TcpListener::~TcpListener() noexcept {
    if (_socket != InvalidSocket) {
        std::error_code ignoredError;
        SocketOps::Close(_socket, true, ignoredError);
    }
}

void EasyEvent::TcpListener::handleEvents(IOEvents events) {
    UnusedParameter(events);
    Address address;
    std::error_code ec;
    for (int i = 0; i < DefaultBacklog; ++i) {
        SocketHolder socket(SocketOps::Accept(_socket, address, ec));
        if (ec) {
            if (ec == SocketErrors::WouldBlock || ec == SocketErrors::TryAgain) {
                return;
            } else if (ec == SocketErrors::ConnectionAborted) {
                continue;
            } else {
                throwError(ec, "TcpListener");
            }
        } else {
            _callback(socket.release(), address);
        }
    }
}

SocketType EasyEvent::TcpListener::getFD() const {
    return _socket;
}

void EasyEvent::TcpListener::closeFD() {
    std::error_code ec;
    SocketOps::Close(_socket, false, ec);
    if (!ec) {
        _socket = InvalidSocket;
    } else {
        throwError(ec, "TcpListener");
    }
}

void EasyEvent::TcpListener::startListening(Task<void(SocketType, const Address &)> &&callback) {
    if (listening()) {
        std::error_code ec = EventErrors::AlreadyListening;
        throwError(ec, "TcpListener");
    }
    _callback = std::move(callback);
    _ioLoop->addHandler(shared_from_this(), IO_EVENT_READ);
}

void EasyEvent::TcpListener::close() {
    if (listening()) {
        _ioLoop->removeHandler(shared_from_this());
        _callback = nullptr;
    }
    if (!closed()) {
        closeFD();
    }
}


void EasyEvent::TcpServer::bind(unsigned short port, const std::string &address, ProtocolSupport protocol, int backlog) {
    auto sockets = bindSockets(port, address, protocol, backlog);
    if (_started) {
        addSockets(std::move(sockets));
    } else {
        if (_pendingSockets.empty()) {
            _pendingSockets = std::move(sockets);
        } else {
            for (auto& socket: sockets) {
                _pendingSockets.emplace_back(std::move(socket));
            }
        }
    }
}

void EasyEvent::TcpServer::start(Task<void(ConnectionPtr, const Address&)>&& callback) {
    Assert(!_started);
    _started = true;
    auto sockets = std::move(_pendingSockets);
    _pendingSockets.clear();
    addSockets(std::move(sockets));
    _callback = std::move(callback);
}

void EasyEvent::TcpServer::stop() {
    if (_stopped) {
        return;
    }
    _stopped = true;
    for (auto& handler: _handlers) {
        handler.second->close();
    }
    _handlers.clear();
}

void EasyEvent::TcpServer::handleConnection(ConnectionPtr connection, const Address &address) {
    if (_callback) {
        _callback(std::move(connection), address);
    } else {
        std::error_code ec = EventErrors::CallbackNotFound;
        throwError(ec, "TcpServer");
    }
}

std::vector<EasyEvent::SocketHolder> EasyEvent::TcpServer::bindSockets(unsigned short port, const std::string &address,
                                                                       ProtocolSupport protocol, int backlog) {
    auto addresses = Resolver::getAddresses(address, port, protocol, false, true);
    std::vector<SocketHolder> sockets;
    for (auto& addr: addresses) {
        SocketHolder holder(SocketOps::Socket(addr.getFamily(), SOCK_STREAM, IPPROTO_TCP));
        SocketOps::SetReuseAddress(holder.get(), true);
        if (addr.isIPv6()) {
            SocketOps::SetIPv6Only(holder.get(), true);
        }
        SocketOps::SetNonblock(holder.get(), true);
        SocketOps::Bind(holder.get(), addr);
        SocketOps::Listen(holder.get(), backlog);
        sockets.emplace_back(std::move(holder));
    }
    return sockets;
}

void EasyEvent::TcpServer::addSockets(std::vector<SocketHolder> &&sockets) {
    for (auto& socket: sockets) {
        auto listener = std::make_shared<TcpListener>(_ioLoop, socket.release());
        listener->startListening([self=shared_from_this(), this](SocketType socket, const Address& address) {
            handleIncomingConnection(socket, address);
        });
        _handlers.insert(std::pair<SocketType, TcpListenerHolder>(listener->getFD(), listener));
    }
}

void EasyEvent::TcpServer::handleIncomingConnection(SocketType socket, const Address &address) {
    try {
        auto connection = TcpConnection::create(_ioLoop, socket, _maxBufferSize);
        handleConnection(std::move(connection), address);
    } catch (std::exception& e) {
        LOG_ERROR(getLogger()) << "Error in connection callback: " << e.what();
    } catch (...) {
        LOG_ERROR(getLogger()) << "Unknown error in connection callback";
    }
}