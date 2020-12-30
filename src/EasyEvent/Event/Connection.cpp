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
    if (closed()) {
        LOG_WARN(_logger) << "Got events for closed connection.";
        return;
    }

    try {
        if (_connecting) {
            handleConnect();
        }
        if (closed()) {
            return;
        }
        if ((events & IO_EVENT_READ) != 0) {
            handleRead();
        }
        if (closed()) {
            return;
        }
        if ((events & IO_EVENT_WRITE) != 0) {
            handleWrite();
        }
        if (closed()) {
            return;
        }
        if ((events & IO_EVENT_ERROR) != 0) {
            std::error_code ec;
            int err = getFdError(ec);
            throwError(ec, "Connection");
            _error = {err, getSocketErrorCategory()};
            _ioLoop->addCallback([this, self=shared_from_this()]() {
                close();
            });
            return;
        }
        IOEvents state = IO_EVENT_ERROR;
        if (reading()) {
            state |= IO_EVENT_READ;
        }
        if (writing()) {
            state |= IO_EVENT_WRITE;
        }
        if (state == IO_EVENT_ERROR && _readBuffer.getActiveSize() == 0) {
            state |= IO_EVENT_READ;
        }
        if (state != _state) {
            Assert(_state != IO_EVENT_NONE);
            _state = state;
            _ioLoop->updateHandler(shared_from_this(), _state);
        }
    } catch (std::system_error& e) {
        if (e.code() == EventErrors::UnsatisfiableRead) {
            LOG_INFO(_logger) << "Unsatisfiable read, closing connection.";
            close(EventErrors::UnsatisfiableRead);
        } else {
            LOG_ERROR(_logger) << "Uncaught system error, closing connection: " << e.what();
            close(e.code());
            throw;
        }
    } catch (std::exception& e) {
        LOG_ERROR(_logger) << "Uncaught exception closing connection: " << e.what();
        close(UserErrors::UnexpectedBehaviour);
        throw;
    } catch (...) {
        LOG_ERROR(_logger) << "Uncaught unknown exception closing connection";
        close(UserErrors::UnexpectedBehaviour);
        throw;
    }
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

void EasyEvent::Connection::connect(const Address &address, Task<void(std::error_code)> &&callback) {
    _connecting = true;
    _connectCallback = std::move(callback);

    std::error_code ec;
    SocketOps::Connect(_socket, address, ec);
    if (ec && !isInProgress(ec) && !isWouldBlock(ec)) {
        runConnectCallback(ec);
    } else {
        addIOState(IO_EVENT_WRITE);
    }
}

void EasyEvent::Connection::readUntilRegex(const std::string &regex, Task<void(std::string)> &&callback, size_t maxBytes) {
    setReadCallback(std::move(callback));

    _readRegex = std::regex(regex);
    _readMaxBytes = maxBytes;

    try {
        tryInlineRead();
    } catch (std::system_error& e) {
        if (e.code() == EventErrors::UnsatisfiableRead) {
            LOG_INFO(_logger) << "Unsatisfiable read, closing connection";
            close(e.code());
        } else {
            throw;
        }
    }
}

void EasyEvent::Connection::readUntil(std::string delimiter, Task<void(std::string)> &&callback, size_t maxBytes) {
    setReadCallback(std::move(callback));

    _readDelimiter = std::move(delimiter);
    _readMaxBytes = maxBytes;

    try {
        tryInlineRead();
    } catch (std::system_error& e) {
        if (e.code() == EventErrors::UnsatisfiableRead) {
            LOG_INFO(_logger) << "Unsatisfiable read, closing connection";
            close(e.code());
        } else {
            throw;
        }
    }
}

void EasyEvent::Connection::readBytes(size_t numBytes, Task<void(std::string)> &&callback, bool partial) {
    setReadCallback(std::move(callback));

    _readBytes = numBytes;
    _readPartial = partial;

    tryInlineRead();
}

