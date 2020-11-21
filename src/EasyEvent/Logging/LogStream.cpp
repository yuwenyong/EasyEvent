//
// Created by yuwenyong on 2020/11/7.
//

#include "EasyEvent/Logging/LogStream.h"
#include "EasyEvent/Logging/LogMessage.h"
#include "EasyEvent/Logging/Log.h"


EasyEvent::LogStream::~LogStream() {
    if (shouldLog()) {
        auto text = _os.str();
        if (!text.empty()) {
            auto message = std::make_unique<LogMessage>(_logger, _level, _timestamp, std::move(text));
            Log::instance().write(std::move(message));
        }
    }
}
