//
// Created by yuwenyong on 2020/12/12.
//

#include "EasyEvent/Event/Connection.h"
#include "EasyEvent/Common/StrUtil.h"


EasyEvent::Connection::Connection(IOLoop *ioLoop, SocketType socket, size_t maxReadBufferSize, size_t maxWriteBufferSize)
                                  : _ioLoop(ioLoop ? ioLoop : IOLoop::current())
                                  , _logger(_ioLoop->getLogger())
                                  , _socket(socket)
                                  , _maxReadBufferSize(maxReadBufferSize ? maxReadBufferSize : DefaultMaxReadBufferSize)
                                  , _maxWriteBufferSize(maxWriteBufferSize)
                                  , _readBuffer(DefaultReadBufferCapacity)
                                  , _writeBuffer(InitialWriteBufferSize) {

    SocketOps::SetNonblock(_socket, true);

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

void EasyEvent::Connection::runReadCallback(size_t size) {
    auto result = consume(size);
    Task<void(std::string)> callback = std::move(_readCallback);
    _readCallback = nullptr;
    if (callback) {

    } else {

    }
}

void EasyEvent::Connection::tryInlineRead(std::error_code &ec) {
    size_t pos = findReadPos(ec);
    if (ec) {
        return;
    }
    if (pos > 0) {

    }
}

size_t EasyEvent::Connection::findReadPos(std::error_code &ec) {
    if (_readBytes > 0 &&
        (_readBuffer.getActiveSize() >= _readBytes ||
        (_readPartial && _readBuffer.getActiveSize() > 0))) {
        size_t numBytes = std::min(_readBytes, _readBuffer.getActiveSize());
        return numBytes;
    } else if (!_readDelimiter.empty()) {
        if (_readBuffer.getActiveSize()) {
            const char* loc = StrUtil::find((char*)_readBuffer.getReadPointer(), _readBuffer.getActiveSize(),
                                            _readDelimiter.c_str(), _readDelimiter.size());
            if (loc != nullptr) {
                size_t numBytes = (size_t)(loc - (const char*)_readBuffer.getReadPointer()) + _readDelimiter.size();
                checkMaxBytes(numBytes, ec);
                return numBytes;
            }
            checkMaxBytes(_readBuffer.getActiveSize(), ec);
        }
    } else if (_readRegex) {
        if (_readBuffer.getActiveSize()) {
            std::cmatch m;
            if (std::regex_search((const char*)_readBuffer.getReadPointer(),
                                  (const char*)_readBuffer.getReadPointer() + _readBuffer.getActiveSize(),
                                  m, *_readRegex)) {
                auto numBytes = (size_t)(m.position(0) + m.length(0));
                checkMaxBytes(numBytes, ec);
                return numBytes;
            }
            checkMaxBytes(_readBuffer.getActiveSize(), ec);
        }
    }
    return 0;
}