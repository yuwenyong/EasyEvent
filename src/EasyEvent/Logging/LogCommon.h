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

    class LogMessage;
    class Logger;
    class Sink;

    using SinkPtr = std::shared_ptr<Sink>;

    enum class LoggingErrors {
        AlreadyRegistered = 1,
    };

    class EASY_EVENT_API LoggingErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getLoggingErrorCategory();

    inline std::error_code make_error_code(LoggingErrors err) {
        return {static_cast<int>(err), getLoggingErrorCategory()};
    }
}

namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::LoggingErrors>: public true_type {};

}

#endif //EASYEVENT_LOGGING_LOGCOMMON_H
