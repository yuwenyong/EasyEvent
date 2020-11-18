//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGSTREAM_H
#define EASYEVENT_LOGGING_LOGSTREAM_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Time.h"
#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Logging/Logger.h"


namespace EasyEvent {


    class EASY_EVENT_API LogStream {
    public:
        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;

        LogStream(Logger* logger, LogLevel level)
                : _logger(logger)
                , _level(level) {
            _timestamp = Time::now();
        }

        template <typename ValueT>
        LogStream& operator<<(ValueT &&val) {
            if (shouldLog()) {
                stream() << std::forward<ValueT>(val);
            }
            return *this;
        }

        ~LogStream();
    protected:
        std::ostringstream& stream() {
            return _os;
        }

        bool shouldLog() const {
            return _logger != nullptr && _level >= _logger->getLevel();
        }

        Logger* _logger;
        LogLevel _level;
        Time _timestamp;
        std::ostringstream _os;
    };

}

#define LOG_DEBUG(logger)       EasyEvent::LogStream(logger, EasyEvent::LOG_LEVEL_DEBUG)
#define LOG_INFO(logger)        EasyEvent::LogStream(logger, EasyEvent::LOG_LEVEL_INFO)
#define LOG_WARN(logger)        EasyEvent::LogStream(logger, EasyEvent::LOG_LEVEL_WARN)
#define LOG_ERROR(logger)       EasyEvent::LogStream(logger, EasyEvent::LOG_LEVEL_ERROR)
#define LOG_CRITICAL(logger)    EasyEvent::LogStream(logger, EasyEvent::LOG_LEVEL_CRITICAL)

#endif //EASYEVENT_LOGGING_LOGSTREAM_H
