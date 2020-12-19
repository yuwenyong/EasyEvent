//
// Created by yuwenyong on 2020/12/19.
//

#include "EasyEvent/Event/TcpServer.h"


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