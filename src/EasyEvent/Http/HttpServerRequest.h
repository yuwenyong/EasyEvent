//
// Created by yuwenyong.vincent on 2021/8/15.
//

#ifndef EASYEVENT_HTTP_HTTPSERVERREQUEST_H
#define EASYEVENT_HTTP_HTTPSERVERREQUEST_H

#include "EasyEvent/Http/HttpServerUtil.h"

namespace EasyEvent {

    class EASY_EVENT_API HttpServerRequest {
    public:
        HttpServerRequest(const HttpConnectionPtr& connection,
                          const RequestStartLine* startLine= nullptr,
                          const HttpHeadersPtr& headers = nullptr,
                          const std::string& method = {},
                          const std::string& uri = {},
                          const std::string& version = "HTTP/1.0",
                          std::string body = {},
                          std::string host = {},
                          HttpFiles files = {});

        bool supportsHttp11() const {
            return _version == "HTTP/1.1";
        }

        std::string fullURL() const {
            return _protocol + "://" + _host + _uri;
        }

        Time requestTime() const {
            if (!this->_finishTime) {
                return Time::now() - _startTime;
            } else {
                return _finishTime - _startTime;
            }
        }

        const HttpHeadersPtr& getHTTPHeaders() const {
            return _headers;
        }

        const std::string& getMethod() const {
            return _method;
        }

        const std::string& getURI() const {
            return _uri;
        }

        const std::string& getVersion() const {
            return _version;
        }

        void setBody(std::string body) {
            _body = std::move(body);
        }

        const std::string& getBody() const {
            return _body;
        }

        const std::string& getRemoteIP() const {
            return _remoteIP;
        }

        const std::string& getProtocol() const {
            return _protocol;
        }

        const std::string& getHost() const {
            return _host;
        }

        const std::string& getHostName() const {
            return _hostName;
        }

        const HttpFiles & getFiles() const {
            return _files;
        }

        ConstHttpConnectionPtr getConnection() const {
            return _connection.shared();
        }

        HttpConnectionPtr getConnection() {
            return _connection.shared();
        }

        const std::string& getPath() const {
            return _path;
        }

        const std::string& getQuery() const {
            return _query;
        }

        HttpArguments& arguments() {
            return _arguments;
        }

        const HttpArguments& getArguments() const {
            return _arguments;
        }

        void addArgument(const std::string &name, std::string value) {
            _arguments[name].emplace_back(std::move(value));
        }

        void addArguments(const std::string &name, StringVec values) {
            for (auto &value: values) {
                addArgument(name, std::move(value));
            }
        }

        HttpArguments& queryArguments() {
            return _queryArguments;
        }

        const HttpArguments & getQueryArguments() const {
            return _queryArguments;
        }

        HttpArguments & bodyArguments() {
            return _bodyArguments;
        }

        const HttpArguments & getBodyArguments() const {
            return _bodyArguments;
        }

        HttpFiles& files() {
            return _files;
        }

        void addFile(const std::string &name, HttpFile file) {
            _files[name].emplace_back(std::move(file));
        }

        void parseBody(StringVec& errors);

        void onDetached() {
            _connection.takeover();
        }
    private:
        std::string _method;
        std::string _uri;
        std::string _version;
        HttpHeadersPtr _headers;
        std::string _body;
        std::string _remoteIP;
        std::string _protocol;
        std::string _host;
        std::string _hostName;
        HttpFiles _files;
        Holder<HttpConnection> _connection;
        Time _startTime;
        Time _finishTime;
        std::string _path;
        std::string _query;

        HttpArguments _arguments;
        HttpArguments _queryArguments;
        HttpArguments _bodyArguments;
    };

    using HttpServerRequestPtr = std::shared_ptr<HttpServerRequest>;

    EASY_EVENT_API std::ostream& operator<<(std::ostream &os, const HttpServerRequest &request);
}

#endif //EASYEVENT_HTTP_HTTPSERVERREQUEST_H
