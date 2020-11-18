//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/FileSink.h"


EasyEvent::FileSink::FileSink(std::string fileName, bool trunc, LogLevel level, SinkFlags flags)
    : Sink(level, flags)
    , _fileName(std::move(fileName))
    , _trunc(trunc) {
    if (isThreadSafe()) {
        _mutex = std::make_unique<std::mutex>();
    }
    _logFile = fopen(_fileName.c_str(), _trunc ? "w" : "a");
}

EasyEvent::FileSink::~FileSink() noexcept {
    closeFile();
}

void EasyEvent::FileSink::write(LogMessage *message, const std::string &text) {
    if (isThreadSafe()) {
        std::lock_guard<std::mutex> lock(*_mutex);
        _write(message, text);
    } else {
        _write(message, text);
    }
}

void EasyEvent::FileSink::_write(LogMessage *message, const std::string &text) {
    fprintf(_logFile, "%s\n", text.c_str());
    fflush(_logFile);
}