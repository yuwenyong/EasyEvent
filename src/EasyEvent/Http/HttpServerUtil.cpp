//
// Created by yuwenyong.vincent on 2021/9/5.
//
#include "EasyEvent/Http/HttpServerUtil.h"
#include "EasyEvent/Common/StrUtil.h"
#include "EasyEvent/Event/SslConnection.h"


EasyEvent::Logger * EasyEvent::HttpServerConnectionDelegate::getLogger() {
    return nullptr;
}

const EasyEvent::HttpConnectionOptions EasyEvent::HttpConnectionOptions::Default;

EasyEvent::HttpRequestContext::HttpRequestContext(const ConnectionPtr& stream, const Address* address,
                                                  const std::string& protocol, StringSet trustedDownstream)
        : _trustedDownstream(std::move(trustedDownstream)) {

    if (address != nullptr) {
        _remoteIP = address->getAddrString();
    } else {
        _remoteIP = "0.0.0.0";
    }
    if (!protocol.empty()) {
        _protocol = protocol;
    } else if (std::dynamic_pointer_cast<SslConnection>(stream)) {
        _protocol = HttpProtocolNames::Https;
    } else {
        _protocol = HttpProtocolNames::Http;
    }
    _origRemoteIP = _remoteIP;
    _origProtocol = _protocol;
}

void EasyEvent::HttpRequestContext::applyXheaders(const HttpHeaders &headers) {
    std::string ip = headers.get(HttpHeaderFields::XForwardedFor, _remoteIP);
    StringVec ips = StrUtil::split(ip, ',');

    for (auto iter = ips.rbegin(); iter != ips.rend(); ++iter) {
        StrUtil::trim(*iter);
        if (_trustedDownstream.find(*iter) == _trustedDownstream.end()) {
            ip = *iter;
            break;
        }
    }
    ip = headers.get(HttpHeaderFields::XRealIp, ip);
    if (Address::isAddress(ip.c_str())) {
        _remoteIP = ip;
    }

    std::string protoHeader = headers.get(HttpHeaderFields::XScheme, headers.get(HttpHeaderFields::XForwardedProto));
    if (!protoHeader.empty()) {
        StringVec protos = StrUtil::split(protoHeader, ',');
        protoHeader = StrUtil::trimCopy(protos.back());
        if (protoHeader == HttpProtocolNames::Http || protoHeader == HttpProtocolNames::Https) {
            _protocol = std::move(protoHeader);
        }
    }

}

std::ostream& EasyEvent::operator<<(std::ostream &os, const HttpRequestContext& context) {
    os << context.getRemoteIP();
    return os;
}