void EasyEvent::Connection::write(const void *data, size_t size, Task<void()> &&callback) {
    checkClosed();
    if (size > 0) {
        checkWriteBuffer(size);
        _writeBuffer.write(data, size);
    }
    if (callback) {
        _writeCallback = std::move(callback);
    }
    if (!_connecting) {
        handleWrite();
        if (!_writeBuffer.empty()) {
            addIOState(IO_EVENT_WRITE);
        }
        maybeAddErrorListener();
    }
}

void EasyEvent::Connection::write(std::string &&data, Task<void()> &&callback) {
    checkClosed();
    if (data.size() > 0) {
        checkWriteBuffer(data.size());
        _writeBuffer.write(std::move(data));
    }
    if (callback) {
        _writeCallback = std::move(callback);
    }
    if (!_connecting) {
        handleWrite();
        if (!_writeBuffer.empty()) {
            addIOState(IO_EVENT_WRITE);
        }
        maybeAddErrorListener();
    }
}

void EasyEvent::Connection::write(std::vector<int8> &&data, Task<void()> &&callback) {
    checkClosed();
    if (data.size() > 0) {
        checkWriteBuffer(data.size());
        _writeBuffer.write(std::move(data));
    }
    if (callback) {
        _writeCallback = std::move(callback);
    }
    if (!_connecting) {
        handleWrite();
        if (!_writeBuffer.empty()) {
            addIOState(IO_EVENT_WRITE);
        }
        maybeAddErrorListener();
    }
}

void EasyEvent::Connection::write(std::vector<uint8> &&data, Task<void()> &&callback) {
    checkClosed();
    if (data.size() > 0) {
        checkWriteBuffer(data.size());
        _writeBuffer.write(std::move(data));
    }
    if (callback) {
        _writeCallback = std::move(callback);
    }
    if (!_connecting) {
        handleWrite();
        if (!_writeBuffer.empty()) {
            addIOState(IO_EVENT_WRITE);
        }
        maybeAddErrorListener();
    }
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
                } catch (std::system_error& e) {
                    LOG_ERROR(_logger) << "Uncaught system error in close callback: " << e.what();
                    throw;
                } catch (std::exception& e) {
                    LOG_ERROR(_logger) << "Uncaught exception in close callback: " << e.what();
                    throw;
                } catch (...) {
                    LOG_ERROR(_logger) << "Uncaught exception in close callback: Unknown error.";
                    throw;
                }
            });
        }
        _connectCallback = nullptr;
        _readCallback = nullptr;
        _writeCallback = nullptr;
    }
}

size_t EasyEvent::Connection::readToBufferLoop() {
    std::optional<size_t> targetBytes;
    if (_readBytes > 0) {
        targetBytes = _readBytes;
    } else if (_readMaxBytes > 0) {
        targetBytes = _readMaxBytes;
    } else if (reading()) {
        targetBytes = std::nullopt;
    } else {
        targetBytes = 0;
    }
    size_t nextFindPos = 0;
    size_t pos;

    ++_pendingCallbacks;
    std::shared_ptr<void> localDecCallback(nullptr, [this](void*) {
        --_pendingCallbacks;
    });

    while (!closed()) {
        if (readToBuffer() == 0) {
            break;
        }
        if (targetBytes && _readBuffer.getActiveSize() >= *targetBytes) {
            break;
        }

        if (_readBuffer.getActiveSize() >= nextFindPos) {
            pos = findReadPos();
            if (pos != 0) {
                return pos;
            }
            nextFindPos = _readBuffer.getActiveSize() * 2;
        }
    }
    return findReadPos();
}

void EasyEvent::Connection::handleRead() {
    size_t pos;
    try {
        pos = readToBufferLoop();
    } catch (std::system_error& e) {
        if (e.code() == EventErrors::UnsatisfiableRead) {
            throw;
        }
        LOG_WARN(_logger) << "System error on read: " << e.what();
        close(e.code());
        return;
    } catch (std::exception& e) {
        LOG_WARN(_logger) << "Error on read: " << e.what();
        close(EventErrors::UnexpectedBehaviour);
        return;
    } catch (...) {
        LOG_WARN(_logger) << "Unknown error on read event.";
        close(EventErrors::UnexpectedBehaviour);
        return;
    }
    if (pos > 0) {
        readFromBuffer(pos);
    } else {
        maybeRunCloseCallback();
    }
}

