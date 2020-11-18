//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/RotatingFileSink.h"


EasyEvent::RotatingFileSink::RotatingFileSink(std::string fileName, size_t maxBytes, size_t backupCount, LogLevel level,
                                              SinkFlags flags)
                                              : Sink(level, flags)
                                              , _fileName(std::move(fileName))
                                              , _maxBytes(maxBytes)
                                              , _backupCount(std::max(backupCount, 1ul)) {
    if (isThreadSafe()) {
        _mutex = std::make_unique<std::mutex>();
    }
    openFile();
}

EasyEvent::RotatingFileSink::~RotatingFileSink() noexcept {
    closeFile();
}

void EasyEvent::RotatingFileSink::write(LogMessage *message, const std::string &text) {
    if (isThreadSafe()) {
        std::lock_guard<std::mutex> lock(*_mutex);
        _write(message, text);
    } else {
        _write(message, text);
    }
}

void EasyEvent::RotatingFileSink::_write(LogMessage *message, const std::string &text) {
    if (shouldRollover(text)) {
        doRollover();
    }
    fprintf(_logFile, "%s\n", text.c_str());
    fflush(_logFile);
    _fileSize += text.size() + 1;
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