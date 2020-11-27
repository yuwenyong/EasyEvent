//
// Created by yuwenyong on 2020/11/27.
//

#include "EasyEvent/Event/TimerQueue.h"


EasyEvent::TimerHandle EasyEvent::TimerQueue::addTimer(Time deadline, Task<void()> &&callback) {
    auto timer = std::make_shared<Timer>(deadline, std::move(callback));
    TimerHandle res = timer;
    _timers.emplace_back(std::move(timer));
    size_t index = _timers.size() - 1;
    upHeap(index);
    return res;
}

void EasyEvent::TimerQueue::removeTimer(const TimerHandle& timer) {
    auto timeout = timer.lock();
    if (timeout) {
        timeout->callback = nullptr;
        ++_cancellations;
    }
}

std::vector<EasyEvent::TimerPtr> EasyEvent::TimerQueue::getReadyTimers() {
    std::vector<TimerPtr> dueTimers;
    auto now = time();
    while (!_timers.empty()) {
        if (!_timers.front()->callback) {
            if (_timers.size() > 1) {
                std::swap(_timers[0], _timers[_timers.size() - 1]);
            }
            assert(_cancellations > 0);
            --_cancellations;
            _timers.pop_back();
            if (_timers.size() > 1) {
                downHeap(0);
            }
        } else if (_timers.front()->deadline <= now) {
            if (_timers.size() > 1) {
                std::swap(_timers[0], _timers[_timers.size() - 1]);
            }
            dueTimers.emplace_back(std::move(_timers.back()));
            _timers.pop_back();
            if (_timers.size() > 1) {
                downHeap(0);
            }
        } else {
            break;
        }
    }
    if (_cancellations > 512 && _cancellations > (_timers.size() / 2)) {
        std::vector<TimerPtr> timers;
        _cancellations = 0;
        for (auto& timer: _timers) {
            if (timer->callback) {
                timers.emplace_back(std::move(timer));
            }
        }
        _timers.swap(timers);
        buildHeap();
    }
    return dueTimers;
}

void EasyEvent::TimerQueue::upHeap(size_t index) {
    size_t parent;
    while (index > 0) {
        parent = (index - 1) / 2;
        if (_timers[index]->deadline >= _timers[parent]->deadline) {
            break;
        }
        std::swap(_timers[index], _timers[parent]);
        index = parent;
    }
}

void EasyEvent::TimerQueue::downHeap(size_t index) {
    size_t child = index * 2 + 1;
    size_t minChild;
    while (child < _timers.size()) {
        if (child + 1 == _timers.size() || _timers[child]->deadline <= _timers[child + 1]->deadline) {
            minChild = child;
        } else {
            minChild = child + 1;
        }
        if (_timers[index]->deadline <= _timers[minChild]->deadline) {
            break;
        }
        std::swap(_timers[index], _timers[minChild]);
        index = minChild;
        child = index * 2 + 1;
    }
}

void EasyEvent::TimerQueue::buildHeap() {
    if (_timers.size() > 1) {
        size_t index = _timers.size() / 2 - 1;
        while (true){
            downHeap(index);
            if (index == 0) {
                break;
            }
            --index;
        }
    }
}