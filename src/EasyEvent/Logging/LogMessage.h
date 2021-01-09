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
        LogMessage(Logger* logger,
                   const char* fileName,
                   int lineno,
                   const char* funcName,
                   LogLevel level,
                   Time timestamp,
                   std::thread::id threadId,
                   std::string text)
            : LogMessage(logger,
                         fileName,
                         lineno,
                         funcName,
                         level,
                         timestamp,
                         threadId,
                         std::move(text),
                         nullptr) {
        }

        LogMessage(Logger* logger,
                   const char* fileName,
                   int lineno,
                   const char* funcName,
                   LogLevel level,
                   Time timestamp,
                   std::thread::id threadId,
                   std::string text,
                   SinkPtr sink)
                : _logger(logger)
                , _fileName(fileName)
                , _lineno(lineno)
                , _funcName(funcName)
                , _level(level)
                , _timestamp(timestamp)
                , _threadId(threadId)
                , _text(std::move(text))
                , _sink(std::move(sink)) {
        }

        Logger* getLogger() const {
            return _logger;
        }

        const char* getFileName() const {
            return _fileName;
        }

        int getLineno() const {
            return _lineno;
        }

        const char* getFuncName() const {
            return _funcName;
        }

        LogLevel getLevel() const {
            return _level;
        }

        Time getTimestamp() const {
            return _timestamp;
        }

        std::thread::id getThreadId() const {
            return _threadId;
        }

        const std::string& getText() const {
            return _text;
        }

        std::unique_ptr<LogMessage> clone() const {
            return std::make_unique<LogMessage>(_logger, _fileName, _lineno, _funcName, _level, _timestamp, _threadId,
                                                _text, _sink);
        }

        std::unique_ptr<LogMessage> clone(SinkPtr sink) const {
            return std::make_unique<LogMessage>(_logger, _fileName, _lineno, _funcName, _level, _timestamp, _threadId,
                                                _text, std::move(sink));
        }
    protected:
        Logger* _logger;
        const char* _fileName;
        int _lineno;
        const char* _funcName;
        LogLevel _level;
        Time _timestamp;
        std::thread::id _threadId;
        std::string _text;
        SinkPtr _sink;
    };

}


#endif //EASYEVENT_LOGGING_LOGMESSAGE_H
