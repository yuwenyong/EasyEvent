//
// Created by yuwenyong on 2020/11/7.
//

#include "EasyEvent/Logging/LogStream.h"
#include "EasyEvent/Logging/LogMessage.h"
#include "EasyEvent/Logging/Log.h"
#include "EasyEvent/Logging/Logger.h"


EasyEvent::LogStream::~LogStream() {
    if (shouldLog()) {
        auto message = std::make_unique<LogMessage>(_logger, _level, _timestamp, _os.str());
        Log::instance().write(std::move(message));
    }
}

bool EasyEvent::LogStream::shouldLog() const {
    return _logger != nullptr && _level >= _logger->getLevel();
}