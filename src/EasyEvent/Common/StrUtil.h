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

        static bool startsWith(const std::string_view& input, const std::string_view& test);

        static bool endsWith(const std::string_view& input, const std::string_view& test);

        static bool contains(const std::string_view& input, const std::string_view& test);

        static void toLower(std::string& s) {
            std::transform(s.begin(), s.end(), s.begin(), [](char c) {
               return std::tolower(c);
            });
        }

        static void toUpper(std::string& s) {
            std::transform(s.begin(), s.end(), s.begin(), [](char c) {
                return std::tolower(c);
            });
        }

        static std::string toLowerCopy(const std::string_view& s) {
            std::string retVal{s.begin(), s.size()};
            toLower(retVal);
            return retVal;
        }

        static std::string toUpperCopy(const std::string_view& s) {
            std::string retVal{s.begin(), s.size()};
            toUpper(retVal);
            return retVal;
        }

        static void replaceAll(std::string& s, const std::string_view& search, const std::string_view& replace);

        static std::string replaceAllCopy(const std::string_view& s, const std::string_view& search,
                                          const std::string_view& replace) {
            std::string retVal{s.begin(), s.size()};
            replaceAll(retVal, search, replace);
            return retVal;
        }

        static size_t count(const std::string_view& s, char c, size_t start=0, size_t len=0);

        static size_t count(const std::string_view& s, const std::string_view& sub, size_t start=0, size_t len=0);

        static size_t count(const std::string_view& s, const std::function<bool (char)>& pred);

        static std::tuple<std::string, std::string, std::string> partition(
                const std::string& s, const std::string& sep);

        static std::tuple<std::string_view, std::string_view , std::string_view> partition(
                const std::string_view& s, const std::string_view& sep);

        static std::tuple<std::string, std::string, std::string> rpartition(
                const std::string& s, const std::string& sep);

        static std::tuple<std::string_view, std::string_view , std::string_view> rpartition(
                const std::string_view& s, const std::string_view& sep);

        template<typename T>
        static std::string toHexString(T val) {
            std::stringstream stream;
            stream << std::hex << val;
            return stream.str();
        }
    };

}

#endif //EASYEVENT_COMMON_STRUTIL_H
