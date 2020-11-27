//
// Created by yuwenyong on 2020/11/27.
//

#ifndef EASYEVENT_EVENT_TIMERQUEUE_H
#define EASYEVENT_EVENT_TIMERQUEUE_H

#include "EasyEvent/Event/EvtCommon.h"
#include "EasyEvent/Common/Task.h"
#include "EasyEvent/Common/Time.h"


namespace EasyEvent {

    struct Timer {
        Timer(Time d, Task<void()>&& cb)
            : deadline(d)
            , callback(std::move(cb)) {

        }

        Time deadline;
        Task<void()> callback;
    };

    using TimerPtr = std::shared_ptr<Timer>;
    using TimerHandle = std::weak_ptr<Timer>;

    bool operator<(const Timer& lhs, const Timer& rhs) {
        return lhs.deadline < rhs.deadline;
    }

    class EASY_EVENT_API TimerQueue {
    public:
        TimerQueue(const TimerQueue&) = delete;
        TimerQueue& operator=(const TimerQueue&) = delete;

        TimerHandle addTimer(Time deadline, Task<void()>&& callback);

        void removeTimer(const TimerHandle& timer);

        std::vector<TimerPtr> getReadyTimers();

        size_t getTimerCount() const {
            return _timers.size();
        }

        Time getEarliestDeadline() const {
            return _timers.empty() ? Time() : _timers.front()->deadline;
        }

        static Time time() {
            return Time::now();
        }
    protected:
        void upHeap(size_t index);

        void downHeap(size_t index);

        void buildHeap();

        std::vector<TimerPtr> _timers;
        size_t _cancellations{0};
    };

}

#endif //EASYEVENT_EVENT_TIMERQUEUE_H
