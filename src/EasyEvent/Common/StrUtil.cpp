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
    for (size_t pos = 0; pos != input.size(); ++pos) {
        if (!isSpace(input[pos])) {
            return input.substr(pos);
        }
    }
    return {};
}

std::string_view EasyEvent::StrUtil::trimRightCopy(const std::string_view &input, const std::function<bool(char)>& isSpace) {
    for (ssize_t pos = (ssize_t)input.size() - 1; pos >= 0; --pos) {
        if (!isSpace(input[(size_t)pos])) {
            return input.substr(0, (size_t)pos + 1);
        }
    }
    return {};
}

std::string_view EasyEvent::StrUtil::trimCopy(const std::string_view &input, const std::function<bool(char)> &isSpace) {
    ssize_t beg = 0, end = (ssize_t)input.size() - 1;
    for (; beg != (ssize_t)input.size(); ++beg) {
        if (!isSpace(input[(size_t)beg])) {
            break;
        }
    }
    for (; end >= beg; --end) {
        if (!isSpace(input[(size_t)end])) {
            end += 1;
            break;
        }
    }
    if (end > beg) {
        return input.substr((size_t)beg, (size_t)(end - beg));
    } else {
        return {};
    }
}

std::string EasyEvent::StrUtil::trimLeftCopy(const std::string &input, const std::function<bool(char)> &isSpace) {
    for (size_t pos = 0; pos != input.size(); ++pos) {
        if (!isSpace(input[pos])) {
            return input.substr(pos);
        }
    }
    return {};
}

std::string EasyEvent::StrUtil::trimRightCopy(const std::string &input, const std::function<bool(char)> &isSpace) {
    for (ssize_t pos = (ssize_t)input.size() - 1; pos >= 0; --pos) {
        if (!isSpace(input[(size_t)pos])) {
            return input.substr(0, (size_t)pos + 1);
        }
    }
    return {};
}

std::string EasyEvent::StrUtil::trimCopy(const std::string &input, const std::function<bool(char)> &isSpace) {
    ssize_t beg = 0, end = (ssize_t)input.size() - 1;
    for (; beg != (ssize_t)input.size(); ++beg) {
        if (!isSpace(input[(size_t)beg])) {
            break;
        }
    }
    for (; end >= beg; --end) {
        if (!isSpace(input[(size_t)end])) {
            end += 1;
            break;
        }
    }
    if (end > beg) {
        return input.substr((size_t)beg, (size_t)end - (size_t)beg);
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
        std::string_view temp(&(*beg), (size_t)(end - beg));
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

bool EasyEvent::StrUtil::startsWith(const std::string_view &input, const std::string_view &test) {
    auto inputIter = input.begin();
    auto inputEnd = input.end();
    auto testIter = test.begin();
    auto testEnd = test.end();

    for (; inputIter != inputEnd && testIter != testEnd; ++inputIter, ++testIter) {
        if (*inputIter != *testIter) {
            return false;
        }
    }
    return testIter == testEnd;
}

bool EasyEvent::StrUtil::endsWith(const std::string_view &input, const std::string_view &test) {
    auto inputIter = input.rbegin();
    auto inputEnd = input.rend();
    auto testIter = test.rbegin();
    auto testEnd = test.rend();

    for (; inputIter != inputEnd && testIter != testEnd; ++inputIter, ++testIter) {
        if (*inputIter != *testIter) {
            return false;
        }
    }
    return testIter == testEnd;
}

bool EasyEvent::StrUtil::contains(const std::string_view &input, const std::string_view &test) {
    if (test.empty()) {
        return true;
    }
    if (test.size() > input.size()) {
        return false;
    }
    const size_t len = input.size() - test.size();
    for (size_t i = 0; i <= len; ++i) {
        if (memcmp(input.data() + i, test.data(), test.size()) == 0) {
            return true;
        }
    }
    return false;
}

void EasyEvent::StrUtil::replaceAll(std::string &s, const std::string_view &search, const std::string_view &replace) {
    size_t pos = 0;
    const char* ptr;
    while (pos < s.size()) {
        ptr = find(s.data() + pos, s.size() - pos, search.data(), search.size());
        if (ptr == nullptr) {
            break;
        }
        pos = (size_t)(ptr - s.data());
        s.replace(pos, search.size(), replace.data(), replace.size());
        pos += replace.size();
    }
}

size_t EasyEvent::StrUtil::count(const std::string_view &s, char c, size_t start, size_t len) {
    size_t e = len ? std::min(start + len, s.size()) : s.size();
    if (start >= e) {
        return 0;
    }
    size_t i = start, r = 0;
    while ((i = s.find(c, i)) != std::string_view::npos) {
        if (i + 1 > e) {
            break;
        }
        ++i;
        ++r;
    }
    return r;
}

size_t EasyEvent::StrUtil::count(const std::string_view &s, const std::string_view &sub, size_t start, size_t len) {
    size_t e = len ? std::min(start + len, s.size()) : s.size();
    if (start >= e) {
        return 0;
    }
    size_t i = start, r = 0, m = sub.size();
    while ((i = s.find(sub, i)) != std::string_view::npos) {
        if (i + m > e) {
            break;
        }
        i += m;
        ++r;
    }
    return r;
}

size_t EasyEvent::StrUtil::count(const std::string_view &s, const std::function<bool(char)>& pred) {
    size_t r = 0;
    for (char c: s) {
        if (pred(c)) {
            ++r;
        }
    }
    return r;
}

std::tuple<std::string, std::string, std::string> EasyEvent::StrUtil::partition(
        const std::string &s, const std::string &sep) {
    auto pos = s.find(sep);
    if (pos == std::string::npos) {
        return std::make_tuple(s, "", "");
    } else {
        auto offset = pos + sep.size();
        std::string before(s.data(), pos), after(s.data() + offset, s.size() - offset);
        return std::make_tuple(before, sep, after);
    }
}

std::tuple<std::string_view, std::string_view, std::string_view> EasyEvent::StrUtil::partition(
        const std::string_view &s, const std::string_view &sep) {
    auto pos = s.find(sep);
    if (pos == std::string_view::npos) {
        return std::make_tuple(s, std::string_view(), std::string_view());
    } else {
        auto offset = pos + sep.size();
        std::string_view before(s.data(), pos), after(s.data() + offset, s.size() - offset);
        return std::make_tuple(before, sep, after);
    }
}

std::tuple<std::string, std::string, std::string> EasyEvent::StrUtil::rpartition(
        const std::string &s, const std::string &sep) {
    auto pos = s.rfind(sep);
    if (pos == std::string::npos) {
        return std::make_tuple("", "", s);
    } else {
        auto offset = pos + sep.size();
        std::string before(s.data(), pos), after(s.data() + offset, s.size() - offset);
        return std::make_tuple(before, sep, after);
    }
}

std::tuple<std::string_view, std::string_view, std::string_view> EasyEvent::StrUtil::rpartition(
        const std::string_view &s, const std::string_view &sep) {
    auto pos = s.rfind(sep);
    if (pos == std::string_view::npos) {
        return std::make_tuple(std::string_view(), std::string_view(), s);
    } else {
        auto offset = pos + sep.size();
        std::string_view before(s.data(), pos), after(s.data() + offset, s.size() - offset);
        return std::make_tuple(before, sep, after);
    }
}
