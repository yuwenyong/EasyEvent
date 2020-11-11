//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGMESSAGE_H
#define EASYEVENT_LOGGING_LOGMESSAGE_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Time.h"
#include "EasyEvent/Logging/LogCommon.h"


namespace EasyEvent {


    class EASY_EVENT_API LogMessage {
    public:
        LogMessage(LogLevel level, Time timestamp, std::string text)
            : _level(level)
            , _timestamp(timestamp)
            , _text(std::move(text)) {
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
        LogLevel _level;
        Time _timestamp;
        std::string _text;
    };

}


#endif //EASYEVENT_LOGGING_LOGMESSAGE_H
