//
// Created by yuwenyong on 2021/1/10.
//

#ifndef EASYEVENT_LOGGING_FORMATTER_H
#define EASYEVENT_LOGGING_FORMATTER_H

#include "EasyEvent/Logging/LogCommon.h"


namespace EasyEvent {

    class EASY_EVENT_API Formatter {
    public:
        Formatter() {
            setFormat(DefaultFormat);
        }

        explicit Formatter(const std::string& fmt) {
            setFormat(fmt.empty() ? DefaultFormat : fmt);
        }

        void setFormat(const std::string& fmt) {
            _nodes = parse(fmt);
            _fmt = fmt;
        }

        std::string getFormat() const {
            return _fmt;
        }

        std::string format(const LogRecord* record) const;

        static const std::string DefaultFormat;
    protected:
        enum class NodeType {
            Text = 0,
            Message,        // ${message}
            Timestamp,      // ${timestamp}
            LogLevel,       // ${level}
            FileName,       // ${filename}
            LineNo,         // ${lineno}
            FuncName,       // ${funcName}
            ThreadId,       // ${thread}
            LoggerName,     // ${name}
        };

        struct Node {
            NodeType type;
            std::string text;
        };

        static std::vector<Node> parse(const std::string& fmt);

        static const char* getLevelString(LogLevel level);

        std::string _fmt;
        std::vector<Node> _nodes;

        static const std::map<std::string, NodeType> PatternMapping;
    };

}

#endif //EASYEVENT_LOGGING_FORMATTER_H
