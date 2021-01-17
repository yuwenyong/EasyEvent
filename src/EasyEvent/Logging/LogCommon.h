//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGCOMMON_H
#define EASYEVENT_LOGGING_LOGCOMMON_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Errors.h"

namespace EasyEvent {

    enum LogLevel {
        LOG_LEVEL_DEBUG                              = 1,
        LOG_LEVEL_INFO                               = 2,
        LOG_LEVEL_WARN                               = 3,
        LOG_LEVEL_ERROR                              = 4,
        LOG_LEVEL_CRITICAL                           = 5,
        LOG_LEVEL_DISABLE                            = 6,
        LOG_LEVEL_DEFAULT                            = LOG_LEVEL_INFO,
        NUM_ENABLED_LOG_LEVELS                       = 5,
    };

    class Log;
    class LogRecord;
    class Logger;
    class Sink;
    class SinkFactory;

    using SinkPtr = std::shared_ptr<Sink>;
}

#endif //EASYEVENT_LOGGING_LOGCOMMON_H
