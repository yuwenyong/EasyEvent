//
// Created by yuwenyong on 2020/11/14.
//

#ifndef EASYEVENT_LOGGING_CONSOLESINK_H
#define EASYEVENT_LOGGING_CONSOLESINK_H

#include "EasyEvent/Logging/LogCommon.h"
#include "EasyEvent/Logging/Sink.h"


namespace EasyEvent {

    enum class ColorTypes: uint8
    {
        BLACK,
        RED,
        GREEN,
        BROWN,
        BLUE,
        MAGENTA,
        CYAN,
        LGRAY,
        DGRAY,
        LRED,
        LGREEN,
        YELLOW,
        LBLUE,
        LMAGENTA,
        LCYAN,
        WHITE,
    };

    constexpr uint8 MaxColors = uint8(ColorTypes::WHITE) + 1;

    class EASY_EVENT_API ConsoleSink: public Sink {
    private:
        struct MakeSharedTag {};

    public:
        ConsoleSink(bool colored, LogLevel level, bool multiThread, bool async, const std::string& fmt, MakeSharedTag tag)
            : ConsoleSink(colored, level, multiThread, async, fmt) {

        }

        static SinkPtr create(bool colored=false, LogLevel level=LOG_LEVEL_DEFAULT, bool multiThread=false,
                              bool async=false, const std::string& fmt={}) {

            return std::make_shared<ConsoleSink>(colored, level, multiThread, async, fmt, MakeSharedTag{});
        }
    protected:

        ConsoleSink(bool colored, LogLevel level, bool multiThread, bool async, const std::string& fmt)
            : Sink(level, multiThread, async, fmt)
            , _colored(colored) {

        }

        void onWrite(LogRecord *record, const std::string &text) override;

        void setColor(LogLevel level);

        void resetColor(LogLevel level);

        bool _colored;

        static std::mutex gMutex;
        static ColorTypes gColors[NUM_ENABLED_LOG_LEVELS];
    };


    class EASY_EVENT_API ConsoleSinkFactory: public SinkFactory {
    public:
        SinkPtr create(const JsonValue& settings, LogLevel level, bool multiThread, bool async,
                       const std::string& fmt) const override;

        static bool parseColored(const JsonValue& settings);

        static const std::string Colored;
    };

}


#endif //EASYEVENT_LOGGING_CONSOLESINK_H