void EasyEvent::Connection::setReadCallback(Task<void(std::string)> &&task) {
    if (_readCallback) {
        std::error_code ec;
        ec = EventErrors::AlreadyReading;
        throwError(ec, "Connection");
    }
    _readCallback = std::move(task);
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
                    } catch (std::system_error& e) {
                        LOG_ERROR(_logger) << "Uncaught system error in read callback: " << e.what();
                        close(e.code());
                        throw;
                    } catch (std::exception& e) {
                        LOG_ERROR(_logger) << "Uncaught exception in read callback: " << e.what();
                        close(EventErrors::ReadCallbackFailed);
                        throw;
                    } catch (...) {
                        LOG_ERROR(_logger) << "Uncaught exception in read callback: Unknown error.";
                        close(EventErrors::ReadCallbackFailed);
                        throw;
                    }
                });
    } else {
        maybeAddErrorListener();
    }
}

void EasyEvent::Connection::tryInlineRead() {
    size_t pos = findReadPos();
    if (pos > 0) {
        readFromBuffer(pos);
        return;
    }
    checkClosed();
    try {
        pos = readToBufferLoop();
    } catch (...) {
        maybeRunCloseCallback();
        throw;
    }
    if (pos > 0) {
        readFromBuffer(pos);
        return;
    }
    if (closed()) {
        maybeRunCloseCallback();
    } else {
        addIOState(IO_EVENT_READ);
    }
}

size_t EasyEvent::Connection::readToBuffer() {
    ssize_t bytesRead;
    std::error_code ec;
    while (true) {
        _readBuffer.prepare(DefaultReadChunkSize);
        bytesRead = readFromFd(_readBuffer.getWritePointer(), _readBuffer.getRemainingSpace(), ec);
        if (ec) {
            if (isWouldBlock(ec)) {
                return 0;
            }
            if (ec == SocketErrors::Interrupted) {
                continue;
            }
            if (isConnReset(ec)) {
                close(ec);
                return 0;
            }
            close(ec);
            throwError(ec, "Connection");
        }
        break;
    }
    if (bytesRead == 0) {
        close();
        return 0;
    }
    Assert(bytesRead > 0);
    _readBuffer.writeCompleted((size_t)bytesRead);

    if (_readBuffer.getActiveSize() > _maxReadBufferSize) {
        LOG_ERROR(_logger) << "Reached maximum read buffer size";
        close();
        throwError(EventErrors::ConnectionBufferFull, "Connection");
    }

    return (size_t)bytesRead;
}

void EasyEvent::Connection::readFromBuffer(size_t pos) {
    _readBytes = 0;
    _readDelimiter.clear();
    _readRegex = std::nullopt;
    _readPartial = false;
    runReadCallback(pos);
}

size_t EasyEvent::Connection::findReadPos() {
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
                checkMaxBytes(numBytes);
                return numBytes;
            }
            checkMaxBytes(_readBuffer.getActiveSize());
        }
    } else if (_readRegex) {
        if (_readBuffer.getActiveSize()) {
            std::cmatch m;
            if (std::regex_search((const char*)_readBuffer.getReadPointer(),
                                  (const char*)_readBuffer.getReadPointer() + _readBuffer.getActiveSize(),
                                  m, *_readRegex)) {
                auto numBytes = (size_t)(m.position(0) + m.length(0));
                checkMaxBytes(numBytes);
                return numBytes;
            }
            checkMaxBytes(_readBuffer.getActiveSize());
        }
    }
    return 0;
}

