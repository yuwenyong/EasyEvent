//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_LOGGING_SINK_H
#define EASYEVENT_LOGGING_SINK_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Logging/LogRecord.h"
#include "EasyEvent/Logging/Formatter.h"
#include "EasyEvent/Common/TaskPool.h"


namespace EasyEvent {

    class EASY_EVENT_API Sink {
    public:
        Sink(const Sink&) = delete;
        Sink& operator=(const Sink&) = delete;

        virtual ~Sink();

        void setLevel(LogLevel level) {
            _level = level;
        }

        LogLevel getLevel() const {
            return _level;
        }

        void setFormat(const std::string& fmt) {
            if (_multiThread) {
                std::lock_guard<std::mutex> lock(*_mutex);
                _formatter.setFormat(fmt);
            } else {
                _formatter.setFormat(fmt);
            }
        }

        std::string getFormat() const {
            if (_multiThread) {
                std::lock_guard<std::mutex> lock(*_mutex);
                return _formatter.getFormat();
            } else {
                return _formatter.getFormat();
            }
        }

        void write(LogRecord* record);

        void writeQueued(LogRecord* record) {
            if (_multiThread) {
                std::lock_guard<std::mutex> lock(*_mutex);
                writeSync(record);
            } else {
                writeSync(record);
            }
        }

        void close() {
            if (_multiThread) {
                std::lock_guard<std::mutex> lock(*_mutex);
                doClose();
            } else {
                doClose();
            }
        }

        bool isAsync() const {
            return _async;
        }

        bool isMultiThread() const {
            return _multiThread;
        }
    protected:
        Sink(LogLevel level, bool multiThread, bool async, const std::string& fmt)
            : _level(level)
            , _multiThread(multiThread)
            , _async(async)
            , _formatter(fmt) {
            if (_multiThread) {
                _mutex = std::make_unique<std::mutex>();
            }
        }

        virtual void onWrite(LogRecord *record, const std::string& text) = 0;

        virtual void onClose();

        bool shouldLog(LogRecord* record) const {
            Assert(record != nullptr);
            return _level <= record->getLevel();
        }

        void doClose();

        void doWrite(LogRecord* record);

        void writeSync(LogRecord* record);

        void writeAsync(LogRecord* record);

        void ensureQueueCreated();

        std::unique_ptr<std::mutex> _mutex;
        LogLevel _level;
        bool _multiThread;
        bool _async;
        std::unique_ptr<TaskPool> _queue;
        Formatter _formatter;
        bool _closed{false};
    };

}

#endif //EASYEVENT_LOGGING_SINK_H
