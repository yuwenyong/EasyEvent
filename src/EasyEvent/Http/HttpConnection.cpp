//
// Created by yuwenyong.vincent on 2021/7/31.
//

#include "EasyEvent/Http/HttpConnection.h"
#include "EasyEvent/Logging/LogStream.h"
#include "EasyEvent/Common/StrUtil.h"

using namespace std::string_view_literals;

EasyEvent::HttpConnection::HttpConnection(const ConnectionPtr &stream,
                                          HttpServerConnectionDelegatePtr connectionDelegate,
                                          bool xheaders,
                                          const HttpConnectionOptions *options,
                                          HttpRequestContextUPtr &&context)
    : _ioLoop(stream->getIOLoop())
    , _stream(stream)
    , _connectionDelegate(std::move(connectionDelegate))
    , _xheaders(xheaders)
    , _context(std::move(context)) {

    if (options == nullptr) {
        options = &HttpConnectionOptions::Default;
    }
    _noKeepAlive = options->isNoKeepAlive();
    _decompress = options->isDecompressEnabled();
    _chunkSize = options->getChunkSize();
    _maxHeaderSize = options->getMaxHeaderSize();
    _headerTimeout = options->getHeaderTimeout();
    _maxBodySize = options->getMaxBodySize() != 0 ? options->getMaxBodySize() : _stream->getMaxReadBufferSize();
    _bodyTimeout = options->getBodyTimeout();


}

void EasyEvent::HttpConnection::writeHeaders(ResponseStartLine startLine, HttpHeaders &headers, const void *chunk,
                                             size_t length, Task<void()>&& callback) {
    StringVec lines;

    lines.emplace_back("HTTP/1.1 " + std::to_string(startLine.getCode()) + " " + startLine.getReason());
    _responseStartLine = std::move(startLine);
    _chunkingOutput = _requestStartLine.getVersion() == "HTTP/1.1" &&
                      _responseStartLine.getCode() != 204 &&
                      _responseStartLine.getCode() != 304 &&
                      (_responseStartLine.getCode() < 100 || _responseStartLine.getCode() >= 200) &&
                      !headers.has("Content-Length") &&
                      !headers.has("Transfer-Encoding");

    if (_requestStartLine.getVersion() == "HTTP/1.1" && _disconnectOnFinish) {
        headers["Connection"] = "close"sv;
    }
    if (_requestStartLine.getVersion() == "HTTP/1.0" &&
        StrUtil::toLowerCopy(_requestHeaders->get("Connection")) == "keep-alive") {
        headers["Connection"] = "Keep-Alive"sv;
    }
    if (_chunkingOutput) {
        headers["Transfer-Encoding"] = "chunked"sv;
    }
    if (_requestStartLine.getMethod() == "HEAD" || _responseStartLine.getCode() == 304) {
        _expectedContentRemaining = 0;
    } else if (headers.has("Content-Length")) {
        _expectedContentRemaining = std::stoi(headers.at("Content-Length"));
    } else {
        _expectedContentRemaining = std::nullopt;
    }
    headers.getAll([&lines](const std::string &name, const std::string &value) {
        lines.emplace_back(name + ": " + value);
    });
    for (auto &line: lines) {
        if (line.find('\n') != std::string::npos) {
            throwError(HttpOutputErrors::NewlineInHeaders, "HttpConnection", line);
        }
    }
    auto data = StrUtil::join(lines, "\r\n") + "\r\n\r\n";
    if (chunk != nullptr && length != 0) {
        data.append(formatChunk(chunk, length));
    }
    if (callback) {
        _writeCallback = std::move(callback);
    }
    ++_pendingWrite;
    _stream->write(std::move(data), [this, self=shared_from_this()]() {
        onWriteComplete();
    });
}

void EasyEvent::HttpConnection::write(const void *chunk, size_t length, Task<void()> &&callback) {
    if (callback) {
        _writeCallback = std::move(callback);
    }
    ++_pendingWrite;
    _stream->write(formatChunk(chunk, length), [this, self=shared_from_this()]() {
        onWriteComplete();
    });
}

