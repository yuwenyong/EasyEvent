//
// Created by yuwenyong on 2020/12/13.
//

#ifndef EASYEVENT_COMMON_STRUTIL_H
#define EASYEVENT_COMMON_STRUTIL_H

#include "EasyEvent/Common/Config.h"


namespace EasyEvent {

    class EASY_EVENT_API StrUtil {
    public:
        static const char* find(const char* s1, size_t len1, const char* s2, size_t len2);

        static const char* find(const char* s1, const char* s2, size_t len2) {
            return find(s1, strlen(s1), s2, len2);
        }

        static const char* find(const char* s1, size_t len1, const char* s2) {
            return find(s1, len1, s2, strlen(s2));
        }

        static const char* find(const char* s1, const char* s2) {
            return find(s1, strlen(s1), s2, strlen(s2));
        }

        static std::string_view trimLeftCopy(const std::string_view& input, const std::function<bool (char)> &isSpace);

        static void trimLeft(std::string_view& input, const std::function<bool (char)> &isSpace) {
            input = trimLeftCopy(input, isSpace);
        }

        static std::string_view trimLeftCopy(const std::string_view& input) {
            return trimLeftCopy(input, [](char c) { return (bool)std::isspace(c); });
        }

        static void trimLeft(std::string_view& input) {
            input = trimLeftCopy(input);
        }

        static std::string_view trimRightCopy(const std::string_view& input, const std::function<bool (char)> &isSpace);

        static void trimRight(std::string_view& input, const std::function<bool (char)> &isSpace) {
            input = trimRightCopy(input, isSpace);
        }

        static std::string_view trimRightCopy(const std::string_view& input) {
            return trimRightCopy(input, [](char c) { return (bool)std::isspace(c); });
        }

        static void trimRight(std::string_view& input) {
            input = trimRightCopy(input);
        }

        static std::string_view trimCopy(const std::string_view& input, const std::function<bool (char)> &isSpace);

        static void trim(std::string_view& input, const std::function<bool (char)> &isSpace) {
            input = trimCopy(input, isSpace);
        }

        static std::string_view trimCopy(const std::string_view& input) {
            return trimCopy(input, [](char c) { return (bool)std::isspace(c); });
        }

        static void trim(std::string_view& input) {
            input = trimCopy(input);
        }

        static std::string trimLeftCopy(const std::string& input, const std::function<bool (char)> &isSpace);

        static void trimLeft(std::string& input, const std::function<bool (char)> &isSpace) {
            input = trimLeftCopy(input, isSpace);
        }

        static std::string trimLeftCopy(const std::string& input) {
            return trimLeftCopy(input, [](char c) { return (bool)std::isspace(c); });
        }

        static void trimLeft(std::string& input) {
            input = trimLeftCopy(input);
        }

        static std::string trimRightCopy(const std::string& input, const std::function<bool (char)> &isSpace);

        static void trimRight(std::string& input, const std::function<bool (char)> &isSpace) {
            input = trimRightCopy(input, isSpace);
        }

        static std::string trimRightCopy(const std::string& input) {
            return trimRightCopy(input, [](char c) { return (bool)std::isspace(c); });
        }

        static void trimRight(std::string& input) {
            input = trimRightCopy(input);
        }

        static std::string trimCopy(const std::string& input, const std::function<bool (char)> &isSpace);

        static void trim(std::string& input, const std::function<bool (char)> &isSpace) {
            input = trimCopy(input, isSpace);
        }

        static std::string trimCopy(const std::string& input) {
            return trimCopy(input, [](char c) { return (bool)std::isspace(c); });
        }

        static void trim(std::string& input) {
            input = trimCopy(input);
        }

        static StringViewVec split(const std::string_view &s, char delim, bool keepEmpty=true);

        static StringVec split(const std::string &s, char delim, bool keepEmpty=true);

        static StringViewVec split(const std::string_view &s, const std::string_view &delim, bool keepEmpty=true);

        static StringVec split(const std::string &s, const std::string_view &delim, bool keepEmpty=true);

        static StringViewVec splitLines(const std::string_view &s, bool keepends=false);

        static StringVec splitLines(const std::string &s, bool keepends=false);

        static std::string join(const StringViewVec& input, const std::string_view& separator);

        static std::string join(const StringVec& input, const std::string_view& separator);
    };

}

#endif //EASYEVENT_COMMON_STRUTIL_H
