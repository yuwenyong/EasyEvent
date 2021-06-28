//
// Created by yuwenyong.vincent on 2021/6/6.
//

#ifndef EASYEVENT_SSL_SSLINIT_H
#define EASYEVENT_SSL_SSLINIT_H

#include "EasyEvent/Ssl/Types.h"


namespace EasyEvent {

    class EASY_EVENT_API SslInit: private NonCopyable {
    public:

        SslInit(): _ref(instance()) {
            SslInit* tmp = &gInstance;
            memmove(&tmp, &tmp, sizeof(SslInit*));
        }

#if !defined(SSL_OP_NO_COMPRESSION) && (OPENSSL_VERSION_NUMBER >= 0x00908000L)
        static STACK_OF(SSL_COMP)* getNullCompressionMethods();
#endif
    protected:
        class DoInit;

        static std::shared_ptr<DoInit> instance();

        std::shared_ptr<DoInit> _ref;

        static SslInit gInstance;
    };

}

#endif //EASYEVENT_SSL_SSLINIT_H
