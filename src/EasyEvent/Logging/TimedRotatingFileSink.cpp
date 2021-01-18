//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/TimedRotatingFileSink.h"


EasyEvent::TimedRotatingFileSink::TimedRotatingFileSink(std::string fileName, TimedRotatingWhen when, LogLevel level,
                                                        bool multiThread, bool async, const std::string& fmt)
                                                        : Sink(level, multiThread, async, fmt)
                                                        , _fileName(std::move(fileName))
                                                        , _when(when) {
    computeRollover();
    openFile();
}

void EasyEvent::TimedRotatingFileSink::onWrite(LogRecord *record, const std::string &text) {
    if (shouldRollover()) {
        doRollover();
    }
    fprintf(_logFile, "%s", text.c_str());
    fflush(_logFile);
}

void EasyEvent::TimedRotatingFileSink::onClose() {
    closeFile();
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
    if (!_logFile) {
        throwError(errno, "TimedRotatingFileSink");
    }
}


const std::string EasyEvent::TimedRotatingFileSinkFactory::TypeName = "TimedRotatingFile";
const std::string EasyEvent::TimedRotatingFileSinkFactory::FileName = "fileName";
const std::string EasyEvent::TimedRotatingFileSinkFactory::When = "when";

EasyEvent::SinkPtr EasyEvent::TimedRotatingFileSinkFactory::create(const JsonValue &settings, LogLevel level, bool multiThread,
                                                                   bool async, const std::string &fmt) const {
    std::string fileName = parseFileName(settings);
    TimedRotatingWhen when = parseMWhen(settings);
    return TimedRotatingFileSink::create(std::move(fileName), when, level, multiThread, async, fmt);
}

std::string EasyEvent::TimedRotatingFileSinkFactory::parseFileName(const JsonValue &settings) {
    const JsonValue* value = settings.find(FileName);
    if (!value) {
        std::string errMsg = "Argument `" + FileName + "' is required";
        throwError(UserErrors::ArgumentRequired, "TimedRotatingFileSinkFactory", errMsg);
    }
    std::string fileName = value->asString();
    if (fileName.empty()) {
        std::string errMsg = FileName + " can't be empty";
        throwError(UserErrors::BadValue, "TimedRotatingFileSinkFactory", errMsg);
    }
    return fileName;
}

EasyEvent::TimedRotatingWhen EasyEvent::TimedRotatingFileSinkFactory::parseMWhen(const JsonValue &settings) {
    std::string when = "day";
    const JsonValue* value = settings.find(When);
    if (value) {
        when = value->asString();
    }
    if (stricmp(when.c_str(), "day") == 0) {
        return TimedRotatingWhen::Day;
    } else if (stricmp(when.c_str(), "hour") == 0) {
        return TimedRotatingWhen::Hour;
    } else if (stricmp(when.c_str(), "minute") == 0) {
        return TimedRotatingWhen::Minute;
    } else {
        std::string errMsg = "Invalid value `" + when + "' for " + When;
        throwError(UserErrors::BadValue, "TimedRotatingFileSinkFactory", errMsg);
        return TimedRotatingWhen::Day;
    }
}