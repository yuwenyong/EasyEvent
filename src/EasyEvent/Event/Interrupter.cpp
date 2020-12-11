//
// Created by yuwenyong on 2020/12/4.
//

#include "EasyEvent/Event/Interrupter.h"
#include "EasyEvent/Event/SocketOps.h"


EasyEvent::Interrupter::~Interrupter() noexcept {
    closeSocket();
}

void EasyEvent::Interrupter::openSockets() {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    std::error_code ec;
    SocketHolder acceptor(SocketOps::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, ec));
    if (acceptor.get() == InvalidSocket) {
        throwError(ec, "Interrupter");
    }
    SocketOps::SetReuseAddress(acceptor.get(), true, ec);

    Address addr = Address::loopbackAddressIPv4(0);
    if (SocketOps::Bind(acceptor.get(), addr, ec) == SocketErrorRetVal) {
        throwError(ec, "Interrupter");
    }
    if (SocketOps::GetSockName(acceptor.get(), addr, ec) == SocketErrorRetVal) {
        throwError(ec, "Interrupter");
    }
    if (addr.isAnyAddressIPv4()) {
        addr.setUIntIPv4(INADDR_LOOPBACK);
    }
    if (SocketOps::Listen(acceptor.get(), SOMAXCONN, ec) == SocketErrorRetVal) {
        throwError(ec, "Interrupter");
    }
    SocketHolder client(SocketOps::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, ec));
    if (client.get() == InvalidSocket) {
        throwError(ec, "Interrupter");
    }
    if (SocketOps::Connect(client.get(), addr, ec) == SocketErrorRetVal) {
        throwError(ec, "Interrupter");
    }

    SocketHolder server(SocketOps::Accept(acceptor.get(), 0, 0, ec));
    if (server.get() == InvalidSocket) {
        throwError(ec, "Interrupter");
    }

    if (SocketOps::SetUnblock(client.get(), true, ec)) {
        throwError(ec, "Interrupter");
    }
    SocketOps::SetTcpNoDelay(client.get(), true, ec);

    if (SocketOps::SetUnblock(server.get(), true, ec)) {
        throwError(ec, "Interrupter");
    }
    SocketOps::SetTcpNoDelay(server.get(), true, ec);

    _reader = server.release();
    _writer = server.release();
#else
    int fds[2];
    if (pipe(fds) == 0) {
        _reader = fds[0];
        ::fcntl(_reader, F_SETFL, O_NONBLOCK);
        _writer = fds[1];
        ::fcntl(_writer, F_SETFL, O_NONBLOCK);

#   if defined(FD_CLOEXEC)
        ::fcntl(_reader, F_SETFD, FD_CLOEXEC);
        ::fcntl(_writer, F_SETFD, FD_CLOEXEC);
#   endif
    } else {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "Interrupter");
    }
#endif
}

void EasyEvent::Interrupter::handleEvents(IOEvents events) {
    Assert((events & IO_EVENT_READ) != 0);
    reset();
}

SocketType EasyEvent::Interrupter::getSocket() const {
    return _reader;
}

void EasyEvent::Interrupter::closeSocket() {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    std::error_code ec;
    if (_reader != InvalidSocket) {
        SocketOps::Close(_reader, true, ec);
        _reader = InvalidSocket;
    }
    if (_writer != InvalidSocket) {
        SocketOps::Close(_writer, true, ec);
        _writer = InvalidSocket;
    }
#else
    if (_reader != InvalidSocket) {
        ::close(_reader);
        _reader = InvalidSocket;
    }
    if (_writer != InvalidSocket) {
        ::close(_writer);
        _writer = InvalidSocket;
    }
#endif
}

void EasyEvent::Interrupter::interrupt() {
    char byte = 0;
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    std::error_code ec;
    SocketOps::Send(_writer, &byte, 1, 0, ec);
#else
    ::write(_writer, &byte, 1);
#endif
}

bool EasyEvent::Interrupter::reset() {
    char data[512];
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    std::error_code ec;
    while (true) {
        ssize_t bytesRead = SocketOps::Recv(_reader, data, sizeof(data), 0, ec);
        if (bytesRead == sizeof(data)) {
            continue;
        }
        if (bytesRead > 0) {
            return true;
        }
        if (bytesRead == 0) {
            return false;
        }
        if (ec == SocketErrors::WouldBlock || ec == SocketErrors::TryAgain) {
            return true;
        }
        return false;
    }
#else
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
#endif
}