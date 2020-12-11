//
// Created by yuwenyong on 2020/12/11.
//

#ifndef EASYEVENT_COMMON_BUFFER_H
#define EASYEVENT_COMMON_BUFFER_H

#include "EasyEvent/Common/Config.h"


namespace EasyEvent {

    class EASY_EVENT_API Buffer {
    public:

    protected:
        uint8* _storage;
        size_t _capacity;
        size_t _wpos;
        size_t _rpos;
    };

}

#endif //EASYEVENT_COMMON_BUFFER_H
