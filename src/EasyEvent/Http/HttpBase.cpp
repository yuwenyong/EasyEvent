//
// Created by yuwenyong.vincent on 2021/7/26.
//

#include "EasyEvent/Http/HttpBase.h"


const char * EasyEvent::HttpConnectionErrorCategory::name() const noexcept {
    return "http connection error";
}

std::string EasyEvent::HttpConnectionErrorCategory::message(int ev) const {
    switch (static_cast<HttpConnectionErrors>(ev)) {
        case HttpConnectionErrors::ReadHttpHeaderTimeout:
            return "read http header timeout";
        case HttpConnectionErrors::ReadHttpBodyTimeout:
            return "read http body timeout";
        default:
            return "unknown http connection error";
    }
}

const std::error_category& EasyEvent::getHttpConnectionErrorCategory() {
    static const HttpConnectionErrorCategory errCategory{};
    return errCategory;
}


