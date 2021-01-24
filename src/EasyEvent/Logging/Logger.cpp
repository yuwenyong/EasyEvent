//
// Created by yuwenyong on 2020/11/7.
//

#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/Log.h"
#include "EasyEvent/Logging/Sink.h"


bool EasyEvent::Logger::shouldLog(LogLevel level) const {
    const Logger* current = this;
    while (current != nullptr) {
        if (shouldLog(current, level)) {
            return true;
        }
        current = current->propagate() ? current->getParent() : nullptr;
    }
    return false;
}

void EasyEvent::Logger::write(LogRecord *record) {
    if (shouldLog(this, record->getLevel())) {
        doWrite(record);
    }

    if (_propagate && _parent != nullptr) {
        _parent->write(record);
    }
}

void EasyEvent::Logger::doWrite(LogRecord *record) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto &sink: _sinks) {
        sink->write(record);
    }
}

EasyEvent::Logger * EasyEvent::Logger::getChild(const std::string &suffix) const {
    Assert(_manager != nullptr);
    std::string name = _name + '.' + suffix;
    return _manager->getLogger(name);
}

bool EasyEvent::Logger::shouldLog(const Logger *logger, LogLevel level) {
    if (logger->placeholder()) {
        return false;
    }
    if (logger->disabled()) {
        return false;
    }
    if (logger->getLevel() > level) {
        return false;
    }
    return true;
}


const std::string EasyEvent::LoggerFactory::Name = "name";
const std::string EasyEvent::LoggerFactory::Level = "level";
const std::string EasyEvent::LoggerFactory::Propagate = "propagate";
const std::string EasyEvent::LoggerFactory::Disabled = "disabled";
const std::string EasyEvent::LoggerFactory::Sinks = "sinks";


std::string EasyEvent::LoggerFactory::parseName(const JsonValue &settings) {
    const JsonValue* value = settings.find(Name);
    if (!value) {
        std::string errMsg = "Argument `" + Name + "' is required";
        throwError(UserErrors::ArgumentRequired, "LoggerFactory", errMsg);
    }
    std::string name = value->asString();
    if (name.empty()) {
        std::string errMsg = Name + " can't be empty";
        throwError(UserErrors::BadValue, "LoggerFactory", errMsg);
    }
    return name;
}

std::optional<EasyEvent::LogLevel> EasyEvent::LoggerFactory::parseLevel(const JsonValue &settings) {
    std::optional<LogLevel> result;
    std::string level;
    const JsonValue* value = settings.find(Level);
    if (value) {
        level = value->asString();
        if (stricmp(level.c_str(), "critical") == 0) {
            result = LOG_LEVEL_CRITICAL;
        } else if (stricmp(level.c_str(), "error") == 0) {
            result = LOG_LEVEL_ERROR;
        } else if (stricmp(level.c_str(), "warn") == 0) {
            result = LOG_LEVEL_WARN;
        } else if (stricmp(level.c_str(), "info") == 0) {
            result = LOG_LEVEL_INFO;
        } else if (stricmp(level.c_str(), "debug") == 0) {
            result = LOG_LEVEL_DEBUG;
        } else if (stricmp(level.c_str(), "disable") == 0) {
            result = LOG_LEVEL_DISABLE;
        } else {
            std::string errMsg = "Invalid value `" + level + "' for " + Level;
            throwError(UserErrors::BadValue, "LoggerFactory", errMsg);
        }
    }
    return result;
}

std::optional<bool> EasyEvent::LoggerFactory::parsePropagate(const JsonValue &settings) {
    std::optional<bool> propagate;
    const JsonValue* value = settings.find(Propagate);
    if (value) {
        propagate = value->asBool();
    }
    return propagate;
}

std::optional<bool> EasyEvent::LoggerFactory::parseDisabled(const JsonValue &settings) {
    std::optional<bool> disabled;
    const JsonValue* value = settings.find(Disabled);
    if (value) {
        disabled = value->asBool();
    }
    return disabled;
}

std::optional<std::vector<std::string>> EasyEvent::LoggerFactory::parseSinks(const JsonValue &settings) {
    std::optional<std::vector<std::string>> sinks;
    const JsonValue* value = settings.find(Sinks);
    if (value) {
        if (!value->isArray()) {
            std::string errMsg = "`" + Sinks + "' must be an array";
            throwError(UserErrors::BadValue, "LoggerFactory", errMsg);
        }
        sinks = std::vector<std::string>{};
        auto numSinks = value->size();
        for (size_t i = 0; i < numSinks; ++i) {
            sinks->emplace_back((*value)[i].asString());
        }
    }
    return sinks;
}