void EasyEvent::HttpConnection::finish() {
    if (_expectedContentRemaining && *_expectedContentRemaining != 0 && !_stream->closed()) {
        _stream->close();
        throwError(HttpOutputErrors::WriteLessDataThanContentLength, "HttpConnection");
    }
    if (_chunkingOutput) {
        ++_pendingWrite;
        _stream->write("0\r\n\r\n", [this, self=shared_from_this()]() {
            onWriteComplete();
        });
    }
    _writeFinished = true;
    if (!_readFinished) {
        _disconnectOnFinish = true;
    }
    _stream->setNoDelay(true);
    if (_pendingWrite == 0) {
        finishRequest();
    }
}

void EasyEvent::HttpConnection::startServing() {
    _stream->setCloseCallback([this, self=shared_from_this()](std::error_code ec) {
        UnusedParameter(ec);
        onConnectionClose();
    });

    readHeaders();
}

std::string EasyEvent::HttpConnection::formatChunk(const void *data, size_t length) {
    if (_expectedContentRemaining) {
        _expectedContentRemaining = *_expectedContentRemaining - (ssize_t)length;
        if (*_expectedContentRemaining < 0) {
            close();
            throwError(HttpOutputErrors::WriteMoreDataThanContentLength, "HttpConnection");
        }
    }
    if (_chunkingOutput && length != 0) {
        std::string chunk;
        chunk = StrUtil::toHexString(length);
        chunk.reserve(length + 4 + chunk.length());
        chunk.append("\r\n");
        chunk.append((const char *)data, length);
        chunk.append("\r\n");
        return chunk;
    } else {
        return std::string{(const char *)data, length};
    }
}

void EasyEvent::HttpConnection::onWriteComplete() {
    if (--_pendingWrite > 0) {
        return;
    }
    if (_writeCallback) {
        auto callback = std::move(_writeCallback);
        _writeCallback = nullptr;

        try {
            callback();
        } catch (std::exception& e) {
            LOG_INFO(getLogger()) << "Uncaught exception in write callback from " << *_context << ": " << e.what();
        } catch (...) {
            LOG_INFO(getLogger()) << "Unknown exception in write callback from " << *_context;
        }
    }
    if (_writeFinished) {
        finishRequest();
    }
}

void EasyEvent::HttpConnection::finishRequest() {
    if (_xheaders) {
        _context->unapplyXheaders();
    }
    if (_delegate) {
        auto delegate = std::move(_delegate);
        try {
            delegate->onRequestFinished();
        } catch (std::exception& e) {
            LOG_INFO(getLogger()) << "Uncaught exception in delegate request finished callback from " << *_context << ": " << e.what();
        } catch (...) {
            LOG_INFO(getLogger()) << "Unknown exception in delegate request finished callback from " << *_context;
        }
    }
    clearCallbacks();
    if (_disconnectOnFinish) {
        close();
        return;
    }
    if (!_stream->closed()) {
        _stream->setNoDelay(false);
        readHeaders();
    }
}

void EasyEvent::HttpConnection::readHeaders() {
    clearRequestState();
    _stream->readUntilRegex("\r?\n\r?\n", [this, self=shared_from_this()](std::string data) {
        onHeaders(std::move(data));
    }, _maxHeaderSize);

    if (_headerTimeout) {
        _headerTimer = _stream->getIOLoop()->callLater(_headerTimeout, [this, self=shared_from_this()]() {
            onHeadersTimeout();
        });
    }
}

