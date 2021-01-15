//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/Sink.h"
#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/Formatter.h"


EasyEvent::Sink::~Sink() {
    close();
}

void EasyEvent::Sink::write(LogRecord *record) {
    if (!shouldLog(record)) {
        return;
    }
    if (_multiThread) {
        std::lock_guard<std::mutex> lock(*_mutex);
        doWrite(record);
    } else {
        doWrite(record);
    }
}

void EasyEvent::Sink::onClose() {

}

void EasyEvent::Sink::doClose() {
    if (!_closed) {
        if (_queue) {
            _queue->stop();
            _queue->wait();
            _queue.reset();
        }
        onClose();
        _closed = true;
    }
}

void EasyEvent::Sink::doWrite(LogRecord *record) {
    if (_closed) {
        return;
    }
    if (_async) {
        writeAsync(record);
    } else {
        writeSync(record);
    }
}

void EasyEvent::Sink::writeSync(LogRecord *record) {
    std::string text = _formatter.format(record);
    onWrite(record, text);
}

void EasyEvent::Sink::writeAsync(LogRecord *record) {
    ensureQueueCreated();
    auto clonedRecord = record->clone(this);
    _queue->post([record=std::move(clonedRecord)]() mutable {
        record->writeAsync();
    });
}

void EasyEvent::Sink::ensureQueueCreated() {
    if (!_queue) {
        _queue = std::make_unique<TaskPool>();
        _queue->start(1);
    }
}

const std::string EasyEvent::SinkFactory::Name = "name";
const std::string EasyEvent::SinkFactory::Type = "type";
const std::string EasyEvent::SinkFactory::Level = "level";
const std::string EasyEvent::SinkFactory::MultiThread = "multiThread";
const std::string EasyEvent::SinkFactory::Async = "async";
const std::string EasyEvent::SinkFactory::Format = "format";

std::string EasyEvent::SinkFactory::parseName(const JsonValue &settings) {
    return settings[Name].asString();
}

std::string EasyEvent::SinkFactory::parseType(const JsonValue &settings) {
    return settings[Type].asString();
}

EasyEvent::LogLevel EasyEvent::SinkFactory::parseLevel(const JsonValue &settings) {
    JsonValue defaultLevel("debug");
    std::string level = settings.get(Level, defaultLevel).asString();
    if (stricmp(level.c_str(), "critical") == 0) {
        return LOG_LEVEL_CRITICAL;
    } else if (stricmp(level.c_str(), "error") == 0) {
        return LOG_LEVEL_ERROR;
    } else if (stricmp(level.c_str(), "warn") == 0) {
        return LOG_LEVEL_WARN;
    } else if (stricmp(level.c_str(), "info") == 0) {
        return LOG_LEVEL_INFO;
    } else if (stricmp(level.c_str(), "debug") == 0) {
        return LOG_LEVEL_DEBUG;
    } else {
        std::string errMsg = "Invalid value `" + level + "' for log level";
        throwError(UserErrors::BadValue, "SinkFactory", errMsg);
        return LOG_LEVEL_DISABLE;
    }
}

bool EasyEvent::SinkFactory::parseMultiThread(const JsonValue &settings) {
    JsonValue defaultValue(false);
    return settings.get(MultiThread, defaultValue).asBool();
}

bool EasyEvent::SinkFactory::parseAsync(const JsonValue &settings) {
    JsonValue defaultValue(false);
    return settings.get(Async, defaultValue).asBool();
}

std::string EasyEvent::SinkFactory::parseFormat(const JsonValue &settings) {
    JsonValue defaultValue(Formatter::DefaultFormat);
    return settings.get(Format, defaultValue).asString();
}