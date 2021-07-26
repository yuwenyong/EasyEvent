//
// Created by yuwenyong.vincent on 2021/7/26.
//

#include "EasyEvent/Http/HttpUtil.h"
#include "EasyEvent/Common/StrUtil.h"

void EasyEvent::HttpHeaders::add(const std::string& name, const std::string& value) {
    std::string temp;
    const std::string* normName = normalizeName(name, temp);
    _lastKey = *normName;
    if (has(*normName)) {
        _items[*normName] += ',' + value;
        _asList[*normName].emplace_back(value);
    } else {
        (*this)[*normName] = value;
    }
}

const StringVec * EasyEvent::HttpHeaders::getList(const std::string &name) const {
    std::string temp;
    const std::string* normName = normalizeName(name, temp);
    auto iter = _asList.find(*normName);
    return iter != _asList.end() ? &(iter->second) : nullptr;
}

void EasyEvent::HttpHeaders::parseLine(const std::string_view &line) {
    if (line.empty()) {
        throwError(HttpHeaderErrors::EmptyHeaderLine, "HttpHeaders");
    }
    if (std::isspace(line.front())) {
        if (_lastKey.empty()) {
            throwError(HttpHeaderErrors::FirstHeaderLineStartWithWhitespace, "HttpHeaders");
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
            throwError(HttpHeaderErrors::NoColonInHeaderLine, "HttpHeaders");
        }
        std::string_view name = line.substr(0, pos);
        std::string_view value(line.data() + pos + 1, line.size() - (pos + 1));
        StrUtil::trim(value);
        add(std::string(name.data(), name.size()), std::string(value.data(), value.size()));
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

void EasyEvent::HttpHeaders::erase(const std::string &name) {
    std::string temp;
    const std::string* normName = normalizeName(name, temp);
    if (has(*normName)) {
        _items.erase(*normName);
        _asList.erase(*normName);
    }
}

std::string EasyEvent::HttpHeaders::get(const std::string &name, const std::string &defaultValue) const {
    std::string temp;
    const std::string* normName = normalizeName(name, temp);
    auto iter = _items.find(*normName);
    if (iter != _items.end()) {
        return iter->second;
    } else {
        return defaultValue;
    }
}

const std::string* EasyEvent::HttpHeaders::normalizeName(const std::string& origName, std::string& normName) {
    bool normalized = true;
    bool firstLetter = true;
    char c;
    std::string::size_type i = 0;
    for (; i != origName.size(); ++i) {
        c = origName[i];
        if (c == '-') {
            firstLetter = true;
        } else if (firstLetter) {
            if (!std::isupper(c)) {
                normalized = false;
                break;
            }
            firstLetter = false;
        } else {
            if (std::isupper(c)) {
                normalized = false;
                break;
            }
        }
    }
    if (normalized) {
        return &origName;
    }
    normName.reserve(origName.size());
    normName.assign(origName.begin(), origName.begin() + (int)i);
    for (; i != origName.size(); ++i) {
        c = origName[i];
        if (c == '-') {
            normName.push_back(c);
            firstLetter = true;
        } else if (firstLetter) {
            normName.push_back((char)std::toupper(c));
            firstLetter = false;
        } else {
            normName.push_back((char)std::tolower(c));
        }
    }
    return &normName;
}

std::ostream& EasyEvent::operator<<(std::ostream &os, const HttpHeaders &headers) {
    StringVec lines;
    headers.getAll([&lines](const std::string &name, const std::string &value) {
        lines.emplace_back(name + ": " + value + "\n");
    });
    os << StrUtil::join(lines, "");
    return os;
}