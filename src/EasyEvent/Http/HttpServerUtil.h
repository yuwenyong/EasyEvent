//
// Created by yuwenyong.vincent on 2021/9/5.
//

#ifndef EASYEVENT_HTTP_HTTPSERVERUTIL_H
#define EASYEVENT_HTTP_HTTPSERVERUTIL_H

#include "EasyEvent/Http/HttpBase.h"

namespace EasyEvent {

    class HttpConnection;
    class HttpServerRequest;

    using HttpConnectionPtr = std::shared_ptr<HttpConnection>;
    using HttpConnectionHolder = Holder<HttpConnection>;
    using ConstHttpConnectionPtr = std::shared_ptr<const HttpConnection>;
    using HttpServerRequestPtr = std::shared_ptr<HttpServerRequest>;

    class EASY_EVENT_API HttpMessageDelegate {
    public:
        virtual void onHeadersReceived(const RequestStartLine& startLine,
                                       const HttpHeadersPtr& headers) = 0;

        virtual void onDataReceived(std::string chunk) = 0;

        virtual void onReadCompleted() = 0;

        virtual void onRequestFinished() = 0;

        virtual void onConnectionClose() = 0;

        virtual ~HttpMessageDelegate() = default;
    };

    using HttpMessageDelegatePtr = std::shared_ptr<HttpMessageDelegate>;

    class EASY_EVENT_API HttpServerConnectionDelegate {
    public:
        virtual Logger* getLogger();

        virtual HttpMessageDelegatePtr startRequest(
                const HttpConnectionPtr& connection,
                const RequestStartLine& startLine,
                const HttpHeadersPtr& headers) = 0;

        virtual ~HttpServerConnectionDelegate() = default;
    };

    using HttpServerConnectionDelegatePtr = std::shared_ptr<HttpServerConnectionDelegate>;

    class EASY_EVENT_API HttpServerOptions {
    public:

        bool isNoKeepAlive() const {
            return _noKeepAlive;
        }

        void setNoKeepAlive(bool noKeepAlive) {
            _noKeepAlive = noKeepAlive;
        }

        bool getXheaders() const {
            return _xheaders;
        }

        void setXHeaders(bool xheaders) {
            _xheaders = xheaders;
        }

        const std::string& getProtocol() const {
            return _protocol;
        }

        void setProtocol(std::string protocol) {
            _protocol = std::move(protocol);
        }

        bool getDecompressRequest() const {
            return _decompressRequest;
        }

        void setDecompressRequest(bool decompressRequest) {
            _decompressRequest = decompressRequest;
        }

        size_t getChunkSize() const {
            return _chunkSize;
        }

        void setChunkSize(size_t chunkSize) {
            _chunkSize = chunkSize;
        }

        size_t getMaxHeaderSize() const {
            return _maxHeaderSize;
        }

        void setMaxHeaderSize(size_t maxHeaderSize) {
            _maxHeaderSize = maxHeaderSize;
        }

        Time getIdleConnectionTimeout() const {
            return _bodyTimeout;
        }

        void setIdleConnectionTimeout(Time idleConnectionTimeout) {
            _idleConnectionTimeout = idleConnectionTimeout;
        }

        Time getBodyTimeout() const {
            return _bodyTimeout;
        }

        void setBodyTimeout(Time bodyTimeout) {
            _bodyTimeout = bodyTimeout;
        }

        size_t getMaxBodySize() const {
            return _maxBodySize;
        }

        void setMaxBodySize(size_t maxBodySize) {
            _maxBodySize = maxBodySize;
        }

        const StringSet& getTrustedDownstream() const {
            return _trustedDownstream;
        }

        void setTrustedDownstream(StringSet trustedDownstream) {
            _trustedDownstream = std::move(trustedDownstream);
        }
    private:
        bool _noKeepAlive{false};
        bool _xheaders{false};
        std::string _protocol;
        bool _decompressRequest{false};
        size_t _chunkSize{0};
        size_t _maxHeaderSize{0};
        Time _idleConnectionTimeout;
        Time _bodyTimeout;
        size_t _maxBodySize{0};
        StringSet _trustedDownstream;
    };

    class EASY_EVENT_API HttpConnectionOptions {
    public:
        static constexpr size_t DefaultChunkSize = 65535;
        static constexpr size_t DefaultMaxHeaderSize = 65535;

        bool isNoKeepAlive() const {
            return _noKeepAlive;
        }

        void setNoKeepAlive(bool noKeepAlive) {
            _noKeepAlive = noKeepAlive;
        }

        size_t getChunkSize() const {
            return _chunkSize;
        }

        void setChunkSize(size_t chunkSize) {
            _chunkSize = chunkSize;
        }

        size_t getMaxHeaderSize() const {
            return _maxHeaderSize;
        }

        void setMaxHeaderSize(size_t maxHeaderSize) {
            _maxHeaderSize = maxHeaderSize;
        }

        Time getHeaderTimeout() const {
            return _headerTimeout;
        }

        void setHeaderTimeout(Time headerTimeout) {
            _headerTimeout = headerTimeout;
        }

        size_t getMaxBodySize() const {
            return _maxBodySize;
        }

        void setMaxBodySize(size_t maxBodySize) {
            _maxBodySize = maxBodySize;
        }

        Time getBodyTimeout() const {
            return _bodyTimeout;
        }

        void setBodyTimeout(Time bodyTimeout) {
            _bodyTimeout = bodyTimeout;
        }

        bool isDecompressEnabled() const {
            return _decompress;
        }

        void enableDecompress(bool enable) {
            _decompress = enable;
        }

        static const HttpConnectionOptions Default;
    private:
        bool _noKeepAlive{false};
        size_t _chunkSize{DefaultChunkSize};
        size_t _maxHeaderSize{DefaultMaxHeaderSize};
        Time _headerTimeout{};
        size_t _maxBodySize{0};
        Time _bodyTimeout{};
        bool _decompress{false};
    };

    class EASY_EVENT_API HttpRequestContext {
    public:
        HttpRequestContext(const ConnectionPtr& stream, const Address* address, const std::string& protocol,
                           StringSet trustedDownstream={});

        const std::string& getRemoteIP() const {
            return _remoteIP;
        }

        const std::string& getProtocol() const {
            return _protocol;
        }

        void applyXheaders(const HttpHeaders& headers);

        void unapplyXheaders() {
            _remoteIP = _origRemoteIP;
            _protocol = _origProtocol;
        }
    private:
        std::string _remoteIP;
        std::string _origRemoteIP;
        std::string _protocol;
        std::string _origProtocol;
        StringSet _trustedDownstream;
    };

    EASY_EVENT_API std::ostream& operator<<(std::ostream &os, const HttpRequestContext& context);

    using HttpRequestContextUPtr = std::unique_ptr<HttpRequestContext>;

}

#endif //EASYEVENT_HTTP_HTTPSERVERUTIL_H
