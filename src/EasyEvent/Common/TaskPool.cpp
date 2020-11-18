//
// Created by yuwenyong on 2020/11/7.
//

#include "EasyEvent/Common/TaskPool.h"


EasyEvent::TaskPool::~TaskPool() {
    terminate();
    wait();
}

bool EasyEvent::TaskPool::start(size_t threadCount) {
    if (threadCount == 0) {
        return false;
    }
    if (!_threads.empty()) {
        return false;
    }
    if (stopped() || terminated()) {
        return false;
    }
    for (size_t i = 0; i != threadCount; ++i) {
        _threads.emplace_back(std::thread([this](){
            process();
        }));
    }
    return true;
}

void EasyEvent::TaskPool::process() {
    while (true) {
        auto task = _tasks.dequeue();
        if (!task) {
            break;
        }
        (*task)->call();
    }
}