//
// Created by yuwenyong on 2020/11/29.
//

#ifndef EASYEVENT_EVENT_SOCKETOPS_H
#define EASYEVENT_EVENT_SOCKETOPS_H

#include "EasyEvent/Event/EvtCommon.h"


namespace EasyEvent {

    class EASY_EVENT_API SocketOps {
    public:
        static const char* InetNtop(int af, const void* src, char* dest, size_t length, unsigned long scopeId,
                                    std::error_code& ec);

        static const char* InetNtop(int af, const void* src, char* dest, size_t length, unsigned long scopeId) {
            std::error_code ec;
            auto result = InetNtop(af, src, dest, length, scopeId, ec);
            throwError(ec);
            return result;
        }

        static int InetPton(int af, const char* src, void* dest, unsigned long* scopeId, std::error_code& ec);

        static int InetPton(int af, const char* src, void* dest, unsigned long* scopeId) {
            std::error_code ec;
            auto result = InetPton(af, src, dest, scopeId, ec);
            throwError(ec);
            return result;
        }
    };

}

#endif //EASYEVENT_EVENT_SOCKETOPS_H
