//
// Created by yuwenyong on 2020/11/23.
//

#include "EasyEvent/Event/Event.h"


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
    std::shared_ptr<void> localFree(nullptr, [&msg](void*) {
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

const char * EasyEvent::EventErrorCategory::name() const noexcept {
    return "event error";
}

std::string EasyEvent::EventErrorCategory::message(int ev) const {
    switch (static_cast<EventErrors>(ev)) {
        case EventErrors::AlreadyStarted:
            return "already started";
        case EventErrors::AlreadyReading:
            return "already reading";
        default:
            return "(unrecognized error)";
    }
}

const std::error_category & EasyEvent::getSocketErrorCategory() {
    static const SocketErrorCategory errCategory{};
    return errCategory;
}

const std::error_category & EasyEvent::getEventErrorCategory() {
    static const EventErrorCategory errCategory{};
    return errCategory;
}