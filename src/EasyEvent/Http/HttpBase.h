//
// Created by yuwenyong.vincent on 2021/7/26.
//

#ifndef EASYEVENT_HTTP_HTTPBASE_H
#define EASYEVENT_HTTP_HTTPBASE_H

#include "EasyEvent/Event/EventBase.h"
#include "EasyEvent/Common/Time.h"
#include "EasyEvent/Compress/GzipDecompressor.h"
#include "EasyEvent/Event/Connection.h"
#include "EasyEvent/HttpUtil/HttpHeaders.h"
#include "EasyEvent/HttpUtil/HttpParse.h"

namespace EasyEvent {

    enum class HttpConnectionErrors {
        ReadHttpHeaderTimeout = 1,
        ReadHttpBodyTimeout = 2,
    };

    class EASY_EVENT_API HttpConnectionErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getHttpConnectionErrorCategory();

    inline std::error_code make_error_code(HttpConnectionErrors err) {
        return {static_cast<int>(err), getHttpConnectionErrorCategory()};
    }

}


namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::HttpConnectionErrors>: public true_type {};

}

#endif //EASYEVENT_HTTP_HTTPBASE_H
