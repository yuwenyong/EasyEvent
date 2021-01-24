//
// Created by yuwenyong on 2021/1/10.
//

#include "EasyEvent/Logging/Formatter.h"
#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/LogRecord.h"


const std::string EasyEvent::Formatter::DefaultFormat = "[${timestamp}][${level}]${message}";

const std::map<std::string, EasyEvent::Formatter::NodeType> EasyEvent::Formatter::PatternMapping = {
        {"message", Formatter::NodeType::Message},
        {"timestamp", Formatter::NodeType::Timestamp},
        {"level", Formatter::NodeType::LogLevel},
        {"filename", Formatter::NodeType::FileName},
        {"lineno", Formatter::NodeType::LineNo},
        {"funcName", Formatter::NodeType::FuncName},
        {"thread", Formatter::NodeType::ThreadId},
        {"name", Formatter::NodeType::LoggerName}
};

std::string EasyEvent::Formatter::format(const LogRecord *record) const {
    std::stringstream ss;
    for (auto& node: _nodes) {
        switch (node.type) {
            case NodeType::Text:
                ss << node.text;
                break;
            case NodeType::Message:
                ss << record->getMessage();
                break;
            case NodeType::Timestamp:
                ss << record->getTimestamp().toDateTimeString();
                break;
            case NodeType::LogLevel:
                ss << getLevelString(record->getLevel());
                break;
            case NodeType::FileName:
                ss << record->getFileName();
                break;
            case NodeType::LineNo:
                ss << record->getLineno();
                break;
            case NodeType::FuncName:
                ss << record->getFuncName();
                break;
            case NodeType::ThreadId:
                ss << record->getThreadId();
                break;
            case NodeType::LoggerName:
                ss << record->getLogger()->getName();
                break;
        }
    }
    std::string text = ss.str();
    if (text.empty() || text.back() != '\n') {
        text.push_back('\n');
    }
    return text;
}

std::vector<EasyEvent::Formatter::Node> EasyEvent::Formatter::parse(const std::string &fmt) {
    std::string::size_type cpos = 0;
    std::vector<Formatter::Node> nodes;
    while (cpos < fmt.size()) {
        auto spos = fmt.find("${", cpos);
        if (spos == std::string::npos) {
            nodes.emplace_back(Node{NodeType::Text, fmt.substr(cpos)});
            break;
        } else if (spos != cpos) {
            nodes.emplace_back(Node{NodeType::Text, fmt.substr(cpos, spos - cpos)});
        }
        auto epos = fmt.find('}', spos + 2);
        if (epos == std::string::npos) {
            throwError(UserErrors::ParsingFailed, "Formatter", "Bad pattern in format string, '}' expected");
        }
        std::string pattern = fmt.substr(spos + 2, epos - spos - 2);
        auto iter = PatternMapping.find(pattern);
        if (iter == PatternMapping.end()) {
            std::string error = "Unknown pattern '" + pattern + "' in format string";
            throwError(UserErrors::ParsingFailed, "Formatter", error);
        }
        nodes.emplace_back(Node{iter->second, ""});
        cpos = epos + 1;
    }
    return nodes;
}

const char * EasyEvent::Formatter::getLevelString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_CRITICAL:
            return "crit";
        case LOG_LEVEL_ERROR:
            return "fail";
        case LOG_LEVEL_WARN:
            return "warn";
        case LOG_LEVEL_INFO:
            return "info";
        case LOG_LEVEL_DEBUG:
            return "dbug";
        default:
            return "disabled";
    }
}
