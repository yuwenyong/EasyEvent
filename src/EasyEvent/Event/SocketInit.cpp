//
// Created by yuwenyong on 2020/11/24.
//

#include "EasyEvent/Event/SocketInit.h"


EasyEvent::SocketInit::SocketInit() {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    startup(gData, 2, 0);
    throwOnError(gData);
#else
    std::signal(SIGPIPE, SIG_IGN);
#endif
}

EasyEvent::SocketInit::~SocketInit() {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    cleapup(gData);
#endif
}

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS

EasyEvent::SocketInit::Data EasyEvent::SocketInit::gData = {};

void EasyEvent::SocketInit::startup(Data &d, unsigned char major, unsigned char minor) {
    if (::InterlockedIncrement(&d.initCount) == 1) {
        WSADATA wsaData;
        long result = ::WSAStartup(MAKEWORD(major, minor), &wsaData);
        ::InterlockedExchange(&d.result, result);
    }
}

void EasyEvent::SocketInit::cleapup(Data &d) {
    if (::InterlockedDecrement(&d.initCount) == 0) {
        ::WSACleanup();
    }
}

void EasyEvent::SocketInit::throwOnError(Data &d) {
    long result = ::InterlockedExchangeAdd(&d.result, 0);
    if (result != 0) {
        std::error_code ec(result, getSocketErrorCategory());
        EasyEvent::throwError(ec, "winsock");
    }
}

#endif
