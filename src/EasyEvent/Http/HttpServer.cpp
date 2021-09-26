//
// Created by yuwenyong.vincent on 2021/9/21.
//

#include "EasyEvent/Http/HttpServer.h"
#include "EasyEvent/Http/HttpConnection.h"
#include "EasyEvent/Http/HttpServerRequest.h"
#include "EasyEvent/Logging/LogStream.h"
#include "EasyEvent/Common/StrUtil.h"


EasyEvent::HttpServer::HttpServer(
        IOLoop *ioLoop,
        HttpServerConnectionDelegatePtr delegate,
        const HttpServerOptions &options,
        SslContextPtr sslContext,
        size_t maxBufferSize,
        MakeSharedTag tag)
        : TcpServer(ioLoop, std::move(sslContext), maxBufferSize, tag)
        , _delegate(std::move(delegate))
        , _xheaders(options.getXheaders())
        , _protocol(options.getProtocol())
        , _trustedDownstream(options.getTrustedDownstream()) {

    _connParams.enableDecompress(options.getDecompressRequest());

    if (options.getChunkSize() != 0) {
        _connParams.setChunkSize(options.getChunkSize());
    }

    if (options.getMaxHeaderSize() != 0) {
        _connParams.setMaxHeaderSize(options.getMaxHeaderSize());
    }
    _connParams.setHeaderTimeout(options.getIdleConnectionTimeout() ? options.getIdleConnectionTimeout() : Time::seconds(3600));
    _connParams.setMaxBodySize(options.getMaxBodySize());
    _connParams.setBodyTimeout(options.getBodyTimeout());
    _connParams.setNoKeepAlive(options.isNoKeepAlive());
}

void EasyEvent::HttpServer::handleConnection(ConnectionPtr connection, const Address &address) {
    HttpRequestContextUPtr context = std::make_unique<HttpRequestContext>(connection, &address, _protocol, _trustedDownstream);
    HttpConnectionPtr httpConnection = std::make_shared<HttpConnection>(connection, _delegate, _xheaders, &_connParams, std::move(context));
    httpConnection->startServing();
}

std::shared_ptr<EasyEvent::HttpServer> EasyEvent::HttpServer::create(
        IOLoop *ioLoop, Task<void(HttpServerRequestPtr)> &&callback, const HttpServerOptions &options,
        SslContextPtr sslContext, size_t maxBufferSize) {
    auto delegate = std::make_shared<CallableConnectionAdapter>(std::move(callback));
    return create(ioLoop, std::move(delegate), options, std::move(sslContext), maxBufferSize);
}

EasyEvent::HttpMessageDelegatePtr EasyEvent::CallableConnectionAdapter::startRequest(
        const HttpConnectionPtr &connection,
        const RequestStartLine &startLine,
        const HttpHeadersPtr &headers) {
    HttpMessageDelegatePtr delegate = std::make_shared<CallableAdapter>(connection, shared_from_this());
    delegate->onHeadersReceived(startLine, headers);
    return delegate;
}

void EasyEvent::CallableAdapter::onHeadersReceived(const RequestStartLine &startLine, const HttpHeadersPtr &headers) {
    _request = std::make_shared<HttpServerRequest>(_connection.shared(), &startLine, headers);
}

void EasyEvent::CallableAdapter::onDataReceived(std::string chunk) {
    _chunks.emplace_back(std::move(chunk));
}

void EasyEvent::CallableAdapter::onReadCompleted() {
    if (!_chunks.empty()) {
        _request->setBody(StrUtil::join(_chunks, ""));
        StringVec errors;
        _request->parseBody(errors);
        for(auto& error: errors) {
            LOG_WARN(_connection->getLogger()) << error;
        }
    }
    _delegate->doRequestCallback(_request);
}

void EasyEvent::CallableAdapter::onRequestFinished() {
    _chunks.clear();
    _connection.takeover();
    if (_request) {
        _request->onDetached();
    }
}

void EasyEvent::CallableAdapter::onConnectionClose() {
    _chunks.clear();
    _connection.takeover();
    if (_request) {
        _request->onDetached();
    }
}