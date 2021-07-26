//
// Created by yuwenyong on 2020/11/26.
//

#ifndef EASYEVENT_EVENT_SIGNALBLOCKER_H
#define EASYEVENT_EVENT_SIGNALBLOCKER_H

#include "EasyEvent/Event/EventBase.h"


namespace EasyEvent {

    class SignalBlocker {
    public:
        SignalBlocker() {
#if EASY_EVENT_PLATFORM != EASY_EVENT_PLATFORM_WINDOWS
            sigset_t newMask;
            sigfillset(&newMask);
            _blocked = (pthread_sigmask(SIG_BLOCK, &newMask, &_oldMask) == 0);
#endif
        }

        ~SignalBlocker() {
#if EASY_EVENT_PLATFORM != EASY_EVENT_PLATFORM_WINDOWS
            if (_blocked) {
                pthread_sigmask(SIG_SETMASK, &_oldMask, 0);
            }
#endif
        }

        SignalBlocker(const SignalBlocker&) = delete;

        SignalBlocker& operator=(const SignalBlocker&) = delete;

        void block() {
#if EASY_EVENT_PLATFORM != EASY_EVENT_PLATFORM_WINDOWS
            if (!_blocked) {
                sigset_t newMask;
                sigfillset(&newMask);
                _blocked = (pthread_sigmask(SIG_BLOCK, &newMask, &_oldMask) == 0);
            }
#endif
        }

        void unblock() {
#if EASY_EVENT_PLATFORM != EASY_EVENT_PLATFORM_WINDOWS
            if (_blocked) {
                _blocked = (pthread_sigmask(SIG_SETMASK, &_oldMask, 0) != 0);
            }
#endif
        }

    private:
#if EASY_EVENT_PLATFORM != EASY_EVENT_PLATFORM_WINDOWS
        bool _blocked{false};
        sigset_t _oldMask;
#endif
    };

}

#endif //EASYEVENT_EVENT_SIGNALBLOCKER_H
