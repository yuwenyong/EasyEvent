//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGGER_H
#define EASYEVENT_LOGGING_LOGGER_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Logging/LogCommon.h"


namespace EasyEvent {

    class EASY_EVENT_API Logger {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        explicit Logger(std::string name, LogLevel level, LoggerFlags flags)
            : _name(std::move(name))
            , _level(level)
            , _flags(flags) {
            if ((_flags & LOGGER_FLAGS_MULTI_THREAD) != 0) {
                _mutex = std::make_unique<std::mutex>();
            }
        }

        const std::string& getName() const {
            return _name;
        }

        LogLevel getLevel() const {
            return _level;
        }

        void write(std::unique_ptr<LogMessage> &&message) {

        }
    protected:
        void write(LogMessage *message);

        std::string _name;
        LogLevel _level;
        LoggerFlags _flags;
        std::unique_ptr<std::mutex> _mutex;
        std::vector<
    };

}


#endif //EASYEVENT_LOGGING_LOGGER_H
