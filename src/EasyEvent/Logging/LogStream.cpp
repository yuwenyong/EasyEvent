//
// Created by yuwenyong on 2020/11/7.
//

#include "EasyEvent/Logging/LogStream.h"
#include "EasyEvent/Logging/LogRecord.h"
#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/Log.h"


EasyEvent::LogStream::LogStream(Logger *logger,
                                const char *fileName,
                                int lineno,
                                const char *funcName,
                                LogLevel level)
        : _logger(logger)
        , _fileName(fileName)
        , _lineno(lineno)
        , _funcName(funcName)
        , _level(level) {
    _shouldLog = _logger != nullptr && _logger->shouldLog(_level);
    _timestamp = Time::now();
}

EasyEvent::LogStream::~LogStream() {
    if (_shouldLog) {
        auto message = _os.str();
        if (!message.empty()) {
            LogRecord record(_logger, _fileName, _lineno, _funcName, _level, _timestamp,
                             std::this_thread::get_id(), std::move(message));
            _logger->write(&record);
        }
    }
}
