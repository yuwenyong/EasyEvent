//
// Created by yuwenyong on 2020/11/12.
//

#include "EasyEvent/Logging/Log.h"
#include "EasyEvent/Common/Errors.h"
#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/LogMessage.h"


EasyEvent::Log::~Log() {
    if (_thread) {
        _thread->stop();
        _thread->wait();
    }
}

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
        ec = {0, ec.category()};
        return iter->second.get();
    } else {
        ec = make_error_code(LoggingErrors::AlreadyRegistered);
        return nullptr;
    }
}

EasyEvent::Logger * EasyEvent::Log::createLogger(const std::string &name, LogLevel level, LoggerFlags flags) {
    std::error_code ec;
    auto logger = createLogger(name, level, flags, ec);
    throwError(ec);
    return logger;
}

void EasyEvent::Log::write(std::unique_ptr<LogMessage> &&message) {
    Logger* logger = message->getLogger();
    assert(logger != nullptr);
    assert(getLogger(logger->getName()) == logger);
    if (logger->isAsync()) {
        if (!_thread) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_thread) {
                _thread = std::make_unique<TaskPool>();
                _thread->start(1);
            }
        }
        _thread->post([logger, message=std::move(message)]() mutable {
            logger->write(std::move(message));
        });
    } else {
        logger->write(std::move(message));
    }
}