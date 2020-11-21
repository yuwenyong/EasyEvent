//
// Created by yuwenyong on 2020/11/21.
//

#include "EasyEvent/Logging/LogCommon.h"


const char * EasyEvent::LoggingErrorCategory::name() const noexcept {
    return "logging error";
}

std::string EasyEvent::LoggingErrorCategory::message(int ev) const {
    switch (static_cast<LoggingErrorCode>(ev)) {
        case LoggingErrorCode::LoggerAlreadyRegistered:
            return "logger already registered";
        default:
            return "(unrecognized error)";
    }
}

const std::error_category & EasyEvent::getLoggingErrorCategory() {
    static const EasyEvent::LoggingErrorCategory errCategory{};
    return errCategory;
}