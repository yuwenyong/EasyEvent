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
        ConsoleSink(bool colored, LogLevel level, SinkFlags flags, MakeSharedTag tag)
            : ConsoleSink(colored, level, flags) {

        }

        static SinkPtr create(bool colored=false, LogLevel level=LOG_LEVEL_DEFAULT,
                              SinkFlags flags=SINK_FLAGS_DEFAULT) {

            return std::make_shared<ConsoleSink>(colored, level, flags, MakeSharedTag{});
        }
    protected:

        ConsoleSink(bool colored, LogLevel level, SinkFlags flags)
            : Sink(level, flags)
            , _colored(colored) {
            if (isThreadSafe()) {
                _mutex = gMutex;
            }
        }

        void write(LogMessage *message, const std::string &text) override;

        void _write(LogMessage *message, const std::string &text);

        void setColor(LogLevel level);

        void resetColor(LogLevel level);

        bool _colored;
        std::shared_ptr<std::mutex> _mutex;

        static std::shared_ptr<std::mutex> gMutex;
        static ColorTypes gColors[NUM_ENABLED_LOG_LEVELS];
    };

}


#endif //EASYEVENT_LOGGING_CONSOLESINK_H
