//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_LOGCOMMON_H
#define EASYEVENT_LOGGING_LOGCOMMON_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Error.h"

namespace EasyEvent {

    enum LogLevel {
        LOG_LEVEL_DEBUG                              = 1,
        LOG_LEVEL_INFO                               = 2,
        LOG_LEVEL_WARN                               = 3,
        LOG_LEVEL_ERROR                              = 4,
        LOG_LEVEL_CRITICAL                           = 5,
        LOG_LEVEL_DISABLE                            = 6,
        LOG_LEVEL_DEFAULT                            = LOG_LEVEL_INFO,
        NUM_ENABLED_LOG_LEVELS                       = 5,
    };

    class Log;
    class LogRecord;
    class Logger;
    class Sink;
    class SinkFactory;

    using SinkPtr = std::shared_ptr<Sink>;

    class Bin {
    public:
        Bin(const void* data, size_t size): _data(data), _size(size) {}

        Bin(const void* data, size_t size, size_t maxSize): _data(data), _size(size), _maxSize(maxSize) {}

        const void* getData() const {
            return _data;
        }

        size_t getSize() const {
            return _size;
        }

        size_t getMaxSize() const {
            return _maxSize;
        }
    private:
        const void* _data;
        size_t _size;
        size_t _maxSize{0};
    };

    EASY_EVENT_API void dumpData(const void* data, size_t size, std::ostream& strm);

    EASY_EVENT_API std::ostream& operator<<(std::ostream &sout, const Bin &bin);
}

#endif //EASYEVENT_LOGGING_LOGCOMMON_H
