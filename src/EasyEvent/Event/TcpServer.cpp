//
// Created by yuwenyong on 2020/12/19.
//

#include "EasyEvent/Event/TcpServer.h"
#include "EasyEvent/Event/Resolver.h"


void EasyEvent::TcpListener::handleEvents(IOEvents events) {

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


std::vector<EasyEvent::SocketHolder> EasyEvent::TcpServer::bindSockets(unsigned short port, const std::string &address,
                                                                       int family, int backlog) {
    ProtocolSupport protocol;
    switch (family) {
        case AF_UNSPEC:
            protocol = EnableBoth;
            break;
        case AF_INET:
            protocol = EnableIPv4;
            break;
        case AF_INET6:
            protocol = EnableIPv6;
            break;
        default:
            Assert(false);
            break;
    }
    if (backlog == 0) {
        backlog = DefaultBacklog;
    }
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