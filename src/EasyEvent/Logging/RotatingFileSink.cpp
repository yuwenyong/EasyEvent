//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/RotatingFileSink.h"


EasyEvent::RotatingFileSink::RotatingFileSink(std::string fileName, size_t maxBytes, size_t backupCount, LogLevel level,
                                              bool multiThread, bool async, const std::string& fmt)
                                              : Sink(level, multiThread, async, fmt)
                                              , _fileName(std::move(fileName))
                                              , _maxBytes(maxBytes)
                                              , _backupCount(std::max(backupCount, (size_t)1u)) {
    openFile();
}

void EasyEvent::RotatingFileSink::onWrite(LogRecord *record, const std::string &text) {
    if (shouldRollover(text)) {
        doRollover();
    }
    fprintf(_logFile, "%s", text.c_str());
    fflush(_logFile);
    _fileSize += text.size();
}

void EasyEvent::RotatingFileSink::onClose() {
    closeFile();
}

void EasyEvent::RotatingFileSink::doRollover() {
    namespace fs = std::filesystem;
    closeFile();

    int curIndex = (int)_backupCount - 1;
    fs::path srcPath;
    fs::path destPath;
    while (curIndex > 0) {
        srcPath = _fileName + "." + std::to_string(curIndex);
        destPath = _fileName + "." + std::to_string(curIndex + 1);
        if (fs::exists(srcPath)) {
            if (fs::exists(destPath)) {
                fs::remove(destPath);
            }
            fs::rename(srcPath, destPath);
        }
        --curIndex;
    }
    srcPath = _fileName;
    destPath = _fileName + ".1";
    if (fs::exists(destPath)) {
        fs::remove(destPath);
    }
    fs::rename(srcPath, destPath);

    openFile();
}

void EasyEvent::RotatingFileSink::openFile() {
    _logFile = fopen(_fileName.c_str(), "a");
    if (!_logFile) {
        throwError(errno, "RotatingFileSink");
    }
    _fileSize = (size_t)ftell(_logFile);
}


const std::string EasyEvent::RotatingFileSinkFactory::TypeName = "RotatingFile";
const std::string EasyEvent::RotatingFileSinkFactory::FileName = "fileName";
const std::string EasyEvent::RotatingFileSinkFactory::MaxBytes = "maxBytes";
const std::string EasyEvent::RotatingFileSinkFactory::BackupCount = "backupCount";


EasyEvent::SinkPtr EasyEvent::RotatingFileSinkFactory::create(const JsonValue &settings, LogLevel level, bool multiThread,
                                                              bool async, const std::string &fmt) const {
    std::string fileName = parseFileName(settings);
    size_t maxBytes = parseMaxBytes(settings);
    size_t backupCount = parseBackupCount(settings);
    return RotatingFileSink::create(std::move(fileName), maxBytes, backupCount, level, multiThread, async, fmt);

}

std::string EasyEvent::RotatingFileSinkFactory::parseFileName(const JsonValue &settings) {
    const JsonValue* value = settings.find(FileName);
    if (!value) {
        std::string errMsg = "Argument `" + FileName + "' is required";
        throwError(UserErrors::ArgumentRequired, "RotatingFileSinkFactory", errMsg);
    }
    std::string fileName = value->asString();
    if (fileName.empty()) {
        std::string errMsg = FileName + " can't be empty";
        throwError(UserErrors::BadValue, "RotatingFileSinkFactory", errMsg);
    }
    return fileName;
}

size_t EasyEvent::RotatingFileSinkFactory::parseMaxBytes(const JsonValue &settings) {
    const JsonValue* value = settings.find(MaxBytes);
    if (!value) {
        std::string errMsg = "Argument `" + MaxBytes + "' is required";
        throwError(UserErrors::ArgumentRequired, "RotatingFileSinkFactory", errMsg);
    }
    return (size_t)value->asUInt64();
}

size_t EasyEvent::RotatingFileSinkFactory::parseBackupCount(const JsonValue &settings) {
    const JsonValue* value = settings.find(BackupCount);
    if (!value) {
        std::string errMsg = "Argument `" + BackupCount + "' is required";
        throwError(UserErrors::ArgumentRequired, "RotatingFileSinkFactory", errMsg);
    }
    return (size_t)value->asUInt64();
}