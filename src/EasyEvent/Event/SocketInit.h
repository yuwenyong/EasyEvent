//
// Created by yuwenyong on 2020/11/24.
//

#ifndef EASYEVENT_EVENT_SOCKETINIT_H
#define EASYEVENT_EVENT_SOCKETINIT_H

#include "EasyEvent/Event/EventCommon.h"


namespace EasyEvent {

    class EASY_EVENT_API SocketInit {
    public:
        SocketInit();

        ~SocketInit();

    protected:
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
        struct Data {
            long initCount;
            long result;
        };

        static void startup(Data& d, unsigned char major, unsigned char minor);

        static void cleapup(Data& d);

        static void throwOnError(Data& d);

        static Data gData;
#endif
    };

}

#endif //EASYEVENT_EVENT_SOCKETINIT_H
