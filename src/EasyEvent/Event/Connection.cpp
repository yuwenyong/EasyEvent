//
// Created by yuwenyong on 2020/12/12.
//

#include "EasyEvent/Event/Connection.h"


EasyEvent::Connection::Connection(IOLoop *ioLoop, SocketType socket, size_t maxReadBufferSize, size_t maxWriteBufferSize)
                                  : _ioLoop(ioLoop ? ioLoop : IOLoop::current())
                                  , _logger(_ioLoop->getLogger())
                                  , _socket(socket)
                                  , _maxReadBufferSize(maxReadBufferSize ? maxReadBufferSize : DefaultMaxReadBufferSize)
                                  , _maxWriteBufferSize(maxWriteBufferSize)
                                  , _readBuffer(DefaultReadBufferCapacity)
                                  , _writeBuffer(InitialWriteBufferSize) {

}

EasyEvent::Connection::~Connection() noexcept {
    if (_socket != InvalidSocket) {
        std::error_code ignoredError;
        SocketOps::Close(_socket, true, ignoredError);
    }
}

void EasyEvent::Connection::handleEvents(IOEvents events) {

}

SocketType EasyEvent::Connection::getSocket() const {
    return _socket;
}

void EasyEvent::Connection::closeSocket() {
    std::error_code ec;
    closeSocket(ec);
    throwError(ec, "Connection");
}

void EasyEvent::Connection::readUntilRegex(const std::string &regex, Task<void(std::string)> &&task, size_t maxBytes,
                                           std::error_code &ec) {
    setReadCallback(std::move(task), ec);
    if (ec) {
        return;
    }
    _readRegex = std::regex(regex);
    _readMaxBytes = maxBytes;

}

void EasyEvent::Connection::setReadCallback(Task<void(std::string)> &&task, std::error_code &ec) {
    if (_readCallback) {
        ec = EventErrors::AlreadyReading;
        return;
    }
    _readCallback = std::move(task);
    ec.assign(0, ec.category());
}