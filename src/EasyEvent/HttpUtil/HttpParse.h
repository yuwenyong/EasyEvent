//
// Created by yuwenyong.vincent on 2021/9/5.
//

#ifndef EASYEVENT_HTTPUTIL_HTTPPARSE_H
#define EASYEVENT_HTTPUTIL_HTTPPARSE_H

#include "EasyEvent/HttpUtil/UrlParse.h"
#include "EasyEvent/Common/Time.h"


namespace EasyEvent {

    enum class HttpInputErrors {
        MalformedHttpRequestLine = 1,
        MalformedHttpVersionInHttpRequestLine,
        EmptyHeaderLine,
        NoColonInHeaderLine,
        FirstHeaderLineStartWithWhitespace,
        ErrorParsingResponseStartLine,
        TransferEncodingAndContentLengthConflict,
        MultipleUnequalContentLength,
        ContentLengthInvalidType,
        ContentLengthTooLong,
        ChunkedBodyTooLarge,
        UnexpectedChunkEnds,
        ImproperlyTerminateChunkedRequest,
    };

    class EASY_EVENT_API HttpInputErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getHttpInputErrorCategory();

    inline std::error_code make_error_code(HttpInputErrors err) {
        return {static_cast<int>(err), getHttpInputErrorCategory()};
    }

    enum class HttpOutputErrors {
        WriteMoreDataThanContentLength = 1,
        WriteLessDataThanContentLength,
        NewlineInHeaders,
    };

    class EASY_EVENT_API HttpOutputErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getHttpOutputErrorCategory();

    inline std::error_code make_error_code(HttpOutputErrors err) {
        return {static_cast<int>(err), getHttpOutputErrorCategory()};
    }

    class HttpHeaders;

    class EASY_EVENT_API HttpFile {
    public:
        HttpFile(std::string fileName,
                 std::string contentType,
                 std::string body)
                : _fileName(std::move(fileName))
                , _contentType(std::move(contentType))
                , _body(std::move(body)) {

        }

        const std::string& getFileName() const {
            return _fileName;
        }

        const std::string& getContentType() const {
            return _contentType;
        }

        const std::string& getBody() const {
            return _body;
        }
    private:
        std::string _fileName;
        std::string _contentType;
        std::string _body;
    };

    using HttpFiles = std::map<std::string, std::vector<HttpFile>>;
    using HttpArguments = std::map<std::string, std::vector<std::string>>;

    class EASY_EVENT_API HttpParse {
    public:
        static std::string urlConcat(std::string url, const OrderedDict<std::string, std::string>& args);

        static std::string urlConcat(std::string url, const std::vector<StringPair>& args);

        static std::string urlConcat(std::string url, const std::map<std::string, std::string>& args);

        static std::string urlConcat(std::string url, const std::unordered_map<std::string, std::string>& args);

        static std::string urlConcat(std::string url, const std::multimap<std::string, std::string>& args);

        static std::string urlConcat(std::string url, const std::unordered_multimap<std::string, std::string>& args);

        static std::string urlConcat(std::string url, const std::map<std::string, StringVec>& args);

        static std::string urlConcat(std::string url, const std::unordered_map<std::string, StringVec>& args);

        static void parseBodyArguments(const std::string_view& contentType, const std::string& body,
                                       HttpArguments& arguments, HttpFiles& files, StringVec& errors,
                                       const HttpHeaders* headers=nullptr);

        static void parseMultipartFormData(std::string boundary, const std::string& data, HttpArguments& arguments,
                                           HttpFiles& files, StringVec& errors);

        static std::string formatTimestamp(Time ts);

        static std::string encodeUsernamePassword(const std::string& username, const std::string& password) {
            return username + ":" + password;
        }

        static std::tuple<std::string, std::optional<unsigned short>> splitHostAndPort(const std::string &netloc);

        static std::vector<StringPair> QStoQSL(const std::map<std::string, StringVec>& args) {
            std::vector<StringPair> res;
            for (auto &kv: args) {
                for (auto &v: kv.second) {
                    res.emplace_back(kv.first, v);
                }
            }
            return res;
        }

        static std::vector<StringPair> QStoQSL(const std::unordered_map<std::string, StringVec>& args) {
            std::vector<StringPair> res;
            for (auto &kv: args) {
                for (auto &v: kv.second) {
                    res.emplace_back(kv.first, v);
                }
            }
            return res;
        }

    private:
        static StringVec parseParam(std::string_view s);

        static std::tuple<std::string, StringMap> parseHeader(const std::string& line);

        static std::string encodeHeader(const std::string& key, const StringMap& pdict);
    };

    class EASY_EVENT_API RequestStartLine {
    public:
        RequestStartLine() = default;

        RequestStartLine(std::string method, std::string path, std::string version)
            : _method(std::move(method))
            , _path(std::move(path))
            , _version(std::move(version)) {

        }

        void setMethod(std::string method) {
            _method = std::move(method);
        }

        const std::string& getMethod() const {
            return _method;
        }

        void setPath(std::string path) {
            _path = std::move(path);
        }

        const std::string& getPath() const {
            return _path;
        }

        void setVersion(std::string version) {
            _version = std::move(version);
        }

        const std::string& getVersion() const {
            return _version;
        }

        static RequestStartLine parse(const std::string_view& line);
    private:
        std::string _method;
        std::string _path;
        std::string _version;
    };


    class EASY_EVENT_API ResponseStartLine {
    public:
        ResponseStartLine() = default;

        ResponseStartLine(std::string version, int code, std::string reason)
            : _version(std::move(version))
            , _code(code)
            , _reason(std::move(reason)) {

        }

        void setVersion(std::string version) {
            _version = std::move(version);
        }

        const std::string& getVersion() const {
            return _version;
        }

        void setCode(int code) {
            _code = code;
        }

        int getCode() const {
            return _code;
        }

        void setReason(std::string reason) {
            _reason = std::move(reason);
        }

        const std::string& getReason() const {
            return _reason;
        }

        static ResponseStartLine parse(const std::string& line);
    private:
        std::string _version;
        int _code;
        std::string _reason;
    };

}

namespace std {

    template<>
    struct is_error_code_enum<EasyEvent::HttpInputErrors> : public true_type {
    };

    template<>
    struct is_error_code_enum<EasyEvent::HttpOutputErrors> : public true_type {
    };
}

#endif //EASYEVENT_HTTPUTIL_HTTPPARSE_H
