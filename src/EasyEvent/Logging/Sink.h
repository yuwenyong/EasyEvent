//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_SINK_H
#define EASYEVENT_LOGGING_SINK_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Logging/LogMessage.h"


namespace EasyEvent {

    class EASY_EVENT_API Sink {
    public:
        virtual ~Sink() = default;

        LogLevel getLevel() const {
            return _level;
        }

        bool isThreadSafe() const {
            return (_flags & SINK_FLAGS_MULTI_THREAD) != 0;
        }

        void write(LogMessage* message);

        static const char* getLevelString(LogLevel level);
    protected:
        Sink(LogLevel level, SinkFlags flags)
            : _level(level)
            , _flags(flags) {

        }

        virtual void write(LogMessage *message, const std::string& text) = 0;

        bool shouldLog(LogMessage* message) const {
            assert(message != nullptr);
            return _level <= message->getLevel();
        }

        LogLevel _level;
        SinkFlags _flags;
    };

}

#endif //EASYEVENT_LOGGING_SINK_H