void EasyEvent::Connection::handleWrite() {
    std::error_code ec;
    while (true) {
        if (_writeBuffer.empty()) {
            break;
        }
        size_t size = _writeBuffer.getReadSize();
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
        size = std::min(size, (std::size_t)128 * 1024);
#endif
        ssize_t numBytes = writeToFd(_writeBuffer.getReadPtr(), size, ec);
        if (numBytes == 0) {
            break;
        } else if (numBytes > 0) {
            _writeBuffer.advance((size_t)numBytes);
        } else {
            if (isWouldBlock(ec)) {
                break;
            } else {
                if (!isConnReset(ec)) {
                    LOG_WARN(_logger) << "Write error: " << ec << "(" << ec.message() << ")";
                }
                close(ec);
                return;
            }
        }
    }

    if (_writeBuffer.empty()) {
        if (_writeCallback) {
            auto callback = std::move(_writeCallback);
            _writeCallback = nullptr;
            ++_pendingCallbacks;

            _ioLoop->addCallback(
                    [this, self=shared_from_this(), callback=std::move(callback)]() {
                        --_pendingCallbacks;
                        LocalAddErrorListener addErrorListener(this);
                        try {
                            callback();
                        } catch (std::system_error& e) {
                            LOG_ERROR(_logger) << "Uncaught system error in write callback: " << e.what();
                            close(e.code());
                            throw;
                        } catch (std::exception& e) {
                            LOG_ERROR(_logger) << "Uncaught exception in write callback: " << e.what();
                            close(EventErrors::WriteCallbackFailed);
                            throw;
                        } catch (...) {
                            LOG_ERROR(_logger) << "Uncaught exception in write callback: Unknown error.";
                            close(EventErrors::WriteCallbackFailed);
                            throw;
                        }
                    });
        }
    }
}

void EasyEvent::Connection::maybeAddErrorListener() {
    if (_pendingCallbacks != 0) {
        return;
    }
    if (_state == IO_EVENT_NONE || _state == IO_EVENT_ERROR) {
        if (closed()) {
            maybeRunCloseCallback();
        } else if (_readBuffer.getActiveSize() == 0 && _closeCallback) {
            addIOState(IO_EVENT_READ);
        }
    }
}

void EasyEvent::Connection::addIOState(IOEvents state) {
    if (closed()) {
        return;
    }
    if (_state == IO_EVENT_NONE) {
        _state = IO_EVENT_ERROR | state;
        _ioLoop->addHandler(shared_from_this(), _state);
    } else if ((_state & state) == 0) {
        _state |= state;
        _ioLoop->updateHandler(shared_from_this(), _state);
    }
}

void EasyEvent::Connection::runConnectCallback(std::error_code ec) {
    if (_connectCallback) {
        auto callback = std::move(_connectCallback);
        _connectCallback = nullptr;
        ++_pendingCallbacks;
        _ioLoop->addCallback([this, ec, self=shared_from_this(), callback=std::move(callback)]() {
            --_pendingCallbacks;
            LocalAddErrorListener addErrorListener(this);
            try {
                callback(ec);
            } catch (std::system_error& e) {
                LOG_ERROR(_logger) << "Uncaught system error in connect callback: " << e.what();
                throw;
            } catch (std::exception& e) {
                LOG_ERROR(_logger) << "Uncaught exception in connect callback: " << e.what();
                throw;
            } catch (...) {
                LOG_ERROR(_logger) << "Uncaught exception in connect callback: Unknown error.";
                throw;
            }
        });
    }
    if (ec) {
        LOG_WARN(_logger) << "Connect error: " << ec << "(" << ec.message() << ")";
        close(ec);
    } else {
        _connecting = false;
    }
}

void EasyEvent::Connection::handleConnect() {
    std::error_code ec;
    int err = getFdError(ec);
    if (ec == SocketErrors::NoProtocolOption) {
        err = 0;
    }
    if (!ec && err != 0) {
        ec = {err, getSocketErrorCategory()};
    }
    runConnectCallback(ec);
}