//
// Created by yuwenyong on 2020/11/12.
//

#ifndef EASYEVENT_LOGGING_LOG_H
#define EASYEVENT_LOGGING_LOG_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Configuration/Json.h"


namespace EasyEvent {

    class EASY_EVENT_API Log {
    public:
        Log(const Log&) = delete;
        Log& operator=(const Log&) = delete;

        Logger* getRootLogger() const {
            return _rootLogger;
        }

        Logger* getLogger(const std::string& name);

        bool registerFactory(const std::string& type, std::unique_ptr<SinkFactory>&& factory);

        void configure(const JsonValue& settings);

        static Log& instance() {
            static Log log;
            return log;
        }

        static const char* RootLoggerName;
    protected:
        Log();

        void initRootLogger();

        void setupBuiltinFactories();

        Logger* getOrCreateLogger(const std::string& name);

        Logger* createLogger(const std::string& name);

        mutable std::mutex _mutex;
        std::map<std::string, std::unique_ptr<Logger>> _loggers;
        std::map<std::string, std::unique_ptr<SinkFactory>> _factories;
        Logger* _rootLogger{nullptr};
    };

}

#endif //EASYEVENT_LOGGING_LOG_H
