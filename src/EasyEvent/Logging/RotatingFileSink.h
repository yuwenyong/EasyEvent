//
// Created by yuwenyong on 2020/11/14.
//

#ifndef EASYEVENT_LOGGING_ROTATINGFILESINK_H
#define EASYEVENT_LOGGING_ROTATINGFILESINK_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Logging/Sink.h"


namespace EasyEvent {
    class EASY_EVENT_API RotatingFileSink: public Sink {
    private:
        struct MakeSharedTag {};

    public:
        RotatingFileSink(std::string fileName,
                         size_t maxBytes,
                         size_t backupCount,
                         LogLevel level,
                         SinkFlags flags,
                         MakeSharedTag tag)
        : RotatingFileSink(std::move(fileName), maxBytes, backupCount, level, flags) {

        }

        ~RotatingFileSink() noexcept override;

        static SinkPtr create(std::string fileName, size_t maxBytes, size_t backupCount,
                              LogLevel level=LOG_LEVEL_DEFAULT, SinkFlags flags=SINK_FLAGS_DEFAULT) {
            return std::make_shared<RotatingFileSink>(std::move(fileName), maxBytes, backupCount, level, flags,
                                                      MakeSharedTag{});
        }

    protected:
        RotatingFileSink(std::string fileName, size_t maxBytes, size_t backupCount, LogLevel level, SinkFlags flags);

        void write(LogMessage *message, const std::string &text) override;

        void _write(LogMessage *message, const std::string &text);

        bool shouldRollover(const std::string& text) {
            return _maxBytes > 0 && (text.size() + _fileSize > _maxBytes);
        }

        void doRollover();

        void openFile();

        void closeFile() {
            if (_logFile) {
                fclose(_logFile);
                _logFile = nullptr;
            }
        }

        std::string _fileName;
        size_t _maxBytes;
        size_t _backupCount;
        std::unique_ptr<std::mutex> _mutex;
        FILE* _logFile{nullptr};
        size_t _fileSize;
    };
}

#endif //EASYEVENT_LOGGING_ROTATINGFILESINK_H
