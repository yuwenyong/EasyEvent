//
// Created by yuwenyong on 2020/11/14.
//

#ifndef EASYEVENT_LOGGING_TIMEDROTATINGFILESINK_H
#define EASYEVENT_LOGGING_TIMEDROTATINGFILESINK_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Logging/Sink.h"


namespace EasyEvent {

    enum class TimedRotatingWhen {
        Day,
        Hour,
        Minute,
    };

    class EASY_EVENT_API TimedRotatingFileSink: public Sink {
    private:
        struct MakeSharedTag {};

    public:
        TimedRotatingFileSink(std::string fileName, TimedRotatingWhen when, LogLevel level, SinkFlags flags,
                              MakeSharedTag tag)
                              : TimedRotatingFileSink(std::move(fileName), when, level, flags) {

        }

        ~TimedRotatingFileSink() noexcept override;

        static SinkPtr create(std::string fileName, TimedRotatingWhen when=TimedRotatingWhen::Day,
                              LogLevel level=LOG_LEVEL_DEFAULT, SinkFlags flags=SINK_FLAGS_DEFAULT) {
            return std::make_shared<TimedRotatingFileSink>(std::move(fileName), when, level, flags,MakeSharedTag{});
        }

    protected:
        TimedRotatingFileSink(std::string fileName, TimedRotatingWhen when, LogLevel level, SinkFlags flags);

        void write(LogMessage *message, const std::string &text) override;

        void _write(LogMessage *message, const std::string &text);

        bool shouldRollover();

        void doRollover();

        void computeRollover();

        void openFile();

        void closeFile() {
            if (_logFile) {
                fclose(_logFile);
                _logFile = nullptr;
            }
        }

        std::string _fileName;
        TimedRotatingWhen _when;
        std::unique_ptr<std::mutex> _mutex;
        FILE* _logFile{nullptr};
        Time _rolloverAt;
    };
}

#endif //EASYEVENT_LOGGING_TIMEDROTATINGFILESINK_H
