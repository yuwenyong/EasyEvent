//
// Created by yuwenyong on 2021/1/23.
//

#ifndef EASYEVENT_SHARED_LOGGERS_H
#define EASYEVENT_SHARED_LOGGERS_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Logging/Logger.h"

namespace EasyEvent {

    extern const std::string SysLoggerName;
    extern const std::string AppLoggerName;

    EASY_EVENT_API Logger* SysLogger();

    EASY_EVENT_API Logger* AppLogger();

}

#endif //EASYEVENT_SHARED_LOGGERS_H
