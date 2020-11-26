//
// Created by yuwenyong on 2020/11/26.
//

#include "EasyEvent/Event/SignalCtrl.h"


EasyEvent::SignalHandle EasyEvent::SignalCtrl::add(int sigNum, Task<void(int)> &&callback, std::error_code &ec) {
    if (sigNum < 0 || sigNum >= MaxSignalNumber) {
        ec = std::make_error_code(std::errc::invalid_argument);
        return {};
    }

    auto op = std::make_shared<SignalOp>(sigNum, std::move(callback));
    SignalHandle res = op;
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    auto iter = _registrations.find(sigNum);
    if (iter == _registrations.end()) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS

#else
#endif
    } else {
        iter->second.emplace_back(std::move(op));
    }
    ec = {0, ec.category()};
    return res;
}

void EasyEvent::SignalCtrl::remove(SignalHandle handle) {

}

void EasyEvent::SignalCtrl::signalHandler(int sigNum) {

}

EasyEvent::SignalCtrl::~SignalCtrl() {

}