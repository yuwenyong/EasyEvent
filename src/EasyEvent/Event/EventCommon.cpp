//
// Created by yuwenyong on 2020/11/23.
//

#include "EasyEvent/Event/EventCommon.h"


const char * EasyEvent::SocketErrorCategory::name() const noexcept {
    return "socket error";
}

std::string EasyEvent::SocketErrorCategory::message(int ev) const {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    char* msg = 0;
    DWORD length = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS, 0, ev,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&msg, 0, 0);
    std::shared_ptr<void> localFree(nullptr, [msg](void*) {
        ::LocalFree(msg);
    });
    if (length && msg[length - 1] == '\n') {
        msg[--length] = '\0';
    }
    if (length && msg[length - 1] == '\r') {
        msg[--length] = '\0';
    }
    if (length) {
        return msg;
    }
    else {
        return "(unrecognized sokcet error)";
    }
#else
    char buf[256] = "";
    strerror_r(ev, buf, sizeof(buf));
    return buf;
#endif
}

const std::error_category & EasyEvent::getSocketErrorCategory() {
    static const EasyEvent::SocketErrorCategory errCategory{};
    return errCategory;
}