//
// Created by yuwenyong.vincent on 2021/9/21.
//

#ifndef EASYEVENT_HTTP_HTTPSERVER_H
#define EASYEVENT_HTTP_HTTPSERVER_H

#include "EasyEvent/Http/HttpServerUtil.h"
#include "EasyEvent/Event/TcpServer.h"

namespace EasyEvent {

    class EASY_EVENT_API HttpServer: public TcpServer {
    public:
        HttpServer(IOLoop* ioLoop, HttpServerConnectionDelegatePtr delegate, const HttpServerOptions& options,
                   SslContextPtr sslContext, size_t maxBufferSize, MakeSharedTag tag);

        void handleConnection(ConnectionPtr connection, const Address& address) override;

        static std::shared_ptr<HttpServer> create(IOLoop* ioLoop, HttpServerConnectionDelegatePtr delegate,
                                                  const HttpServerOptions& options, SslContextPtr sslContext= nullptr,
                                                 size_t maxBufferSize=0) {
            return std::make_shared<HttpServer>(ioLoop, std::move(delegate), options, std::move(sslContext),
                                                maxBufferSize, MakeSharedTag{});
        }

        static std::shared_ptr<HttpServer> create(IOLoop* ioLoop, Task<void (HttpServerRequestPtr)> &&callback,
                                                  const HttpServerOptions& options, SslContextPtr sslContext= nullptr,
                                                  size_t maxBufferSize=0);
    private:
        HttpServerConnectionDelegatePtr _delegate;
        bool _xheaders;
        std::string _protocol;
        HttpConnectionOptions _connParams;
        StringSet _trustedDownstream;
    };

    class EASY_EVENT_API CallableConnectionAdapter
            : public HttpServerConnectionDelegate
            , public std::enable_shared_from_this<CallableConnectionAdapter> {
    public:
        CallableConnectionAdapter(Task<void (HttpServerRequestPtr)>&& requestCallback)
            : _requestCallback(std::move(requestCallback)) {

        }

        HttpMessageDelegatePtr startRequest(const HttpConnectionPtr &connection, const RequestStartLine &startLine,
                                            const HttpHeadersPtr &headers);

        void doRequestCallback(HttpServerRequestPtr request) {
            _requestCallback(std::move(request));
        }

    private:
        Task<void (HttpServerRequestPtr)> _requestCallback;
    };

    class EASY_EVENT_API CallableAdapter: public HttpMessageDelegate {
    public:
        CallableAdapter(const HttpConnectionPtr &connection, std::shared_ptr<CallableConnectionAdapter> delegate)
            : _connection(connection)
            , _delegate(std::move(delegate)) {

        }

        void onHeadersReceived(const RequestStartLine& startLine, const HttpHeadersPtr& headers) override;

        void onDataReceived(std::string chunk) override;

        void onReadCompleted() override;

        void onRequestFinished() override;

        void onConnectionClose() override;
    private:
        HttpConnectionHolder _connection;
        HttpServerRequestPtr _request;
        std::shared_ptr<CallableConnectionAdapter> _delegate;
        StringVec _chunks;
    };

}

#endif //EASYEVENT_HTTP_HTTPSERVER_H
