//
// Created by yuwenyong.vincent on 2021/9/5.
//

#ifndef EASYEVENT_HTTPUTIL_HTTPUTIL_H
#define EASYEVENT_HTTPUTIL_HTTPUTIL_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Error.h"

namespace EasyEvent {

    enum class HttpProtocol {
        Http,
        Https,
        WS,
        WSS,
    };

    class EASY_EVENT_API HttpProtocolNames {
    public:
        static const char* Http;
        static const char* Https;
        static const char* WS;
        static const char* WSS;
    };

}

#endif //EASYEVENT_HTTPUTIL_HTTPUTIL_H
