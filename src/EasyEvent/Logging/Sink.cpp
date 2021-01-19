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
    const JsonValue* value = settings.find(Name);
    if (!value) {
        std::string errMsg = "Argument `" + Name + "' is required";
        throwError(UserErrors::ArgumentRequired, "SinkFactory", errMsg);
    }
    std::string name = value->asString();
    if (name.empty()) {
        std::string errMsg = Name + " can't be empty";
        throwError(UserErrors::BadValue, "SinkFactory", errMsg);
    }
    return name;
}

std::string EasyEvent::SinkFactory::parseType(const JsonValue &settings) {
    const JsonValue* value = settings.find(Type);
    if (!value) {
        std::string errMsg = "Argument `" + Type + "' is required";
        throwError(UserErrors::ArgumentRequired, "SinkFactory", errMsg);
    }
    std::string type = value->asString();
    if (type.empty()) {
        std::string errMsg = Type + " can't be empty";
        throwError(UserErrors::BadValue, "SinkFactory", errMsg);
    }
    return type;
}

EasyEvent::LogLevel EasyEvent::SinkFactory::parseLevel(const JsonValue &settings) {
    std::string level = "debug";
    const JsonValue* value = settings.find(Level);
    if (value) {
        level = value->asString();
    }
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
    } else if (stricmp(level.c_str(), "disable") == 0) {
        return LOG_LEVEL_DISABLE;
    } else {
        std::string errMsg = "Invalid value `" + level + "' for " + Level;
        throwError(UserErrors::BadValue, "SinkFactory", errMsg);
        return LOG_LEVEL_DISABLE;
    }
}

bool EasyEvent::SinkFactory::parseMultiThread(const JsonValue &settings) {
    bool multiThread = false;
    const JsonValue* value = settings.find(MultiThread);
    if (value) {
        multiThread = value->asBool();
    }
    return multiThread;
}

bool EasyEvent::SinkFactory::parseAsync(const JsonValue &settings) {
    bool async = false;
    const JsonValue* value = settings.find(Async);
    if (value) {
        async = value->asBool();
    }
    return async;
}

std::string EasyEvent::SinkFactory::parseFormat(const JsonValue &settings) {
    std::string format = Formatter::DefaultFormat;
    const JsonValue* value = settings.find(Format);
    if (value) {
        format = value->asString();
    }
    return format;
}