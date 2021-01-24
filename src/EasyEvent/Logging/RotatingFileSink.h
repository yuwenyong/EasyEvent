//
// Created by yuwenyong on 2020/11/14.
//

#ifndef EASYEVENT_LOGGING_ROTATINGFILESINK_H
#define EASYEVENT_LOGGING_ROTATINGFILESINK_H

#include "EasyEvent/Logging/LogCommon.h"
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
                         bool multiThread,
                         bool async,
                         const std::string& fmt,
                         MakeSharedTag tag)
                        : RotatingFileSink(std::move(fileName), maxBytes, backupCount, level, multiThread, async, fmt) {
            UnusedParameter(tag);
        }

        static SinkPtr create(std::string fileName, size_t maxBytes, size_t backupCount,
                              LogLevel level=LOG_LEVEL_DEFAULT, bool multiThread=false, bool async=false,
                              const std::string& fmt={}) {
            return std::make_shared<RotatingFileSink>(std::move(fileName), maxBytes, backupCount, level, multiThread,
                                                      async, fmt, MakeSharedTag{});
        }

    protected:
        RotatingFileSink(std::string fileName, size_t maxBytes, size_t backupCount, LogLevel level, bool multiThread,
                         bool async, const std::string& fmt);

        void onWrite(LogRecord *record, const std::string &text) override;

        void onClose() override;

        bool shouldRollover(const std::string& text) const {
            return _maxBytes > 0 && (text.size() + _fileSize > _maxBytes);
        }

        void doRollover();

        void openFile();

        void closeFile() {
            if (_logFile) {
                fclose(_logFile);
                _logFile = nullptr;
                _fileSize = 0;
            }
        }

        std::string _fileName;
        size_t _maxBytes;
        size_t _backupCount;
        FILE* _logFile{nullptr};
        size_t _fileSize;
    };


    class EASY_EVENT_API RotatingFileSinkFactory: public SinkFactory {
    public:
        SinkPtr create(const JsonValue &settings, LogLevel level, bool multiThread, bool async,
                       const std::string &fmt) const override;

        static std::string parseFileName(const JsonValue& settings);

        static size_t parseMaxBytes(const JsonValue& settings);

        static size_t parseBackupCount(const JsonValue& settings);

        static const std::string TypeName;
        static const std::string FileName;
        static const std::string MaxBytes;
        static const std::string BackupCount;
    };

}

#endif //EASYEVENT_LOGGING_ROTATINGFILESINK_H
