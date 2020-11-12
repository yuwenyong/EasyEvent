//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGSTREAM_H
#define EASYEVENT_LOGGING_LOGSTREAM_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Time.h"
#include "EasyEvent/Logging/LogCommon.h"


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

        std::ostringstream& stream() {
            return _os;
        }

        bool shouldLog() const;

        ~LogStream();
    protected:
        Logger* _logger;
        LogLevel _level;
        Time _timestamp;
        std::ostringstream _os;
    };

    template <typename ValueT>
    LogStream& operator<<(LogStream& stream, const ValueT &val) {
        if (stream.shouldLog()) {
            stream.stream() << val;
        }
        return stream;
    }

}

#endif //EASYEVENT_LOGGING_LOGSTREAM_H
