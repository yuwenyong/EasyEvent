//
// Created by yuwenyong on 2020/10/26.
//

#include "EasyEvent/Common/Errors.h"

const char * EasyEvent::CommonErrCategory::name() const noexcept
{
    return "common";
}

std::string EasyEvent::CommonErrCategory::message(int ev) const
{
    switch (static_cast<CommonErrc>(ev))
    {
        case CommonErrc::InvalidArgument:
            return "invalid argument";
        default:
            return "(unrecognized error)";
    }
}

const EasyEvent::CommonErrCategory EasyEvent::gCommonErrCategory{};



