//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGGER_H
#define EASYEVENT_LOGGING_LOGGER_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Configuration/Json.h"


namespace EasyEvent {

    class EASY_EVENT_API Logger {
    public:
        friend class Log;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        explicit Logger(Log* manager, const std::string& name, LogLevel level)
            : _manager(manager)
            , _name(name)
            , _level(level) {

        }

        std::string getName() const {
            return _name;
        }

        Logger* getParent() const {
            return _parent;
        }

        LogLevel getLevel() const {
            return _level;
        }

        void setLevel(LogLevel level) {
            _level = level;
        }

        bool propagate() const {
            return _propagate;
        }

        void propagate(bool propagate) {
            _propagate = propagate;
        }

        bool disabled() const {
            return _disabled;
        }

        void disabled(bool disabled) {
            _disabled = disabled;
        }

        void setSink(const SinkPtr& sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            _sinks.clear();
            if (sink) {
                _sinks.emplace_back(sink);
                _placeholder = false;
            }
        }

        void appendSink(const SinkPtr& sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            auto iter = std::find(_sinks.begin(), _sinks.end(), sink);
            if (iter == _sinks.end()) {
                _sinks.emplace_back(sink);
                _placeholder = false;
            }
        }

        void removeSink(const SinkPtr& sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            auto iter = std::remove(_sinks.begin(), _sinks.end(), sink);
            if (iter != _sinks.end()) {
                _sinks.erase(iter, _sinks.end());
            }
        }

        void resetSinks() {
            std::lock_guard<std::mutex> lock(_mutex);
            _sinks.clear();
        }

        bool shouldLog(const LogLevel level) const;

        void write(LogRecord* record);

        Logger* getChild(const std::string& suffix) const;
    protected:
        void setParent(Logger* parent) {
            _parent = parent;
        }

        bool placeholder() const {
            return _placeholder;
        }

        void placeholder(bool placeholder) {
            _placeholder = placeholder;
        }

        void doWrite(LogRecord* record);

        static bool shouldLog(const Logger* logger, LogLevel level);

        std::mutex _mutex;
        Log* _manager;
        std::string _name;
        LogLevel _level;
        Logger* _parent{nullptr};
        bool _placeholder{true};
        bool _propagate{true};
        bool _disabled{false};
        std::vector<SinkPtr> _sinks;
    };


    class EASY_EVENT_API LoggerFactory {
    public:
        static std::unique_ptr<Logger> create(Log* manager, const std::string& name, LogLevel level) {
            return std::make_unique<Logger>(manager, name, level);
        }

        static std::string parseName(const JsonValue& settings);

        static std::optional<LogLevel> parseLevel(const JsonValue& settings);

        static std::optional<bool> parsePropagate(const JsonValue& settings);

        static std::optional<bool> parseDisabled(const JsonValue& settings);

        static std::optional<std::vector<std::string>> parseSinks(const JsonValue& settings);

        static const std::string Name;
        static const std::string Level;
        static const std::string Propagate;
        static const std::string Disabled;
        static const std::string Sinks;
    };

}


#endif //EASYEVENT_LOGGING_LOGGER_H
