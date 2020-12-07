//
// Created by yuwenyong on 2020/11/24.
//

#include "EasyEvent/Event/IOLoop.h"
#include "EasyEvent/Logging/LogStream.h"


thread_local EasyEvent::IOLoop* EasyEvent::IOLoop::_current = nullptr;

EasyEvent::IOLoop::IOLoop(Logger *logger, bool makeCurrent)
    : _logger(logger)
    , _waker(std::make_shared<Interrupter>()) {

    addHandler(_waker, IO_EVENT_READ);

    if (makeCurrent) {
        this->makeCurrent();
    }
}

void EasyEvent::IOLoop::addHandler(const SelectablePtr &handler, IOEvents events) {
    _handlers[handler->getSocket()] = handler;
    // TODO, Register
}

void EasyEvent::IOLoop::updateHandler(const SelectablePtr &handler, IOEvents events) {
    // TODO, Update
}

void EasyEvent::IOLoop::removeHandler(const SelectablePtr &handler) {
    assert(_handlers.find(handler->getSocket()) != _handlers.end());
    _handlers.erase(handler->getSocket());
    _discardEvents.insert(handler->getSocket());
    // TODO, Remove
}

void EasyEvent::IOLoop::start(std::error_code& ec) {
    ec.assign(0, ec.category());
    if (_running) {
        ec = EventErrors::IOLoopAlreadyStarted;
        return;
    }
    if (_stopped) {
        _stopped = false;
        return;
    }
    IOLoop* oldCurrent = IOLoop::current();
    if (oldCurrent != this) {
        makeCurrent();
    }
    _threadIdent = std::this_thread::get_id();
    _running = true;

    std::vector<Task<void()>> callbacks;
    std::vector<TimerPtr> readyTimers;
    Time pollTimeout;
    bool hasCallback;

    while (true) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            callbacks.swap(_callbacks);
        }
        readyTimers = _timers.getReadyTimers();

        for (auto &callback: callbacks) {
            try {
                callback();
            } catch (std::exception& e) {
                LOG_ERROR(_logger) << "Exception in callback: " << e.what();
            } catch (...) {
                LOG_ERROR(_logger) << "Unknown error in callback";
            }
            callback = nullptr;
        }
        callbacks.clear();

        for (auto& timer: readyTimers) {
            try {
                timer->callback();
            } catch (std::exception& e) {
                LOG_ERROR(_logger) << "Exception in timer callback: " << e.what();
            } catch (...) {
                LOG_ERROR(_logger) << "Unknown error in timer callback";
            }
            timer = nullptr;
        }
        readyTimers.clear();

        {
            std::lock_guard<std::mutex> lock(_mutex);
            hasCallback = !_callbacks.empty();
        }

        if (hasCallback) {
            pollTimeout = Time::microSeconds(0);
        } else if (_timers.getTimerCount() > 0) {
            pollTimeout = _timers.getEarliestDeadline() - time();
            if (pollTimeout < Time::microSeconds(0)) {
                pollTimeout = Time::microSeconds(0);
            } else if (pollTimeout > Time::seconds(3600)) {
                pollTimeout = Time::seconds(3600);
            }
        } else {
            pollTimeout = Time::seconds(3600);
        }

        if (!_running) {
            break;
        }
    }

    _stopped = false;
    if (oldCurrent == nullptr) {
        IOLoop::clearCurrent();
    } else if (oldCurrent != this) {
        oldCurrent->makeCurrent();
    }
 }

void EasyEvent::IOLoop::stop() {
    _running = false;
    _stopped = true;
    if (_threadIdent != std::this_thread::get_id()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _waker->interrupt();
    }
}

void EasyEvent::IOLoop::addCallback(Task<void()> &&callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _callbacks.emplace_back(std::move(callback));
    if (_threadIdent != std::this_thread::get_id()) {
        _waker->interrupt();
    }
}

void EasyEvent::IOLoop::handleCallbackException(std::exception &e) {
    LOG_ERROR(_logger) << "Exception in callback: " << e.what();
}