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
    _fileSize = (size_t)ftell(_logFile);
}