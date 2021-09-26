//
// Created by yuwenyong.vincent on 2021/8/15.
//

#include "EasyEvent/Http/HttpServerRequest.h"
#include "EasyEvent/Http/HttpConnection.h"
#include "EasyEvent/Common/StrUtil.h"


EasyEvent::HttpServerRequest::HttpServerRequest(
        const std::shared_ptr<HttpConnection>& connection,
        const RequestStartLine* startLine,
        const HttpHeadersPtr& headers,
        const std::string& method,
        const std::string& uri,
        const std::string& version,
        std::string body,
        std::string host,
        HttpFiles files)
        : _method(startLine ? startLine->getMethod() : method)
        , _uri(startLine ? startLine->getPath() : uri)
        , _version(startLine ? startLine->getVersion() : version)
        , _headers(headers ? headers : std::make_shared<HttpHeaders>())
        , _body(std::move(body))
        , _remoteIP(connection->getContext().getRemoteIP())
        , _protocol(connection->getContext().getProtocol())
        , _files(std::move(files))
        , _connection(connection)
        , _startTime(Time::now()) {

    if (!host.empty()) {
        _host = std::move(host);
    } else {
        _host = _headers->get(HttpHeaderFields::Host, "127.0.0.1");
    }
    std::tie(_hostName, std::ignore) = HttpParse::splitHostAndPort(StrUtil::toLowerCopy(_host));
    std::tie(_path, std::ignore, _query) = StrUtil::partition(_uri, "?");
    _arguments = UrlParse::parseQS(_query, true);
    _queryArguments = _arguments;
}

void EasyEvent::HttpServerRequest::parseBody(StringVec& errors) {
    HttpParse::parseBodyArguments(_headers->get(HttpHeaderFields::ContentType, ""),
                                  _body, _bodyArguments, _files,errors, _headers.get());
}


std::ostream& EasyEvent::operator<<(std::ostream &os, const HttpServerRequest &request) {
    StringVec argsList;
    argsList.emplace_back("protocol=" + request.getProtocol());
    argsList.emplace_back("host=" + request.getHost());
    argsList.emplace_back("method=" + request.getMethod());
    argsList.emplace_back("uri=" + request.getURI());
    argsList.emplace_back("version=" + request.getVersion());
    argsList.emplace_back("remoteIp=" + request.getRemoteIP());
//    argsList.emplace_back("body=" + request.getBody());
    std::string args = StrUtil::join(argsList, ",");
    os << "HttpServerRequest(" + args + ")";
    return os;
}