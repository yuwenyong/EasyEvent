//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/FileSink.h"


EasyEvent::FileSink::FileSink(std::string fileName, bool trunc, LogLevel level, bool multiThread, bool async,
                              const std::string& fmt)
    : Sink(level, multiThread, async, fmt)
    , _fileName(std::move(fileName))
    , _trunc(trunc) {
    _logFile = fopen(_fileName.c_str(), _trunc ? "w" : "a");
}

void EasyEvent::FileSink::onWrite(LogRecord *record, const std::string &text) {
    fprintf(_logFile, "%s", text.c_str());
    fflush(_logFile);
}

void EasyEvent::FileSink::onClose() {
    closeFile();
}