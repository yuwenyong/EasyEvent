//
// Created by yuwenyong.vincent on 2021/8/22.
//

#ifndef EASYEVENT_HTTPUTIL_URLPARSE_H
#define EASYEVENT_HTTPUTIL_URLPARSE_H

#include "EasyEvent/HttpUtil/HttpUtil.h"
#include "EasyEvent/Common/OrderedDict.h"


namespace EasyEvent {

    enum class UrlErrors {
        InvalidIPv6Url = 1,
        InvalidSeparator,
        BadQueryField,
    };

    class EASY_EVENT_API UrlErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getUrlErrorCategory();

    inline std::error_code make_error_code(UrlErrors err) {
        return {static_cast<int>(err), getUrlErrorCategory()};
    }

    class EASY_EVENT_API DefragResult {
    public:
        DefragResult() = default;

        DefragResult(std::string url,
                     std::string fragment)
             : _url(std::move(url))
             , _fragment(std::move(fragment)) {

        }

        const std::string& getFragment() const {
            return _fragment;
        }

        std::string getUrl() const {
            if (_fragment.empty()) {
                return _url;
            } else {
                return _url + '#' + _fragment;
            }
        }
    private:
        std::string _url;
        std::string _fragment;
    };

    class EASY_EVENT_API NetlocResult {
    public:
        virtual const std::string& getNetloc() const = 0;

        virtual ~NetlocResult() = default;

        std::string getUserName() const;

        std::string getPassword() const;

        std::string getHostName() const;

        int getPort() const;
    };

    class EASY_EVENT_API SplitResult: public NetlocResult {
    public:
        SplitResult() = default;

        SplitResult(std::string scheme,
                    std::string netloc,
                    std::string path,
                    std::string query,
                    std::string fragment)
            : _scheme(std::move(scheme))
            , _netloc(std::move(netloc))
            , _path(std::move(path))
            , _query(std::move(query))
            , _fragment(std::move(fragment)) {

        }

        const std::string& getScheme() const {
            return _scheme;
        }

        const std::string& getNetloc() const override {
            return _netloc;
        }

        const std::string& getPath() const {
            return _path;
        }

        const std::string& getQuery() const {
            return _query;
        }

        const std::string& getFragment() const {
            return _fragment;
        }

        std::string getUrl() const;
    private:
        std::string _scheme;
        std::string _netloc;
        std::string _path;
        std::string _query;
        std::string _fragment;
    };

    class EASY_EVENT_API ParseResult: public NetlocResult {
    public:
        ParseResult() = default;

        ParseResult(std::string scheme,
                    std::string netloc,
                    std::string path,
                    std::string params,
                    std::string query,
                    std::string fragment)
                : _scheme(std::move(scheme))
                , _netloc(std::move(netloc))
                , _path(std::move(path))
                , _params(std::move(params))
                , _query(std::move(query))
                , _fragment(std::move(fragment)) {

        }

        const std::string& getScheme() const {
            return _scheme;
        }

        const std::string& getNetloc() const override {
            return _netloc;
        }

        const std::string& getPath() const {
            return _path;
        }

        const std::string& getParams() const {
            return _params;
        }

        const std::string& getQuery() const {
            return _query;
        }

        const std::string& getFragment() const {
            return _fragment;
        }

        std::string getUrl() const;
    private:
        std::string _scheme;
        std::string _netloc;
        std::string _path;
        std::string _params;
        std::string _query;
        std::string _fragment;
    };


    class EASY_EVENT_API UrlParse {
    public:
        static ParseResult urlParse(std::string url, std::string schema="", bool allowFragments=true);

        static SplitResult urlSplit(std::string url, std::string schema="", bool allowFragments=true);

        static std::string urlUnparse(const ParseResult& components);

        static std::string urlUnsplit(const SplitResult& components);

        static std::string urlJoin(const std::string& base, const std::string& url, bool allowFragments=true);

        static DefragResult urlDefrag(const std::string& url);

        static std::string unquote(const std::string& s);

        static std::string unquotePlus(const std::string& s);

        static std::string quote(const std::string& s, const std::string& safe="/");

        static std::string quotePlus(const std::string& s, const std::string& safe="");

        static std::string urlEncode(const OrderedDict<std::string, std::string>& query, const std::string& safe="");

        static std::string urlEncode(const std::vector<StringPair>& query, const std::string& safe="");

        static std::string urlEncode(const std::map<std::string, std::string>& query, const std::string& safe="");

        static std::string urlEncode(const std::unordered_map<std::string, std::string>& query, const std::string& safe="");

        static std::string urlEncode(const std::multimap<std::string, std::string>& query, const std::string& safe="");

        static std::string urlEncode(const std::unordered_multimap<std::string, std::string>& query, const std::string& safe="");

        static std::string urlEncode(const std::map<std::string, StringVec>& query, const std::string& safe);

        static std::string urlEncode(const std::unordered_map<std::string, StringVec>& query, const std::string& safe);

        static std::map<std::string, StringVec> parseQS(const std::string &queryString, bool keepBlankValues=false,
                                                        bool strictParsing=false, const std::string& separator="&");

        static std::vector<StringPair> parseQSL(const std::string &queryString, bool keepBlankValues=false,
                                                bool strictParsing=false, const std::string& separator="&");

        static const StringViewSet UsesRelative;
        static const StringViewSet UsesNetloc;
        static const StringViewSet UsesParams;

        static const std::string_view SchemeChars;
        static const std::map<std::string_view, char> HexToChar;

        static const std::string_view AlwaysSafe;
        static const std::map<char, std::string_view> SafeMap;
    private:
        static std::tuple<std::string_view, std::string_view> splitParams(const std::string_view& url);

        static std::tuple<std::string_view, std::string_view> splitNetloc(const std::string_view& url, size_t start=0);
    };

}

namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::UrlErrors>: public true_type {};

}

#endif //EASYEVENT_HTTPUTIL_URLPARSE_H
