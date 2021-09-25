//
// Created by yuwenyong.vincent on 2021/7/31.
//

#ifndef EASYEVENT_HTTP_HTTPCONNECTION_H
#define EASYEVENT_HTTP_HTTPCONNECTION_H

#include "EasyEvent/Http/HttpServerUtil.h"


namespace EasyEvent {

    class EASY_EVENT_API HttpConnection: public std::enable_shared_from_this<HttpConnection> {
    public:
        HttpConnection(const ConnectionPtr &stream, HttpServerConnectionDelegatePtr connectionDelegate,
                       bool xheaders, const HttpConnectionOptions *options, HttpRequestContextUPtr &&context);

        IOLoop* getIOLoop() {
            return _ioLoop;
        }

        Logger* getLogger() {
            Logger* logger = _connectionDelegate->getLogger();
            return logger != nullptr ? logger : _ioLoop->getLogger();
        }

        const HttpRequestContext& getContext() const {
            return *_context;
        }

        void setCloseCallback(Task<void ()>&& callback) {
            _closeCallback = std::move(callback);
        }

        void close() {
            _stream->close();
        }

        void setBodyTimeout(Time timeout) {
            _bodyTimeout = timeout;
        }

        void setMaxBodySize(size_t maxBodySize) {
            _maxBodySize = maxBodySize;
        }

        void writeHeaders(ResponseStartLine startLine, HttpHeaders& headers, const void *chunk = nullptr,
                          size_t length = 0, Task<void()>&& callback = nullptr);

        void writeHeaders(ResponseStartLine startLine, HttpHeaders &headers, const char *chunk,
                          Task<void()>&& callback = nullptr) {
            writeHeaders(std::move(startLine), headers, chunk, strlen(chunk), std::move(callback));
        }

        void writeHeaders(ResponseStartLine startLine, HttpHeaders &headers, const std::string &chunk,
                          Task<void()>&& callback = nullptr) {
            writeHeaders(std::move(startLine), headers, chunk.c_str(), chunk.size(), std::move(callback));
        }

        void writeHeaders(ResponseStartLine startLine, HttpHeaders &headers, const std::vector<int8> &chunk,
                          Task<void()>&& callback = nullptr) {
            writeHeaders(std::move(startLine), headers, chunk.data(), chunk.size(), std::move(callback));
        }

        void writeHeaders(ResponseStartLine startLine, HttpHeaders &headers, const std::vector<uint8> &chunk,
                          Task<void()>&& callback = nullptr) {
            writeHeaders(std::move(startLine), headers, chunk.data(), chunk.size(), std::move(callback));
        }

        void write(const void* chunk, size_t length, Task<void()>&& callback = nullptr);

        void write(const char* data, Task<void()>&& callback= nullptr) {
            write(data, strlen(data), std::move(callback));
        }

        void write(const std::string& data, Task<void()>&& callback= nullptr) {
            write(data.data(), data.size(), std::move(callback));
        }

        void write(const std::vector<int8>& data, Task<void()>&& callback= nullptr) {
            write(data.data(), data.size(), std::move(callback));
        }

        void write(const std::vector<uint8>& data, Task<void()>&& callback= nullptr) {
            write(data.data(), data.size(), std::move(callback));
        }

        void finish();

        void startServing();
    private:
        std::string formatChunk(const void *data, size_t length);

        void onWriteComplete();

        void finishRequest();

        void clearRequestState() {
            _totalSize = 0;
            _disconnectOnFinish = false;
            clearCallbacks();
            _chunkingOutput = false;
            _pendingWrite = 0;
            _readFinished = false;
            _writeFinished = false;
            _decompressor.reset();
            _delegate = nullptr;
            _requestHeaders = nullptr;
            _expectedContentRemaining = std::nullopt;
        }

        void clearCallbacks() {
            _writeCallback = nullptr;
            _closeCallback = nullptr;
        }

        void readHeaders();

        void onHeaders(std::string data);

        void onHeadersTimeout();

        void readBody();

        void readFixedBody(size_t contentLength);

        void readFixedBodyBlock();

        void onFixedBody(std::string data);

        void readChunkLength();

        void readChunkData(size_t chunkLen);

        void readChunkDataBlock();

        void readChunkEnds();

        void readLastChunkEnds();

        void onChunkLength(std::string data);

        void onChunkData(std::string data);

        void onChunkEnds(std::string data);

        void onLastChunkEnds(std::string data);

        void onDataReceived(std::string data);

        void readFinished();

        void onBodyTimeout();

        bool canKeepAlive(const RequestStartLine& startLine, const HttpHeaders& headers);

        void parseHeaders(std::string_view data);

        void onConnectionClose();

        IOLoop* _ioLoop;
        ConnectionHolder _stream;
        HttpServerConnectionDelegatePtr _connectionDelegate;
        bool _noKeepAlive;
        bool _xheaders;
        bool _decompress;
        bool _disconnectOnFinish{false};
        bool _chunkingOutput{false};
        int _pendingWrite{0};
        bool _writeFinished{false};
        bool _readFinished{false};
        size_t _chunkSize;
        size_t _maxHeaderSize;
        size_t _maxBodySize;
        size_t _bytesToRead{0};
        size_t _bytesRead{0};
        size_t _totalSize{0};
        Time _headerTimeout;
        Time _bodyTimeout;
        HttpRequestContextUPtr _context;
        Task<void ()> _writeCallback;
        Task<void ()> _closeCallback;
        HttpHeadersPtr _requestHeaders;
        RequestStartLine _requestStartLine;
        ResponseStartLine _responseStartLine;

        std::unique_ptr<GzipDecompressor> _decompressor;
        TimerHandle _headerTimer;
        TimerHandle _bodyTimer;
        HttpMessageDelegatePtr _delegate;
        std::optional<ssize_t> _expectedContentRemaining;
    };

}

#endif //EASYEVENT_HTTP_HTTPCONNECTION_H
