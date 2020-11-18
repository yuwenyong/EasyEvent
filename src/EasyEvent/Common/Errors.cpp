//
// Created by yuwenyong on 2020/10/26.
//

#include "EasyEvent/Common/Errors.h"

const char * EasyEvent::CommonErrCategory::name() const noexcept {
    return "common";
}

std::string EasyEvent::CommonErrCategory::message(int ev) const {
    switch (static_cast<CommonErrc>(ev)) {
        case CommonErrc::InvalidArgument:
            return "invalid argument";
        case CommonErrc::NotFound:
            return "not found";
        case CommonErrc::AlreadyRegistered:
            return "already registered";
        default:
            return "(unrecognized error)";
    }
}

const std::error_category& EasyEvent::getCommonErrCategory() {
    static const EasyEvent::CommonErrCategory errCategory{};
    return errCategory;
}
