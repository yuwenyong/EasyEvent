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
    if (logger->getLevel() < level) {
        return false;
    }
    return true;
}