void EasyEvent::HttpConnection::onHeaders(std::string data) {
    if (!_headerTimer.expired()) {
        _ioLoop->removeTimeout(_headerTimer);
    }

    try
    {
        parseHeaders(data);
        _disconnectOnFinish = !canKeepAlive(_requestStartLine, *_requestHeaders);

        if (_decompress) {
            if (_requestHeaders->get("Content-Encoding") == "gzip") {
                _decompressor = std::make_unique<GzipDecompressor>();
                _requestHeaders->add("X-Consumed-Content-Encoding", _requestHeaders->at("Content-Encoding"));
                _requestHeaders->erase("Content-Encoding");
            }
        }

        if (_xheaders) {
            _context->applyXheaders(*_requestHeaders);
        }

        _delegate = _connectionDelegate->startRequest(shared_from_this(), _requestStartLine, _requestHeaders);
        Assert(_delegate != nullptr);

        if (_requestHeaders->get("Expect") == "100-continue" && !_writeFinished) {
            write("HTTP/1.1 100 (Continue)\r\n\r\n");
        }
        readBody();
    } catch (std::system_error& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(e.code());
    } catch (std::exception& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(UserErrors::UnexpectedBehaviour);
    } catch (...) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": Unknown errors";
        _stream->close(UserErrors::UnexpectedBehaviour);
    }
}

void EasyEvent::HttpConnection::onHeadersTimeout() {
    LOG_INFO(getLogger()) << "Read HTTP headers failed from " << *_context << ": Timeout";
    _stream->close(HttpConnectionErrors::ReadHttpHeaderTimeout);
}

void EasyEvent::HttpConnection::readBody() {
    if (_requestHeaders->has("Content-Length")) {
        if (_requestHeaders->has("Transfer-Encoding")) {
            throwError(HttpInputErrors::TransferEncodingAndContentLengthConflict, "HttpConnection");
        }
        if (_requestHeaders->get("Content-Length").find(',') != std::string::npos) {
            auto pieces = StrUtil::split(std::string_view(_requestHeaders->at("Content-Length")), ',');
            for (auto &piece: pieces) {
                StrUtil::trim(piece);
            }
            for (auto &piece: pieces) {
                if (piece != pieces[0]) {
                    throwError(HttpInputErrors::MultipleUnequalContentLength, "HttpConnection",
                               _requestHeaders->at("Content-Length"));
                }
            }
            (*_requestHeaders)["Content-Length"] = pieces[0];
        }
        size_t contentLength;
        try {
            contentLength = (size_t)std::stoul(_requestHeaders->at("Content-Length"));
        } catch (...) {
            throwError(HttpInputErrors::ContentLengthInvalidType, "HttpConnection",
                       _requestHeaders->at("Content-Length"));
        }
        if (contentLength > _maxBodySize) {
            _stream->close(HttpInputErrors::ContentLengthTooLong);
            return;
        }
        readFixedBody(contentLength);
        if (_bodyTimeout) {
            _bodyTimer = _stream->getIOLoop()->callLater(_bodyTimeout, [this, self=shared_from_this()]() {
                onBodyTimeout();
            });
        }
        return;
    }
    if (StrUtil::toLowerCopy(_requestHeaders->get("Transfer-Encoding", "")) == "chunked") {
        readChunkLength();
        if (_bodyTimeout) {
            _bodyTimer = _stream->getIOLoop()->callLater(_bodyTimeout, [this, self=shared_from_this()]() {
                onBodyTimeout();
            });
        }
        return;
    }
    readFinished();
}

void EasyEvent::HttpConnection::readFixedBody(size_t contentLength) {
    if (contentLength > 0) {
        _bytesRead = 0;
        _bytesToRead = contentLength;
        readFixedBodyBlock();
    } else {
        readFinished();
    }
}

void EasyEvent::HttpConnection::readFixedBodyBlock() {
    _stream->readBytes(std::min(_chunkSize, _bytesToRead), [this, self=shared_from_this()](std::string data) {
        onFixedBody(std::move(data));
    }, true);
}

