//
// Created by yuwenyong on 2020/12/12.
//

#include "EasyEvent/Event/Connection.h"
#include "EasyEvent/Common/StrUtil.h"
#include "EasyEvent/Logging/LogStream.h"


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

SocketType EasyEvent::Connection::getFD() const {
    return _socket;
}

void EasyEvent::Connection::closeFD() {
    std::error_code ec;
    SocketOps::Close(_socket, false, ec);
    if (!ec) {
        _socket = InvalidSocket;
    } else {
        throwError(ec, "Connection");
    }
}

void EasyEvent::Connection::readUntilRegex(const std::string &regex, Task<void(std::string)> &&task, size_t maxBytes) {
    std::error_code ec;
    setReadCallback(std::move(task), ec);
    if (ec) {
        throwError(ec, "Connection");
    }
    _readRegex = std::regex(regex);
    _readMaxBytes = maxBytes;

}

void EasyEvent::Connection::close(const std::error_code& error) {
    if (!closed()) {
        if (error) {
            _error = error;
        }
        if (_state != IO_EVENT_NONE) {
            _ioLoop->removeHandler(shared_from_this());
            _state = IO_EVENT_NONE;
        }
        closeFD();
        _closed = true;
    }
    maybeRunCloseCallback();
}

void EasyEvent::Connection::maybeRunCloseCallback() {
    if (closed() && _pendingCallbacks == 0) {
        if (_closeCallback) {
            auto callback = std::move(_closeCallback);
            _closeCallback = nullptr;
            ++_pendingCallbacks;
            _ioLoop->addCallback([this, self=shared_from_this(), callback=std::move(callback)]() {
                --_pendingCallbacks;
                LocalAddErrorListener addErrorListener(this);
                try {
                    callback(_error);
                } catch (std::exception& e) {
                    _error = EventErrors::CloseCallbackFailed;
                    LOG_ERROR(_logger) << "Uncaught exception in close callback: " << e.what();
                    throw;
                } catch (...) {
                    _error = EventErrors::CloseCallbackFailed;
                    LOG_ERROR(_logger) << "Uncaught exception in close callback: Unknown error.";
                    throw;
                }
            });
        }
        _readCallback = nullptr;
        _writeCallback = nullptr;
    }
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
    if (_readCallback) {
        auto callback = std::move(_readCallback);
        _readCallback = nullptr;
        ++_pendingCallbacks;

        _ioLoop->addCallback(
                [this, self=shared_from_this(), callback=std::move(callback), result=std::move(result)]() mutable {
                    --_pendingCallbacks;
                    LocalAddErrorListener addErrorListener(this);
                    try {
                        callback(std::move(result));
                    } catch (std::exception& e) {
                        _error = EventErrors::ReadCallbackFailed;
                        LOG_ERROR(_logger) << "Uncaught exception in read callback: " << e.what();
                        throw;
                    } catch (...) {
                        _error = EventErrors::ReadCallbackFailed;
                        LOG_ERROR(_logger) << "Uncaught exception in read callback: Unknown error.";
                        throw;
                    }
                });
    } else {
        maybeAddErrorListener();
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

void EasyEvent::Connection::readFromBuffer(size_t pos) {
    _readBytes = 0;
    _readDelimiter.clear();
    _readRegex = std::nullopt;
    _readPartial = false;
    runReadCallback(pos);
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

void EasyEvent::Connection::maybeAddErrorListener() {

}