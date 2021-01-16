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


const std::string EasyEvent::FileSinkFactory::FileName = "fileName";
const std::string EasyEvent::FileSinkFactory::Trunc = "trunc";


EasyEvent::SinkPtr EasyEvent::FileSinkFactory::create(const JsonValue &settings, LogLevel level, bool multiThread,
                                                      bool async, const std::string &fmt) const {
    std::string fileName = parseFileName(settings);
    bool trunc = parseTrunc(settings);
    return FileSink::create(std::move(fileName), trunc, level, multiThread, async, fmt);
}

std::string EasyEvent::FileSinkFactory::parseFileName(const JsonValue &settings) {
    const JsonValue* value = settings.find(FileName);
    if (!value) {
        std::string errMsg = "Argument `" + FileName + "' is required";
        throwError(UserErrors::ArgumentRequired, "FileSinkFactory", errMsg);
    }
    std::string fileName = value->asString();
    if (fileName.empty()) {
        std::string errMsg = "Invalid value `" + fileName + "' for " + FileName;
        throwError(UserErrors::BadValue, "FileSinkSinkFactory", errMsg);
    }
    return fileName;
}

bool EasyEvent::FileSinkFactory::parseTrunc(const JsonValue &settings) {
    bool trunc = false;
    const JsonValue* value = settings.find(Trunc);
    if (value) {
        trunc = value->asBool();
    }
    return trunc;
}
