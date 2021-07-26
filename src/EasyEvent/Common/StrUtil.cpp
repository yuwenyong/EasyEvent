//
// Created by yuwenyong on 2020/12/13.
//

#include "EasyEvent/Common/StrUtil.h"


const char * EasyEvent::StrUtil::find(const char *s1, size_t len1, const char *s2, size_t len2) {
    if (len2 > len1) {
        return nullptr;
    }
    const size_t len = len1 - len2;
    for (size_t i = 0; i <= len; ++i) {
        if (memcmp(s1 + i, s2, len2) == 0) {
            return s1 + i;
        }
    }
    return nullptr;
}

std::string_view EasyEvent::StrUtil::trimLeftCopy(const std::string_view &input, const std::function<bool(char)>& isSpace) {
    for (ssize_t pos = 0; pos != (ssize_t)input.size(); ++pos) {
        if (!isSpace(input[pos])) {
            return input.substr(pos);
        }
    }
    return {};
}

std::string_view EasyEvent::StrUtil::trimRightCopy(const std::string_view &input, const std::function<bool(char)>& isSpace) {
    for (ssize_t pos = (ssize_t)input.size() - 1; pos >= 0; --pos) {
        if (!isSpace(input[pos])) {
            return input.substr(0, pos + 1);
        }
    }
    return {};
}

std::string_view EasyEvent::StrUtil::trimCopy(const std::string_view &input, const std::function<bool(char)> &isSpace) {
    ssize_t beg = 0, end = (ssize_t)input.size() - 1;
    for (; beg != (ssize_t)input.size(); ++beg) {
        if (!isSpace(input[beg])) {
            break;
        }
    }
    for (; end >= beg; --end) {
        if (!isSpace(input[end])) {
            end += 1;
            break;
        }
    }
    if (end > beg) {
        return input.substr(beg, end - beg);
    } else {
        return {};
    }
}

std::string EasyEvent::StrUtil::trimLeftCopy(const std::string &input, const std::function<bool(char)> &isSpace) {
    for (ssize_t pos = 0; pos != (ssize_t)input.size(); ++pos) {
        if (!isSpace(input[pos])) {
            return input.substr(pos);
        }
    }
    return {};
}

std::string EasyEvent::StrUtil::trimRightCopy(const std::string &input, const std::function<bool(char)> &isSpace) {
    for (ssize_t pos = (ssize_t)input.size() - 1; pos >= 0; --pos) {
        if (!isSpace(input[pos])) {
            return input.substr(0, pos + 1);
        }
    }
    return {};
}

std::string EasyEvent::StrUtil::trimCopy(const std::string &input, const std::function<bool(char)> &isSpace) {
    ssize_t beg = 0, end = (ssize_t)input.size() - 1;
    for (; beg != (ssize_t)input.size(); ++beg) {
        if (!isSpace(input[beg])) {
            break;
        }
    }
    for (; end >= beg; --end) {
        if (!isSpace(input[end])) {
            end += 1;
            break;
        }
    }
    if (end > beg) {
        return input.substr(beg, end - beg);
    } else {
        return {};
    }
}

StringViewVec EasyEvent::StrUtil::split(const std::string_view &s, char delim, bool keepEmpty) {
    StringViewVec result;
    std::string_view::size_type beg = 0, end;
    while (true) {
        end = s.find(delim, beg);
        std::string_view temp;
        if (end == std::string_view::npos) {
            temp = s.substr(beg);
        } else {
            temp = s.substr(beg, end - beg);
        }
        if (keepEmpty || !temp.empty()) {
            result.push_back(temp);
        }
        if (end == std::string_view::npos) {
            break;
        }
        beg = end + 1;
    }
    return result;
}

StringVec EasyEvent::StrUtil::split(const std::string &s, char delim, bool keepEmpty) {
    StringVec result;
    std::string::size_type beg = 0, end;
    while (true) {
        end = s.find(delim, beg);
        std::string temp;
        if (end == std::string::npos) {
            temp = s.substr(beg);
        } else {
            temp = s.substr(beg, end - beg);
        }
        if (keepEmpty || !temp.empty()) {
            result.push_back(temp);
        }
        if (end == std::string::npos) {
            break;
        }
        beg = end + 1;
    }
    return result;
}

StringViewVec EasyEvent::StrUtil::split(const std::string_view &s, const std::string_view &delim, bool keepEmpty) {
    StringViewVec result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string_view ::const_iterator beg = s.begin(), end;
    while (true) {
        end = std::search(beg, s.end(), delim.begin(), delim.end());
        std::string_view temp(&(*beg), end - beg);
        if (keepEmpty || !temp.empty()) {
            result.push_back(temp);
        }
        if (end == s.end()) {
            break;
        }
        beg = end + (std::ptrdiff_t)delim.size();
    }
    return result;
}

StringVec EasyEvent::StrUtil::split(const std::string &s, const std::string_view &delim, bool keepEmpty) {
    StringVec result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string::const_iterator beg = s.begin(), end;
    while (true) {
        end = std::search(beg, s.end(), delim.begin(), delim.end());
        std::string temp(beg, end);
        if (keepEmpty || !temp.empty()) {
            result.push_back(temp);
        }
        if (end == s.end()) {
            break;
        }
        beg = end + (std::ptrdiff_t)delim.size();
    }
    return result;
}

StringViewVec EasyEvent::StrUtil::splitLines(const std::string_view &s, bool keepends) {
    StringViewVec result;
    size_t i = 0, j = 0, length = s.size();
    while (i < length) {
        size_t eol;
        while (i < length && s[i] != '\r' && s[i] != '\n') {
            ++i;
        }
        eol = i;
        if (i < length) {
            if (s[i] == '\r' && i + 1 < length && s[i + 1] == '\n') {
                i += 2;
            } else {
                ++i;
            }
            if (keepends) {
                eol = i;
            }
        }
        result.push_back(s.substr(j, eol - j));
        j = i;
    }
    return result;
}

StringVec EasyEvent::StrUtil::splitLines(const std::string &s, bool keepends) {
    StringVec result;
    size_t i = 0, j = 0, length = s.size();
    while (i < length) {
        size_t eol;
        while (i < length && s[i] != '\r' && s[i] != '\n') {
            ++i;
        }
        eol = i;
        if (i < length) {
            if (s[i] == '\r' && i + 1 < length && s[i + 1] == '\n') {
                i += 2;
            } else {
                ++i;
            }
            if (keepends) {
                eol = i;
            }
        }
        result.push_back(s.substr(j, eol - j));
        j = i;
    }
    return result;
}

std::string EasyEvent::StrUtil::join(const StringViewVec &input, const std::string_view &separator) {
    if (input.empty()) {
        return {};
    }
    std::string result;
    size_t length = 0;
    for (auto &s: input) {
        length += s.size();
    }
    length += (input.size() - 1) * separator.size();
    result.reserve(length);
    for (auto &s: input) {
        if (!result.empty()) {
            result.append(separator.begin(), separator.end());
        }
        result.append(s.begin(), s.end());
    }
    return result;
}

std::string EasyEvent::StrUtil::join(const StringVec &input, const std::string_view &separator) {
    if (input.empty()) {
        return {};
    }
    std::string result;
    size_t length = 0;
    for (auto &s: input) {
        length += s.size();
    }
    length += (input.size() - 1) * separator.size();
    result.reserve(length);
    for (auto &s: input) {
        if (!result.empty()) {
            result.append(separator.begin(), separator.end());
        }
        result.append(s);
    }
    return result;
}

