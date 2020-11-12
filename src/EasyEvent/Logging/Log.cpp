//
// Created by yuwenyong on 2020/11/12.
//

#include "EasyEvent/Logging/Log.h"
#include "EasyEvent/Common/Errors.h"
#include "EasyEvent/Common/TaskPool.h"
#include "EasyEvent/Logging/Logger.h"


EasyEvent::Logger * EasyEvent::Log::getOrCreateLogger(const std::string &name, LogLevel level, LoggerFlags flags) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto iter = _loggers.find(name);
    if (iter == _loggers.end()) {
        _loggers[name] = std::make_unique<Logger>(name, level, flags);
        iter = _loggers.find(name);
        assert(iter != _loggers.end());
    }
    return iter->second.get();
}

EasyEvent::Logger * EasyEvent::Log::createLogger(const std::string &name,LogLevel level, LoggerFlags flags,
                                                 std::error_code &ec) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto iter = _loggers.find(name);
    if (iter == _loggers.end()) {
        _loggers[name] = std::make_unique<Logger>(name, level, flags);
        iter = _loggers.find(name);
        assert(iter != _loggers.end());
        return iter->second.get();
    } else {
        ec = make_error_code(CommonErrc::AlreadyRegistered);
        return nullptr;
    }
}