//
// Created by yuwenyong on 2020/11/14.
//

#ifndef EASYEVENT_LOGGING_FILESINK_H
#define EASYEVENT_LOGGING_FILESINK_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Logging/Sink.h"


namespace EasyEvent {

    class EASY_EVENT_API FileSink: public Sink {
    private:
        struct MakeSharedTag {};
    public:
        FileSink(std::string fileName, bool trunc, LogLevel level, bool multiThread, bool async, const std::string& fmt,
                 MakeSharedTag tag)
            : FileSink(std::move(fileName), trunc, level, multiThread, async, fmt) {

        }

        static SinkPtr create(std::string fileName, bool trunc=false, LogLevel level=LOG_LEVEL_DEFAULT,
                              bool multiThread=false, bool async=false, const std::string& fmt={}) {
            return std::make_shared<FileSink>(std::move(fileName), trunc, level, multiThread, async, fmt, MakeSharedTag{});
        }
    protected:
        FileSink(std::string fileName, bool trunc, LogLevel level, bool multiThread, bool async, const std::string& fmt);

        void onWrite(LogRecord *record, const std::string &text) override;

        void onClose() override;

        void closeFile() {
            if (_logFile) {
                fclose(_logFile);
                _logFile = nullptr;
            }
        }

        std::string _fileName;
        bool _trunc;
        FILE* _logFile{nullptr};
    };


    class EASY_EVENT_API FileSinkFactory: public SinkFactory {
    public:
        SinkPtr create(const JsonValue &settings, LogLevel level, bool multiThread, bool async,
                       const std::string &fmt) const override;

        static std::string parseFileName(const JsonValue& settings);

        static bool parseTrunc(const JsonValue& settings);

        static const std::string FileName;
        static const std::string Trunc;
    };

}

#endif //EASYEVENT_LOGGING_FILESINK_H
