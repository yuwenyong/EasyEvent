//
// Created by yuwenyong on 2020/11/12.
//

#include "EasyEvent/Logging/Log.h"
#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/ConsoleSink.h"
#include "EasyEvent/Logging/FileSink.h"
#include "EasyEvent/Logging/RotatingFileSink.h"
#include "EasyEvent/Logging/TimedRotatingFileSink.h"

const char* EasyEvent::Log::RootLoggerName = "Root";

const std::string EasyEvent::Log::Sinks = "sinks";
const std::string EasyEvent::Log::Loggers = "loggers";

EasyEvent::Logger * EasyEvent::Log::getLogger(const std::string &name) {
    if (name == RootLoggerName) {
        return _rootLogger;
    } else {
        std::lock_guard<std::mutex> lock(_mutex);
        return getOrCreateLogger(name);
    }
}

bool EasyEvent::Log::registerFactory(const std::string &type, std::unique_ptr<SinkFactory> &&factory) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_factories.find(type) != _factories.end()) {
        return false;
    }
    _factories[type] = std::move(factory);
    return true;
}

void EasyEvent::Log::configure(const JsonValue &settings) {
    std::lock_guard<std::mutex> lock(_mutex);
    std::map<std::string, SinkPtr> sinks;
    configureSinks(settings, sinks);
    configureLoggers(settings, sinks);
}

EasyEvent::LogLevel EasyEvent::Log::getLevel() const {
    return _rootLogger->getLevel();
}

void EasyEvent::Log::setLevel(LogLevel level) {
    _rootLogger->setLevel(level);
}

bool EasyEvent::Log::disabled() const {
    return _rootLogger->disabled();
}

void EasyEvent::Log::disabled(bool disabled) {
    _rootLogger->disabled(disabled);
}

void EasyEvent::Log::setSink(const SinkPtr &sink) {
    _rootLogger->setSink(sink);
}

void EasyEvent::Log::appendSink(const SinkPtr &sink) {
    _rootLogger->appendSink(sink);
}

void EasyEvent::Log::removeSink(const SinkPtr &sink) {
    _rootLogger->removeSink(sink);
}

void EasyEvent::Log::resetSinks() {
    _rootLogger->resetSinks();
}

EasyEvent::Log::Log() {
    setupBuiltinFactories();
    initRootLogger();
}

void EasyEvent::Log::initRootLogger() {
    auto logger = std::make_unique<Logger>(this, RootLoggerName, LOG_LEVEL_DEFAULT);
    _rootLogger = logger.get();
    _loggers[RootLoggerName] = std::move(logger);
    _rootLogger->setSink(ConsoleSink::create(true));
}

void EasyEvent::Log::setupBuiltinFactories() {
    _factories[ConsoleSinkFactory::TypeName] = std::make_unique<ConsoleSinkFactory>();
    _factories[FileSinkFactory::TypeName] = std::make_unique<FileSinkFactory>();
    _factories[RotatingFileSinkFactory::TypeName] = std::make_unique<RotatingFileSinkFactory>();
    _factories[TimedRotatingFileSinkFactory::TypeName] = std::make_unique<TimedRotatingFileSinkFactory>();
}

EasyEvent::Logger * EasyEvent::Log::getOrCreateLogger(const std::string &name) {
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

void EasyEvent::Log::configureSinks(const JsonValue &settings, std::map<std::string, SinkPtr>& sinks) {
    const JsonValue *sinksConf = settings.find(Sinks);
    if (sinksConf) {
        if (!sinksConf->isArray()) {
            std::string errMsg = "`" + Sinks + "' must be an array";
            throwError(UserErrors::BadValue, "Log", errMsg);
        }
        auto numSinks = sinksConf->size();
        for (size_t i = 0; i < numSinks; ++i) {
            const JsonValue& sinkConf = sinksConf[i];
            std::string name = SinkFactory::parseName(sinkConf);
            if (sinks.find(name) != sinks.end()) {
                std::string errMsg = "Duplicate sink name `" + name + "' found";
                throwError(UserErrors::DuplicateValues, "Log", errMsg);
            }
            std::string type = SinkFactory::parseType(sinkConf);
            auto iter = _factories.find(type);
            if (iter == _factories.end()) {
                std::string errMsg = "Sink type `" + type + "' not found";
                throwError(UserErrors::NotFound, "Log", errMsg);
            }
            auto factory = iter->second.get();
            LogLevel level = SinkFactory::parseLevel(sinkConf);
            bool multiThread = SinkFactory::parseMultiThread(sinkConf);
            bool async = SinkFactory::parseAsync(sinkConf);
            std::string format = SinkFactory::parseFormat(sinkConf);
            sinks[name] = factory->create(sinkConf, level, multiThread, async, format);
        }
    }
}

void EasyEvent::Log::configureLoggers(const JsonValue &settings, const std::map<std::string, SinkPtr> &sinks) {
    const JsonValue *loggersConf = settings.find(Loggers);
    if (loggersConf) {
        if (!loggersConf->isArray()) {
            std::string errMsg = "`" + Loggers + "' must be an array";
            throwError(UserErrors::BadValue, "Log", errMsg);
        }
        auto numLoggers = loggersConf->size();
        for (size_t i = 0; i < numLoggers; ++i) {
            //const JsonValue& loggerConf = loggersConf[i];
        }
    }
}