void EasyEvent::HttpConnection::onFixedBody(std::string data) {
    try {
        onDataReceived(std::move(data));
        if (_bytesRead < _bytesToRead) {
            readFixedBodyBlock();
        } else {
            readFinished();
        }
    } catch (std::system_error& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(e.code());
    } catch (std::exception& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(UserErrors::UnexpectedBehaviour);
    } catch (...) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": Unknown errors";
        _stream->close(UserErrors::UnexpectedBehaviour);
    }
}

void EasyEvent::HttpConnection::readChunkLength() {
    _stream->readUntil("\r\n", [this, self=shared_from_this()](std::string data) {
        onChunkLength(std::move(data));
    }, 64);
}

void EasyEvent::HttpConnection::readChunkData(size_t chunkLen) {
    _bytesRead = 0;
    _bytesToRead = chunkLen;
    readChunkDataBlock();
}

void EasyEvent::HttpConnection::readChunkDataBlock() {
    _stream->readBytes(std::min(_chunkSize, _bytesToRead), [this, self=shared_from_this()](std::string data) {
        onChunkData(std::move(data));
    }, true);
}

void EasyEvent::HttpConnection::readChunkEnds() {
    _stream->readBytes(2, [this, self=shared_from_this()](std::string data) {
        onChunkEnds(std::move(data));
    });
}

void EasyEvent::HttpConnection::readLastChunkEnds() {
    _stream->readBytes(2, [this, self=shared_from_this()](std::string data) {
        onLastChunkEnds(std::move(data));
    });
}

void EasyEvent::HttpConnection::onChunkLength(std::string data) {
    try {
        StrUtil::trim(data);
        size_t chunkLen = std::stoul(data, nullptr, 16);
        if (chunkLen == 0) {
            readLastChunkEnds();
        } else {
            if (chunkLen + _totalSize > _maxBodySize) {
                throwError(HttpInputErrors::ChunkedBodyTooLarge, "HttpConnection");
            }
            readChunkData(chunkLen);
        }
    } catch (std::system_error& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(e.code());
    } catch (std::exception& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(UserErrors::UnexpectedBehaviour);
    } catch (...) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": Unknown errors";
        _stream->close(UserErrors::UnexpectedBehaviour);
    }
}

void EasyEvent::HttpConnection::onChunkData(std::string data) {
    try {
        onDataReceived(std::move(data));
        if (_bytesRead < _bytesToRead) {
            readChunkDataBlock();
        } else {
            readChunkEnds();
        }
    } catch (std::system_error& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(e.code());
    } catch (std::exception& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(UserErrors::UnexpectedBehaviour);
    } catch (...) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": Unknown errors";
        _stream->close(UserErrors::UnexpectedBehaviour);
    }
}

void EasyEvent::HttpConnection::onChunkEnds(std::string data) {
    try {
        if (data != "\r\n") {
            throwError(HttpInputErrors::UnexpectedChunkEnds, "HttpConnection");
        }
        readChunkLength();
    } catch (std::system_error& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(e.code());
    } catch (std::exception& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(UserErrors::UnexpectedBehaviour);
    } catch (...) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": Unknown errors";
        _stream->close(UserErrors::UnexpectedBehaviour);
    }
}

void EasyEvent::HttpConnection::onLastChunkEnds(std::string data) {
    try {
        if (data != "\r\n") {
            throwError(HttpInputErrors::ImproperlyTerminateChunkedRequest, "HttpConnection");
        }
        readFinished();
    } catch (std::system_error& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(e.code());
    } catch (std::exception& e) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": " << e.what();
        _stream->close(UserErrors::UnexpectedBehaviour);
    } catch (...) {
        LOG_INFO(getLogger()) << "Malformed HTTP request from " << *_context << ": Unknown errors";
        _stream->close(UserErrors::UnexpectedBehaviour);
    }
}

