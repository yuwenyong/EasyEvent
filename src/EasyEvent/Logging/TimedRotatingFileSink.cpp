//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/TimedRotatingFileSink.h"


EasyEvent::TimedRotatingFileSink::TimedRotatingFileSink(std::string fileName, TimedRotatingWhen when, LogLevel level,
                                                        SinkFlags flags)
                                                        : Sink(level, flags)
                                                        , _fileName(std::move(fileName))
                                                        , _when(when) {
    if (isThreadSafe()) {
        _mutex = std::make_unique<std::mutex>();
    }
    computeRollover();
    openFile();
}

EasyEvent::TimedRotatingFileSink::~TimedRotatingFileSink() noexcept {
    closeFile();
}

void EasyEvent::TimedRotatingFileSink::write(LogMessage *message, const std::string &text) {
    if (isThreadSafe()) {
        std::lock_guard<std::mutex> lock(*_mutex);
        _write(message, text);
    } else {
        _write(message, text);
    }
}

void EasyEvent::TimedRotatingFileSink::_write(LogMessage *message, const std::string &text) {
    if (shouldRollover()) {
        doRollover();
    }
    fprintf(_logFile, "%s\n", text.c_str());
    fflush(_logFile);
}

bool EasyEvent::TimedRotatingFileSink::shouldRollover() {
    Time t = Time::now();
    if (t.seconds() > _rolloverAt.seconds()) {
        struct tm dt = t.localTime();
        struct tm dt2 = _rolloverAt.localTime();
        switch (_when) {
            case EasyEvent::TimedRotatingWhen::Day: {
                if (dt.tm_year == dt2.tm_year && dt.tm_mon == dt2.tm_mon && dt.tm_mday == dt2.tm_mday) {
                    _rolloverAt = t;
                    return false;
                }
                break;
            }
            case EasyEvent::TimedRotatingWhen::Hour: {
                if (dt.tm_year == dt2.tm_year && dt.tm_mon == dt2.tm_mon && dt.tm_mday == dt2.tm_mday &&
                    dt.tm_hour == dt2.tm_hour) {
                    _rolloverAt = t;
                    return false;
                }
                break;
            }
            case EasyEvent::TimedRotatingWhen::Minute: {
                if (dt.tm_year == dt2.tm_year && dt.tm_mon == dt2.tm_mon && dt.tm_mday == dt2.tm_mday &&
                    dt.tm_hour == dt2.tm_hour && dt.tm_min == dt2.tm_min) {
                    _rolloverAt = t;
                    return false;
                }
                break;
            }
            default: {
                Assert(false);
                break;
            }
        }
        return true;
    } else {
        return false;
    }
}

void EasyEvent::TimedRotatingFileSink::doRollover() {
    closeFile();
    computeRollover();
    openFile();
}

void EasyEvent::TimedRotatingFileSink::computeRollover() {
    Time t = Time::now();
    struct tm dt = t.localTime();
    switch (_when) {
        case EasyEvent::TimedRotatingWhen::Day: {
            dt.tm_hour = 23;
            dt.tm_min = 59;
            dt.tm_sec = 59;
            break;
        }
        case EasyEvent::TimedRotatingWhen::Hour: {
            dt.tm_min = 59;
            dt.tm_sec = 59;
            break;
        }
        case EasyEvent::TimedRotatingWhen::Minute: {
            dt.tm_sec = 59;
            break;
        }
        default: {
            Assert(false);
            break;
        }
    }
    _rolloverAt = Time::makeTime(dt);
}

void EasyEvent::TimedRotatingFileSink::openFile() {
    std::string suffix;
    switch (_when) {
        case EasyEvent::TimedRotatingWhen::Day: {
            suffix = _rolloverAt.toString("%Y-%m-%d");
            break;
        }
        case EasyEvent::TimedRotatingWhen::Hour: {
            suffix = _rolloverAt.toString("%Y-%m-%d_%H");
            break;
        }
        case EasyEvent::TimedRotatingWhen::Minute: {
            suffix = _rolloverAt.toString("%Y-%m-%d_%H-%M");
            break;
        }
        default: {
            Assert(false);
            break;
        }
    }
    std::string fileName = _fileName;
    auto pos = fileName.find_last_of('.');
    if (pos != std::string::npos) {
        fileName.insert(pos, suffix);
    } else {
        fileName += suffix;
    }
    _logFile = fopen(fileName.c_str(), "a");
}