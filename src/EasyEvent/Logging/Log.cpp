//
// Created by yuwenyong on 2020/11/12.
//

#include "EasyEvent/Logging/Log.h"
#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/ConsoleSink.h"


const char* EasyEvent::Log::RootLoggerName = "Root";

EasyEvent::Logger * EasyEvent::Log::getLogger(const std::string &name) {
    if (name == RootLoggerName) {
        return _rootLogger;
    }
    return getOrCreateLogger(name);
}

EasyEvent::Log::Log() {
    initRootLogger();
}

void EasyEvent::Log::initRootLogger() {
    auto logger = std::make_unique<Logger>(this, RootLoggerName, LOG_LEVEL_DEFAULT);
    _rootLogger = logger.get();
    _loggers[RootLoggerName] = std::move(logger);
    _rootLogger->setSink(ConsoleSink::create(true));
}

EasyEvent::Logger * EasyEvent::Log::getOrCreateLogger(const std::string &name) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto iter = _loggers.find(name);
    Logger* logger;
    if (iter == _loggers.end()) {
        logger = createLogger(name);
    } else {
        logger = iter->second.get();
    }
    return logger;
}

EasyEvent::Logger * EasyEvent::Log::createLogger(const std::string &name) {
    auto logger = std::make_unique<Logger>(this, name, LOG_LEVEL_DEFAULT);
    Logger* result = logger.get(), *current = logger.get();

    _loggers[name] = std::move(logger);

    std::string::size_type pos = name.rfind('.');
    while (pos != std::string::npos) {
        std::string parentName = name.substr(0, pos);
        if (parentName.empty()) {
            break;
        }
        auto iter = _loggers.find(parentName);
        if (iter != _loggers.end()) {
            current->setParent(iter->second.get());
            current = nullptr;
            break;
        }
        auto parentLogger = std::make_unique<Logger>(this, parentName, LOG_LEVEL_DEFAULT);
        current->setParent(parentLogger.get());
        current = parentLogger.get();
        _loggers[parentName] = std::move(parentLogger);
        pos = name.rfind('.', pos - 1);
    }
    if (current) {
        current->setParent(_rootLogger);
    }
    return result;
}