void EasyEvent::HttpConnection::onDataReceived(std::string data) {
    if (data.empty()) {
        return;
    }

    _bytesRead += data.size();
    _totalSize += data.size();

    if (_decompressor) {
        std::string decompressed;
        decompressed = _decompressor->decompressToString(data, _chunkSize);
        if (!decompressed.empty() && !_writeFinished) {
            _delegate->onDataReceived(std::move(decompressed));
        }

        while (!_decompressor->getUnconsumedTail().empty()) {
            decompressed = _decompressor->decompressToString(_decompressor->getUnconsumedTail(), _chunkSize);
            if (!decompressed.empty() && !_writeFinished) {
                _delegate->onDataReceived(std::move(decompressed));
            }
        }
    } else {
        if (!_writeFinished) {
            _delegate->onDataReceived(std::move(data));
        }
    }
}

void EasyEvent::HttpConnection::readFinished() {
    _readFinished = true;

    if (!_bodyTimer.expired()) {
        _ioLoop->removeTimeout(_bodyTimer);
    }

    if (_decompress) {
        auto tail = _decompressor->flushToString();
        if (!tail.empty()) {
            if (!_writeFinished) {
                _delegate->onDataReceived(std::move(tail));
            }
        }
    }

    if (!_writeFinished) {
        _delegate->onReadCompleted();
    }
}

void EasyEvent::HttpConnection::onBodyTimeout() {
    LOG_INFO(getLogger()) << "Timeout reading body from " << *_context << ": Timeout";
    _stream->close(HttpConnectionErrors::ReadHttpBodyTimeout);
}

void EasyEvent::HttpConnection::onConnectionClose() {
    if (!_headerTimer.expired()) {
        _ioLoop->removeTimeout(_headerTimer);
    }

    if (!_bodyTimer.expired()) {
        _ioLoop->removeTimeout(_bodyTimer);
    }

    if (_closeCallback) {
        auto callback = std::move(_closeCallback);
        try {
            callback();
        } catch (std::exception& e) {
            LOG_INFO(getLogger()) << "Uncaught exception in close callback from " << *_context << ": " << e.what();
        } catch (...) {
            LOG_INFO(getLogger()) << "Unknown exception in close callback from " << *_context;
        }
    }
    clearCallbacks();
    if (_delegate) {
        auto delegate = std::move(_delegate);
        try {
            delegate->onConnectionClose();
        } catch (std::exception& e) {
            LOG_INFO(getLogger()) << "Uncaught exception in delegate connection close callback from " << *_context << ": " << e.what();
        } catch (...) {
            LOG_INFO(getLogger()) << "Unknown exception in delegate connection close callback from " << *_context;
        }
    }
    if (_xheaders) {
        _context->unapplyXheaders();
    }


    _stream.takeover();
}

void EasyEvent::HttpConnection::parseHeaders(std::string_view data) {
    StrUtil::trimLeft(data, [](char c) { return c == '\r' || c == '\n'; });
    auto eol = data.find('\n');
    std::string_view startLine, rest;
    if (eol != std::string_view::npos) {
        startLine = data.substr(0, eol);
        StrUtil::trimRight(startLine, [](char c) { return c == '\r'; });
        rest = data.substr(eol);
    } else {
        startLine = data;
    }
    _requestStartLine = RequestStartLine::parse(startLine);
    _requestHeaders = HttpHeaders::parseAsShared(rest);
}

bool EasyEvent::HttpConnection::canKeepAlive(const RequestStartLine &startLine, const HttpHeaders &headers) {
    if (_noKeepAlive) {
        return false;
    }
    std::string connectionHeader = headers.get("Connection");
    if (!connectionHeader.empty()) {
        StrUtil::toLower(connectionHeader);
    }
    if (startLine.getVersion() == "HTTP/1.1") {
        return connectionHeader != "close";
    } else if (headers.has("Content-Length") ||
               StrUtil::toLowerCopy(headers.get("Transfer-Encoding", "")) == "chunked" ||
               startLine.getMethod() == "HEAD" ||
               startLine.getMethod() == "GET") {
        return connectionHeader == "keep-alive";
    }
    return false;
}