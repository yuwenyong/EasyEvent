//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGCOMMON_H
#define EASYEVENT_LOGGING_LOGCOMMON_H

namespace EasyEvent {

    enum LogLevel {
        LOG_LEVEL_DEBUG                              = 1,
        LOG_LEVEL_INFO                               = 2,
        LOG_LEVEL_WARN                               = 3,
        LOG_LEVEL_ERROR                              = 4,
        LOG_LEVEL_DISABLE                            = 5
    };

    enum LoggerFlags {
        LOGGER_FLAGS_NONE                           = 0x00,
        LOGGER_FLAGS_MULTI_THREAD                   = 0x01,
        LOGGER_FLAGS_ASYNC                          = 0x02,
        LOGGER_FLAGS_DEFAULT                        = LOGGER_FLAGS_NONE,
    };

    enum SinkFlags {
        SINK_FLAGS_NONE                             = 0x00,
        SINK_FLAGS_PREFIX_TIMESTAMP                 = 0x01,
        SINK_FLAGS_PREFIX_LOG_LEVEL                 = 0x02,
        SINK_FLAGS_PREFIX_LOGGER_NAME               = 0x04,
        SINK_FLAGS_MULTI_THREAD                     = 0x08,
        SINK_FLAGS_DEFAULT                          = SINK_FLAGS_PREFIX_TIMESTAMP | SINK_FLAGS_PREFIX_LOG_LEVEL
    };
}

#endif //EASYEVENT_LOGGING_LOGCOMMON_H
