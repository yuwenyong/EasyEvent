//
// Created by yuwenyong.vincent on 2021/7/26.
//

#ifndef EASYEVENT_HTTP_HTTPUTIL_H
#define EASYEVENT_HTTP_HTTPUTIL_H

#include "EasyEvent/Http/HttpBase.h"


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

            explicit operator std::string_view () const {
                return *_value;
            }
        protected:
            std::string *_value{nullptr};
            StringVec *_values{nullptr};
        };

        HttpHeaders() = default;

        HttpHeaders(std::initializer_list<StringPair> nameValues) {
            for(auto &nameValue: nameValues) {
                (*this)[nameValue.first] = nameValue.second;
            }
        }

        explicit HttpHeaders(const StringMap &nameValues) {
            update(nameValues);
        }

        void add(const std::string& name, const std::string& value);

        const StringVec* getList(const std::string& name) const;

        void getAll(const std::function<void (const std::string&, const std::string&)>& callback) const {
            for (auto &name: _asList) {
                for (auto &value: name.second) {
                    callback(name.first, value);
                }
            }
        }

        void parseLine(const std::string_view& line);

        void parseLines(const std::string_view& headers);

        HttpHeadersSetter operator[](const std::string& name) {
            std::string temp;
            const std::string* normName = normalizeName(name, temp);
            return HttpHeadersSetter(&_items[*normName], &_asList[*normName]);
        }

        bool has(const std::string& name) const {
            std::string temp;
            const std::string* normName = normalizeName(name, temp);
            return _items.find(*normName) != _items.end();
        }

        const std::string& at(const std::string& name) const {
            std::string temp;
            const std::string* normName = normalizeName(name, temp);
            return _items.at(*normName);
        }

        void erase(const std::string &name);

        std::string get(const std::string &name, const std::string &defaultValue="") const;

        void update(const std::vector<StringPair> &nameValues) {
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

        static std::shared_ptr<HttpHeaders> parse(const std::string_view &headers) {
            auto h = std::make_shared<HttpHeaders>();
            h->parseLines(headers);
            return h;
        }
    private:
        static const std::string* normalizeName(const std::string& origName, std::string& normName);


        StringMap _items;
        StringVecMap _asList;
        std::string _lastKey;
    };

    using HttpHeadersPtr = std::shared_ptr<HttpHeaders>;

    EASY_EVENT_API std::ostream& operator<<(std::ostream &os, const HttpHeaders &headers);
}


#endif //EASYEVENT_HTTP_HTTPUTIL_H
