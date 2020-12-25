//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGGER_H
#define EASYEVENT_LOGGING_LOGGER_H

#include "EasyEvent/Logging/LogCommon.h"


namespace EasyEvent {

    class EASY_EVENT_API Logger {
    public:
        friend class Log;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        explicit Logger(const std::string& name, LogLevel level, LoggerFlags flags)
            : _name(name)
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

        bool isThreadSafe() const {
            return (_flags & LOGGER_FLAGS_MULTI_THREAD) != 0;
        }

        bool isAsync() const {
            return (_flags & LOGGER_FLAGS_ASYNC) != 0;
        }

        void addSink(SinkPtr sink) {
            if (isThreadSafe()) {
                std::lock_guard<std::mutex> lock(*_mutex);
                _addSink(std::move(sink));
            } else {
                _addSink(std::move(sink));
            }
        }

        void delSink(SinkPtr sink) {
            if (isThreadSafe()) {
                std::lock_guard<std::mutex> lock(*_mutex);
                _delSink(std::move(sink));
            } else {
                _delSink(std::move(sink));
            }
        }
    protected:
        void write(std::unique_ptr<LogMessage> &&message) {
            if (isThreadSafe()) {
                std::lock_guard<std::mutex> lock(*_mutex);
                _write(message.get());
            } else {
                _write(message.get());
            }
        }

        void _write(LogMessage *message);

        void _addSink(SinkPtr sink) {
            _sinks.emplace_back(std::move(sink));
        }

        void _delSink(SinkPtr sink) {
            auto iter = std::remove(_sinks.begin(), _sinks.end(), sink);
            if (iter != _sinks.end()) {
                _sinks.erase(iter, _sinks.end());
            }
        }

        std::string _name;
        LogLevel _level;
        LoggerFlags _flags;
        std::unique_ptr<std::mutex> _mutex;
        std::vector<SinkPtr> _sinks;
    };

}


#endif //EASYEVENT_LOGGING_LOGGER_H
