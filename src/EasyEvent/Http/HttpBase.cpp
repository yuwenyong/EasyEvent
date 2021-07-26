//
// Created by yuwenyong.vincent on 2021/7/26.
//

#include "EasyEvent/Http/HttpBase.h"

const char * EasyEvent::HttpHeaderErrorCategory::name() const noexcept {
    return "http header error";
}

std::string EasyEvent::HttpHeaderErrorCategory::message(int ev) const {
    switch (static_cast<HttpHeaderErrors>(ev)) {
        case HttpHeaderErrors::EmptyHeaderLine:
            return "empty header line";
        case HttpHeaderErrors::NoColonInHeaderLine:
            return "no colon in header line";
        case HttpHeaderErrors::FirstHeaderLineStartWithWhitespace:
            return "first header line start with whitespace";
        default:
            return "unknown http header error";
    }
}

const std::error_category& EasyEvent::getHttpHeaderErrorCategory() {
    static const HttpHeaderErrorCategory errCategory{};
    return errCategory;
}


const EasyEvent::HttpConnectionOptions EasyEvent::HttpConnectionOptions::Default;