//
// Created by yuwenyong on 2020/11/21.
//

#include "EasyEvent/Common/Task.h"


EasyEvent::BadTaskCall::~BadTaskCall() noexcept = default;

const char * EasyEvent::BadTaskCall::what() const noexcept {
    return "bad task call";
}