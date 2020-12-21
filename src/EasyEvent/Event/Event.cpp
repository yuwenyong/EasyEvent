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
        return "(unrecognized socket error)";
    }
#else
    char buf[256] = "";
    return strerror_result(strerror_r(ev, buf, sizeof(buf)), buf);
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
        case EventErrors::UnsatisfiableRead:
            return "unsatisfiable read";
        case EventErrors::ReadCallbackFailed:
            return "read callback failed";
        case EventErrors::WriteCallbackFailed:
            return "write callback failed";
        case EventErrors::CloseCallbackFailed:
            return "close callback failed";
        case EventErrors::ConnectionClosed:
            return "connection closed";
        case EventErrors::ConnectionBufferFull:
            return "connection buffer full";
        case EventErrors::UnexpectedBehaviour:
            return "unexpected behaviour";
        case EventErrors::AlreadyListening:
            return "already listening";
        case EventErrors::CallbackNotFound:
            return "callback not found";
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