//
// Created by yuwenyong.vincent on 2021/9/5.
//

#ifndef EASYEVENT_HTTPUTIL_HTTPHEADERS_H
#define EASYEVENT_HTTPUTIL_HTTPHEADERS_H

#include "EasyEvent/HttpUtil/HttpParse.h"

namespace EasyEvent {

    class EASY_EVENT_API HttpHeaders {
    public:
        class HttpHeadersSetter {
        public:
            HttpHeadersSetter(std::string *value, StringVec* values)
            : _value(value)
                    , _values(values) {
            }

            HttpHeadersSetter& operator=(const std::string &value) {
                *_value = value;
                *_values = {value, };
                return *this;
            }

            HttpHeadersSetter& operator=(std::string&& value) {
                *_value = std::move(value);
                *_values = {*_value, };
                return *this;
            }

            HttpHeadersSetter& operator=(const std::string_view &value) {
                *this = std::string(value.data(), value.size());
                return *this;
            }

            explicit operator std::string_view () const {
                return *_value;
            }
        protected:
            std::string *_value{nullptr};
            StringVec *_values{nullptr};
        };

        HttpHeaders() = default;

        HttpHeaders(std::initializer_list<StringPair> nameValues) {
            update(nameValues);
        }

        explicit HttpHeaders(const StringMap &nameValues) {
            update(nameValues);
        }

        void add(const std::string_view& name, const std::string_view& value);

        const StringVec* getList(const std::string_view& name) const;

        void getAll(const std::function<void (const std::string&, const std::string&)>& callback) const {
            for (auto &name: _asList) {
                for (auto &value: name.second) {
                    callback(name.first, value);
                }
            }
        }

        void parseLine(const std::string_view& line);

        void parseLines(const std::string_view& headers);

        HttpHeadersSetter operator[](const std::string_view& name) {
            std::string normName(name.data(), name.size());
            normalizeName(normName);
            return HttpHeadersSetter(&_items[normName], &_asList[normName]);
        }

        bool has(const std::string_view& name) const {
            std::string normName(name.data(), name.size());
            normalizeName(normName);
            return _items.find(normName) != _items.end();
        }

        const std::string& at(const std::string_view& name) const {
            std::string normName(name.data(), name.size());
            normalizeName(normName);
            return _items.at(normName);
        }

        void erase(const std::string_view &name);

        std::string get(const std::string_view &name, const std::string &defaultValue="") const;

        void update(const std::vector<StringPair> &nameValues) {
            for(auto &nameValue: nameValues) {
                (*this)[nameValue.first] = nameValue.second;
            }
        }

        void update(std::initializer_list<StringPair> nameValues) {
            for(auto &nameValue: nameValues) {
                (*this)[nameValue.first] = nameValue.second;
            }
        }

        void update(const StringMap &nameValues) {
            for(auto &nameValue: nameValues) {
                (*this)[nameValue.first] = nameValue.second;
            }
        }

        void clear() {
            _items.clear();
            _asList.clear();
        }

        const StringMap& items() const {
            return _items;
        }

        static HttpHeaders parse(const std::string_view& headers) {
            HttpHeaders h;
            h.parseLines(headers);
            return h;
        }

        static std::unique_ptr<HttpHeaders> parseAsUnique(const std::string_view& headers) {
            auto h = std::make_unique<HttpHeaders>();
            h->parseLines(headers);
            return h;
        }

        static std::unique_ptr<HttpHeaders> parseAsShared(const std::string_view& headers) {
            auto h = std::make_unique<HttpHeaders>();
            h->parseLines(headers);
            return h;
        }
    private:
        static void normalizeName(std::string& name);

        StringMap _items;
        StringVecMap _asList;
        std::string _lastKey;
    };

    using HttpHeadersPtr = std::shared_ptr<HttpHeaders>;
    using HttpHeadersUPtr = std::unique_ptr<HttpHeaders>;

    EASY_EVENT_API std::ostream& operator<<(std::ostream &os, const HttpHeaders &headers);

    class EASY_EVENT_API HttpHeaderFields {
    public:
        static const char* XForwardedFor;
        static const char* XRealIp;
        static const char* XScheme;
        static const char* XForwardedProto;

        static const char* ContentEncoding;
        static const char* Host;
        static const char* ContentType;
    };

}

#endif //EASYEVENT_HTTPUTIL_HTTPHEADERS_H
