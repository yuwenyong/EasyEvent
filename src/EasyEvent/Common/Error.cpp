//
// Created by yuwenyong on 2020/10/26.
//

#include "EasyEvent/Common/Error.h"

const char * EasyEvent::UserErrorCategory::name() const noexcept {
    return "user error";
}

std::string EasyEvent::UserErrorCategory::message(int ev) const {
    switch (static_cast<UserErrors>(ev)) {
        case UserErrors::InvalidArgument:
            return "invalid argument";
        case UserErrors::NotFound:
            return "not found";
        case UserErrors::NotSupported:
            return "not supported";
        case UserErrors::OperationCanceled:
            return "operation canceled";
        case UserErrors::NotConvertible:
            return "not convertible";
        case UserErrors::OutOfRange:
            return "out of range";
        case UserErrors::UnexpectedBehaviour:
            return "unexpected behaviour";
        case UserErrors::BadValue:
            return "bad value";
        case UserErrors::ParsingFailed:
            return "parsing failed";
        case UserErrors::OperationForbidden:
            return "operation forbidden";
        case UserErrors::PreconditionFailed:
            return "precondition failed";
        case UserErrors::AlreadyExists:
            return "already exists";
        case UserErrors::DuplicateValues:
            return "duplicate values";
        case UserErrors::BadState:
            return "bad state";
        case UserErrors::ArgumentRequired:
            return "argument required";
        default:
            return "(unrecognized error)";
    }
}

const std::error_category& EasyEvent::getUserErrorCategory() {
    static const UserErrorCategory errCategory{};
    return errCategory;
}

void EasyEvent::doThrowError(const std::error_code &err) {
    throw std::system_error(err);
}

void EasyEvent::doThrowError(const std::error_code &err, const char *location) {
    std::string whatMsg = location;
    whatMsg += ": ";
    whatMsg += err.message();
    throw std::system_error(err, whatMsg);
}

void EasyEvent::doThrowError(const std::error_code &err, const char *location, const char *what) {
    std::string whatMsg = location;
    whatMsg += ": ";
    whatMsg += what;
    throw std::system_error(err, whatMsg);
}