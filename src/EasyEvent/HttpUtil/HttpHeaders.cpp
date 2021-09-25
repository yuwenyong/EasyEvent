//
// Created by yuwenyong.vincent on 2021/9/5.
//

#include "EasyEvent/HttpUtil/HttpHeaders.h"
#include "EasyEvent/Common/StrUtil.h"


void EasyEvent::HttpHeaders::add(const std::string_view& name, const std::string_view& value) {
    std::string normName(name.data(), name.size());
    normalizeName(normName);
    _lastKey = normName;
    if (has(normName)) {
        _items[normName] += ',' + std::string(value.data(), value.size());
        _asList[normName].emplace_back(value);
    } else {
        (*this)[normName] = value;
    }
}

const StringVec * EasyEvent::HttpHeaders::getList(const std::string_view &name) const {
    std::string normName(name.data(), name.size());
    normalizeName(normName);
    auto iter = _asList.find(normName);
    return iter != _asList.end() ? &(iter->second) : nullptr;
}

void EasyEvent::HttpHeaders::parseLine(const std::string_view &line) {
    if (line.empty()) {
        throwError(HttpInputErrors::EmptyHeaderLine, "HttpHeaders");
    }
    if (std::isspace(line.front())) {
        if (_lastKey.empty()) {
            throwError(HttpInputErrors::FirstHeaderLineStartWithWhitespace, "HttpHeaders");
            auto &asList = _asList.at(_lastKey);
            Assert(!asList.empty());
            size_t pos = 0;
            for (; pos != line.size(); ++pos) {
                if (pos + 1 == line.size() || !std::isspace(line[pos + 1])) {
                    break;
                }
            }
            asList.back().append(line.begin() + (int)pos, line.end());
            _items.at(_lastKey).append(line.begin() + (int)pos, line.end());
        }
    } else {
        size_t pos = line.find(':');
        if (pos == std::string::npos) {
            throwError(HttpInputErrors::NoColonInHeaderLine, "HttpHeaders");
        }
        std::string_view name = line.substr(0, pos);
        std::string_view value(line.data() + pos + 1, line.size() - (pos + 1));
        StrUtil::trim(value);
        add(name, value);
    }
}

void EasyEvent::HttpHeaders::parseLines(const std::string_view &headers) {
    auto lines = StrUtil::splitLines(headers);
    for (auto &line: lines) {
        if (!line.empty()) {
            parseLine(line);
        }
    }
}

void EasyEvent::HttpHeaders::erase(const std::string_view &name) {
    std::string normName(name.data(), name.size());
    normalizeName(normName);
    if (has(normName)) {
        _items.erase(normName);
        _asList.erase(normName);
    }
}

std::string EasyEvent::HttpHeaders::get(const std::string_view &name, const std::string &defaultValue) const {
    std::string normName(name.data(), name.size());
    normalizeName(normName);
    auto iter = _items.find(normName);
    if (iter != _items.end()) {
        return iter->second;
    } else {
        return defaultValue;
    }
}

void EasyEvent::HttpHeaders::normalizeName(std::string& name) {
    bool firstLetter = true;
    for (char &c: name) {
        if (c == '-') {
            firstLetter = true;
        } else if (firstLetter) {
            c = (char)std::toupper(c);
            firstLetter = false;
        } else {
            c = (char)std::tolower(c);
        }
    }
}

std::ostream& EasyEvent::operator<<(std::ostream &os, const HttpHeaders &headers) {
    StringVec lines;
    headers.getAll([&lines](const std::string &name, const std::string &value) {
        lines.emplace_back(name + ": " + value + "\n");
    });
    os << StrUtil::join(lines, "");
    return os;
}

const char* EasyEvent::HttpHeaderFields::XForwardedFor = "X-Forwarded-For";
const char* EasyEvent::HttpHeaderFields::XRealIp = "X-Real-Ip";
const char* EasyEvent::HttpHeaderFields::XScheme = "X-Scheme";
const char* EasyEvent::HttpHeaderFields::XForwardedProto = "X-Forwarded-Proto";

const char* EasyEvent::HttpHeaderFields::ContentEncoding = "Content-Encoding";
const char* EasyEvent::HttpHeaderFields::Host = "Host";
const char* EasyEvent::HttpHeaderFields::ContentType = "Content-Type";