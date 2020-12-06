//
// Created by yuwenyong on 2020/12/6.
//

#ifndef EASYEVENT_COMMON_NONCOPYABLE_H
#define EASYEVENT_COMMON_NONCOPYABLE_H

#include "EasyEvent/Common/Config.h"

namespace EasyEvent {
    class NonCopyable {
    public:
        NonCopyable() {}
        ~NonCopyable() {}
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };
}

#endif //EASYEVENT_COMMON_NONCOPYABLE_H
