//
// Created by yuwenyong on 2020/10/26.
//

#include "EasyEvent/Common/Errors.h"

const char * EasyEvent::UserErrorCategory::name() const noexcept {
    return "user error";
}

std::string EasyEvent::UserErrorCategory::message(int ev) const {
    switch (static_cast<UserErrorCode>(ev)) {
        case UserErrorCode::InvalidArgument:
            return "invalid argument";
        case UserErrorCode::NotFound:
            return "not found";
        case UserErrorCode::AlreadyRegistered:
            return "already registered";
        default:
            return "(unrecognized error)";
    }
}

const std::error_category& EasyEvent::getUserErrorCategory() {
    static const EasyEvent::UserErrorCategory errCategory{};
    return errCategory;
}

void EasyEvent::doThrowError(const std::error_code &err) {
    std::system_error e(err);
    throw e;
}

void EasyEvent::doThrowError(const std::error_code &err, const char *location) {
    std::string whatMsg = location;
    whatMsg += ": ";
    whatMsg += err.message();
    std::system_error e(err, whatMsg);
    throw e;
}