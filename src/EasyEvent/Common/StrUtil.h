//
// Created by yuwenyong on 2020/12/13.
//

#ifndef EASYEVENT_COMMON_STRUTIL_H
#define EASYEVENT_COMMON_STRUTIL_H

#include "EasyEvent/Common/Config.h"


namespace EasyEvent {

    class EASY_EVENT_API StrUtil {
    public:
        static const char* find(const char* s1, size_t len1, const char* s2, size_t len2);

        static const char* find(const char* s1, const char* s2, size_t len2) {
            return find(s1, strlen(s1), s2, len2);
        }

        static const char* find(const char* s1, size_t len1, const char* s2) {
            return find(s1, len1, s2, strlen(s2));
        }

        static const char* find(const char* s1, const char* s2) {
            return find(s1, strlen(s1), s2, strlen(s2));
        }
    };

}

#endif //EASYEVENT_COMMON_STRUTIL_H
