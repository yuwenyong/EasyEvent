//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_SINK_H
#define EASYEVENT_LOGGING_SINK_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Logging/LogCommon.h"

namespace EasyEvent {

    class EASY_EVENT_API Sink {
    public:
        virtual ~Sink() = default;
        virtual void write(LogMessage* message) = 0;
    };

}

#endif //EASYEVENT_LOGGING_SINK_H
