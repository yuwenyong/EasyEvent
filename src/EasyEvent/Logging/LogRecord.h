//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGRECORD_H
#define EASYEVENT_LOGGING_LOGRECORD_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Common/Time.h"


namespace EasyEvent {


    class EASY_EVENT_API LogRecord {
    public:
        LogRecord(Logger* logger,
                  const char* fileName,
                  int lineno,
                  const char* funcName,
                  LogLevel level,
                  Time timestamp,
                  std::thread::id threadId,
                  std::string message)
            : LogRecord(logger,
                        fileName,
                        lineno,
                        funcName,
                        level,
                        timestamp,
                        threadId,
                        std::move(message),
                        nullptr) {
        }

        LogRecord(Logger* logger,
                  const char* fileName,
                  int lineno,
                  const char* funcName,
                  LogLevel level,
                  Time timestamp,
                  std::thread::id threadId,
                  std::string message,
                  Sink* sink)
                : _logger(logger)
                , _fileName(fileName)
                , _lineno(lineno)
                , _funcName(funcName)
                , _level(level)
                , _timestamp(timestamp)
                , _threadId(threadId)
                , _message(std::move(message))
                , _sink(sink) {
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

        const std::string& getMessage() const {
            return _message;
        }

        void writeAsync();

        std::unique_ptr<LogRecord> clone() const {
            return std::make_unique<LogRecord>(_logger, _fileName, _lineno, _funcName, _level, _timestamp, _threadId,
                                               _message, _sink);
        }

        std::unique_ptr<LogRecord> clone(Sink* sink) const {
            return std::make_unique<LogRecord>(_logger, _fileName, _lineno, _funcName, _level, _timestamp, _threadId,
                                               _message, sink);
        }
    protected:
        Logger* _logger;
        const char* _fileName;
        int _lineno;
        const char* _funcName;
        LogLevel _level;
        Time _timestamp;
        std::thread::id _threadId;
        std::string _message;
        Sink* _sink;
    };

}


#endif //EASYEVENT_LOGGING_LOGRECORD_H
