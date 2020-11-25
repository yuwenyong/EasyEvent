//
// Created by yuwenyong on 2020/11/12.
//

#ifndef EASYEVENT_LOGGING_LOG_H
#define EASYEVENT_LOGGING_LOG_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Common/TaskPool.h"


namespace EasyEvent {

    class EASY_EVENT_API Log {
    public:
        Log(const Log&) = delete;
        Log& operator=(const Log&) = delete;

        ~Log();

        Logger* getLogger(const std::string& name) const {
            std::lock_guard<std::mutex> lock(_mutex);
            auto iter = _loggers.find(name);
            return iter != _loggers.end() ? iter->second.get() : nullptr;
        }

        Logger* getOrCreateLogger(const std::string& name,
                                  LogLevel level=LOG_LEVEL_DEFAULT,
                                  LoggerFlags flags=LOGGER_FLAGS_DEFAULT);

        Logger* createLogger(const std::string& name, LogLevel level, LoggerFlags flags, std::error_code &ec);

        Logger* createLogger(const std::string& name,
                             LogLevel level=LOG_LEVEL_DEFAULT,
                             LoggerFlags flags=LOGGER_FLAGS_DEFAULT);

        void write(std::unique_ptr<LogMessage> &&message);

        void stop() {
            if (_thread) {
                _thread->stop();
                _thread->wait();
            }
        }

        static Log& instance() {
            static Log log;
            return log;
        }
    protected:
        Log() = default;

        std::map<std::string, std::unique_ptr<Logger>> _loggers;
        mutable std::mutex _mutex;
        std::unique_ptr<TaskPool> _thread;
    };

}

#endif //EASYEVENT_LOGGING_LOG_H
