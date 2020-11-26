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
    std::lock_guard<std::mutex> lock(_mutex);
    auto iter = _registrations.find(sigNum);
    if (iter == _registrations.end()) {
        registerSignal(sigNum, ec);
        if (ec) {
            return {};
        }
        _registrations[sigNum] = {std::move(op)};
    } else {
        iter->second.emplace_back(std::move(op));
    }
    ec = {0, ec.category()};
    return res;
}

void EasyEvent::SignalCtrl::remove(SignalHandle handle, std::error_code &ec) {
    auto op = handle.lock();
    if (!op) {
        ec = {0, ec.category()};
        return;
    }
    std::lock_guard<std::mutex> lock(_mutex);
    auto iter = _registrations.find(op->_sigNum);
    if (iter == _registrations.end()) {
        ec = std::make_error_code(std::errc::invalid_argument);
        return;
    }
    auto opCount = std::count(iter->second.begin(), iter->second.end(), op);
    if (opCount == 0) {
        ec = std::make_error_code(std::errc::invalid_argument);
        return;
    }
    if (iter->second.size() == (size_t)opCount) {
        unregisterSignal(op->_sigNum, ec);
        if (ec) {
            return;
        }
        _registrations.erase(iter);
    } else {
        auto newEnd = std::remove(iter->second.begin(), iter->second.end(), op);
        assert(newEnd != iter->second.end());
        iter->second.erase(newEnd, iter->second.end());
    }
    ec = {0, ec.category()};
}

void EasyEvent::SignalCtrl::registerSignal(int sigNum, std::error_code &ec) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    if (::signal(sigNum, signalHandler) == SIG_ERR) {
        ec = std::make_error_code(std::errc::invalid_argument);
        return;
    }
#else
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    sigfillset(&sa.sa_mask);
    if (::sigaction(sigNum, &sa, 0) == -1) {
        ec = {errno, std::system_category()};
        return;
    }
#endif
    ec = {0, ec.category()};
}

void EasyEvent::SignalCtrl::unregisterSignal(int sigNum, std::error_code &ec) {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    if (::signal(sigNum, SIG_DFL) == SIG_ERR) {
        ec = std::make_error_code(std::errc::invalid_argument);
        return;
    }
#else
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    if (::sigaction(sigNum, &sa, 0) == -1) {
        ec = {errno, std::system_category()};
        return;
    }
#endif
    ec = {0, ec.category()};
}

EasyEvent::SignalCtrl::~SignalCtrl() {

}

void EasyEvent::SignalCtrl::signalHandler(int sigNum) {
    auto ops = instance().getOperations(sigNum);
    for (auto &op: ops) {
        op->_callback(sigNum);
    }
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    if (instance().getOperationCount(sigNum) > 0) {
        ::signal(sigNum, signalHandler);
    }
#endif
}