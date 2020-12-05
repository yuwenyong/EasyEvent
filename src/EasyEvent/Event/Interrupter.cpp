//
// Created by yuwenyong on 2020/12/4.
//

#include "EasyEvent/Event/Interrupter.h"
#include "EasyEvent/Event/SocketOps.h"


EasyEvent::Interrupter::~Interrupter() noexcept {
    closeSocket();
}

void EasyEvent::Interrupter::openSockets() {
    std::error_code ec;
    SocketType acceptor = SocketOps::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, ec);
    if (acceptor == InvalidSocket) {
        throwError(ec, "Interrupter");
    }
    Address addr = Address::loopbackAddressIPv4(0);
    if (SocketOps::Bind(acceptor, addr, ec) == SocketErrorRetVal) {
        throwError(ec, "Interrupter");
    }

    int fds[2];
    if (pipe(fds) == 0) {
        _reader = fds[0];
        ::fcntl(_reader, F_SETFL, O_NONBLOCK);
        _writer = fds[1];
        ::fcntl(_writer, F_SETFL, O_NONBLOCK);

#if defined(FD_CLOEXEC)
        ::fcntl(_reader, F_SETFD, FD_CLOEXEC);
        ::fcntl(_writer, F_SETFD, FD_CLOEXEC);
#endif
    } else {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "Interrupter");
    }
}

void EasyEvent::Interrupter::handleEvents(IOEvents events) {
    if (((uint16_t)events & (uint16_t)IOEvents::IO_EVENT_READ) != 0) {
        reset();
    }
}

SocketType EasyEvent::Interrupter::getSocket() const {
    return _reader;
}

void EasyEvent::Interrupter::closeSocket() {
    if (_reader != InvalidSocket) {
        ::close(_reader);
        _reader = InvalidSocket;
    }
    if (_writer != InvalidSocket) {
        ::close(_writer);
        _writer = InvalidSocket;
    }
}

void EasyEvent::Interrupter::interrupt() {
    char byte = 0;
    ::write(_writer, &byte, 1);
}

bool EasyEvent::Interrupter::reset() {
    char data[512];
    while (true) {
        ssize_t bytesRead = ::read(_reader, data, sizeof(data));
        if (bytesRead == sizeof(data)) {
            continue;
        }
        if (bytesRead > 0) {
            return true;
        }
        if (bytesRead == 0) {
            return false;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return true;
        }
        return false;
    }
}