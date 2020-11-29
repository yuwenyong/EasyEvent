//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGMESSAGE_H
#define EASYEVENT_LOGGING_LOGMESSAGE_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Common/Time.h"


namespace EasyEvent {


    class EASY_EVENT_API LogMessage {
    public:
        LogMessage(Logger* logger, LogLevel level, Time timestamp, std::string text)
            : _logger(logger)
            , _level(level)
            , _timestamp(timestamp)
            , _text(std::move(text)) {
        }

        Logger* getLogger() const {
            return _logger;
        }

        LogLevel getLevel() const {
            return _level;
        }

        Time getTimestamp() const {
            return _timestamp;
        }

        const std::string& getText() const {
            return _text;
        }

    protected:
        Logger* _logger;
        LogLevel _level;
        Time _timestamp;
        std::string _text;
    };

}


#endif //EASYEVENT_LOGGING_LOGMESSAGE_H
