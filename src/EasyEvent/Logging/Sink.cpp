//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/Sink.h"
#include "EasyEvent/Logging/Logger.h"


void EasyEvent::Sink::write(LogMessage *message) {
    if (!shouldLog(message)) {
        return;
    }
    std::ostringstream ss;

    if (_flags & SINK_FLAGS_PREFIX_TIMESTAMP) {
        ss << '[' << message->getTimestamp().toDateTimeString() << ']';
    }

    if (_flags & SINK_FLAGS_PREFIX_LOG_LEVEL) {
        ss << '[' << getLevelString(message->getLevel()) << ']';
    }

    if (_flags & SINK_FLAGS_PREFIX_LOGGER_NAME) {
        ss << '[' << message->getLogger()->getName() << "] ";
    }

    ss << message->getText();
    std::string data = ss.str();
    if (!data.empty() && data.back() == '\n') {
        data.pop_back();
    }
    write(message, data);
}

const char * EasyEvent::Sink::getLevelString(LogLevel level) {
    switch (level)
    {
        case LOG_LEVEL_CRITICAL:
            return "crit";
        case LOG_LEVEL_ERROR:
            return "fail";
        case LOG_LEVEL_WARN:
            return "warn";
        case LOG_LEVEL_INFO:
            return "info";
        case LOG_LEVEL_DEBUG:
            return "dbug";
        default:
            return "disabled";
    }
}