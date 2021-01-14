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
        TimedRotatingFileSink(std::string fileName, TimedRotatingWhen when, LogLevel level, bool multiThread,
                              bool async, const std::string& fmt,
                              MakeSharedTag tag)
                              : TimedRotatingFileSink(std::move(fileName), when, level, multiThread, async, fmt) {

        }

        static SinkPtr create(std::string fileName, TimedRotatingWhen when=TimedRotatingWhen::Day,
                              LogLevel level=LOG_LEVEL_DEFAULT, bool multiThread=false, bool async=false,
                              const std::string& fmt={}) {
            return std::make_shared<TimedRotatingFileSink>(std::move(fileName), when, level, multiThread, async, fmt,
                                                           MakeSharedTag{});
        }

    protected:
        TimedRotatingFileSink(std::string fileName, TimedRotatingWhen when, LogLevel level, bool multiThread,
                              bool async, const std::string& fmt);

        void onWrite(LogRecord *record, const std::string &text) override;

        void onClose() override;

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
        FILE* _logFile{nullptr};
        Time _rolloverAt;
    };
}

#endif //EASYEVENT_LOGGING_TIMEDROTATINGFILESINK_H
