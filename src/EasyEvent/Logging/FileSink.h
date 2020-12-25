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
        FileSink(std::string fileName, bool trunc, LogLevel level, SinkFlags flags, MakeSharedTag tag)
            : FileSink(std::move(fileName), trunc, level, flags) {

        }

        ~FileSink() noexcept override;

        static SinkPtr create(std::string fileName, bool trunc=false, LogLevel level=LOG_LEVEL_DEFAULT,
                              SinkFlags flags=SINK_FLAGS_DEFAULT) {
            return std::make_shared<FileSink>(std::move(fileName), trunc, level, flags, MakeSharedTag{});
        }
    protected:
        FileSink(std::string fileName, bool trunc, LogLevel level, SinkFlags flags);

        void write(LogMessage *message, const std::string &text) override;

        void _write(LogMessage *message, const std::string &text);

        void closeFile() {
            if (_logFile) {
                fclose(_logFile);
                _logFile = nullptr;
            }
        }

        std::string _fileName;
        bool _trunc;
        std::unique_ptr<std::mutex> _mutex;
        FILE* _logFile{nullptr};
    };
}

#endif //EASYEVENT_LOGGING_FILESINK_H
