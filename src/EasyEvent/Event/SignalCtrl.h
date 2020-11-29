//
// Created by yuwenyong on 2020/11/26.
//

#ifndef EASYEVENT_EVENT_SIGNALCTRL_H
#define EASYEVENT_EVENT_SIGNALCTRL_H

#include "EasyEvent/Event/EvtCommon.h"
#include "EasyEvent/Common/Task.h"


namespace EasyEvent {

#if defined(NSIG) && (NSIG > 0)
    enum { MaxSignalNumber = NSIG };
#else
    enum { MaxSignalNumber = 128 };
#endif

    class SignalOp {
    public:
        friend class SignalCtrl;

        SignalOp(int sigNum, Task<void(int)>&& callback)
            : _sigNum(sigNum)
            , _callback(std::move(callback)) {

        }

    protected:
        int _sigNum;
        Task<void(int)> _callback;
    };

    using SignalOpPtr = std::shared_ptr<SignalOp>;
    using SignalHandle = std::weak_ptr<SignalOp>;

    class EASY_EVENT_API SignalCtrl {
    public:
        SignalCtrl(const SignalCtrl&) = delete;
        SignalCtrl& operator=(const SignalCtrl&) = delete;

        SignalHandle add(int sigNum, Task<void(int)> &&callback, std::error_code &ec);

        SignalHandle add(int sigNum, Task<void(int)> &&callback) {
            std::error_code ec;
            auto res = add(sigNum, std::move(callback), ec);
            throwError(ec, "add signal handler");
            return res;
        }

        void remove(SignalHandle handle, std::error_code &ec);

        void remove(SignalHandle handle) {
            std::error_code ec;
            remove(std::move(handle), ec);
            throwError(ec, "remove signal handler");
        }

        static SignalCtrl& instance() {
            static SignalCtrl sc;
            return sc;
        }
    private:
        SignalCtrl() = default;

        ~SignalCtrl();

        void registerSignal(int sigNum, std::error_code &ec);

        void unregisterSignal(int sigNum, std::error_code &ec);

        std::vector<SignalOpPtr> getOperations(int sigNum) const {
            std::lock_guard<std::mutex> lock(_mutex);
            return _registrations.at(sigNum);
        }

        size_t getOperationCount(int sigNum) const {
            std::lock_guard<std::mutex> lock(_mutex);
            auto iter = _registrations.find(sigNum);
            return iter != _registrations.end() ? iter->second.size() : 0;
        }

        static void signalHandler(int sigNum);

        mutable std::mutex _mutex;
        std::map<int, std::vector<SignalOpPtr>> _registrations;
    };

}

#endif //EASYEVENT_EVENT_SIGNALCTRL_H
