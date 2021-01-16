//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/ConsoleSink.h"


std::mutex EasyEvent::ConsoleSink::gMutex = {};

EasyEvent::ColorTypes EasyEvent::ConsoleSink::gColors[EasyEvent::NUM_ENABLED_LOG_LEVELS] = {
    EasyEvent::ColorTypes::LGRAY,
    EasyEvent::ColorTypes::GREEN,
    EasyEvent::ColorTypes::YELLOW,
    EasyEvent::ColorTypes::MAGENTA,
    EasyEvent::ColorTypes::RED
};

void EasyEvent::ConsoleSink::onWrite(LogRecord *record, const std::string &text) {
    std::lock_guard<std::mutex> lock(gMutex);
    LogLevel level = record->getLevel();
    if (_colored) {
        setColor(level);
        fprintf(stdout, "%s", text.c_str());
        resetColor(level);
    } else {
        fprintf(stdout, "%s", text.c_str());
    }
}

void EasyEvent::ConsoleSink::setColor(LogLevel level) {
    auto color = static_cast<uint8>(gColors[level - 1]);
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    static WORD WinColorFG[MaxColors] =
            {
                    0,                                                              // BLACK
                    FOREGROUND_RED,                                                 // RED
                    FOREGROUND_GREEN,                                               // GREEN
                    FOREGROUND_RED | FOREGROUND_GREEN,                              // BROWN
                    FOREGROUND_BLUE,                                                // BLUE
                    FOREGROUND_RED | FOREGROUND_BLUE,                               // MAGENTA
                    FOREGROUND_GREEN | FOREGROUND_BLUE,                             // CYAN
                    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,            // LGRAY
                    FOREGROUND_INTENSITY,                                           // DGRAY
                    FOREGROUND_RED | FOREGROUND_INTENSITY,                          // LRED
                    FOREGROUND_GREEN | FOREGROUND_INTENSITY,                        // LGREEN
                    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,       // YELLOW
                    FOREGROUND_BLUE | FOREGROUND_INTENSITY,                         // LBLUE
                    FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,        // LMAGENTA
                    FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,      // LCYAN
                    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY  // WHITE
            };

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, WinColorFG[color]);
#else
    enum ANSITextAttr
    {
        TA_NORMAL                                = 0,
        TA_BOLD                                  = 1,
        TA_BLINK                                 = 5,
        TA_REVERSE                               = 7
    };

    enum ANSIFgTextAttr
    {
        FG_BLACK                                 = 30,
        FG_RED,
        FG_GREEN,
        FG_BROWN,
        FG_BLUE,
        FG_MAGENTA,
        FG_CYAN,
        FG_WHITE,
    };

    enum ANSIBgTextAttr
    {
        BG_BLACK                                 = 40,
        BG_RED,
        BG_GREEN,
        BG_BROWN,
        BG_BLUE,
        BG_MAGENTA,
        BG_CYAN,
        BG_WHITE
    };

    static uint8 UnixColorFG[MaxColors] =
    {
        FG_BLACK,                                          // BLACK
        FG_RED,                                            // RED
        FG_GREEN,                                          // GREEN
        FG_BROWN,                                          // BROWN
        FG_BLUE,                                           // BLUE
        FG_MAGENTA,                                        // MAGENTA
        FG_CYAN,                                           // CYAN
        FG_WHITE,                                          // LGRAY
        FG_BLACK,                                          // DGRAY
        FG_RED,                                            // LRED
        FG_GREEN,                                          // LGREEN
        FG_BROWN,                                          // YELLOW
        FG_BLUE,                                           // LBLUE
        FG_MAGENTA,                                        // LMAGENTA
        FG_CYAN,                                           // LCYAN
        FG_WHITE,                                          // WHITE
    };

    fprintf(stdout, "\x1b[%s%dm", (color >= (uint8)ColorTypes::DGRAY && color < MaxColors ? "1;" : "0;"),
            UnixColorFG[color]);
#endif
}

void EasyEvent::ConsoleSink::resetColor(LogLevel level) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#else
    fprintf(stdout, "\x1b[0m");
#endif
}


const std::string EasyEvent::ConsoleSinkFactory::Colored = "colored";

EasyEvent::SinkPtr EasyEvent::ConsoleSinkFactory::create(const JsonValue &settings, LogLevel level, bool multiThread,
                                                         bool async, const std::string &fmt) const {
    bool colored = parseColored(settings);
    return ConsoleSink::create(colored, level, multiThread, async, fmt);
}

bool EasyEvent::ConsoleSinkFactory::parseColored(const JsonValue &settings) {
    bool colored = false;
    const JsonValue* value = settings.find(Colored);
    if (value) {
        colored = value->asBool();
    }
    return colored;
}
