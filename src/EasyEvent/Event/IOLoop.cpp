//
// Created by yuwenyong on 2020/11/24.
//

#include "EasyEvent/Event/IOLoop.h"
#include "EasyEvent/Logging/LogStream.h"


thread_local EasyEvent::IOLoop* EasyEvent::IOLoop::_current = nullptr;

EasyEvent::IOLoop::IOLoop(Logger *logger, bool makeCurrent)
    : _logger(logger)
    , _waker(std::make_shared<Interrupter>()) {

#ifdef EASY_EVENT_USE_EPOLL
    _epollFd = doEpollCreate();
#endif

    addHandler(_waker, IO_EVENT_READ);

    if (makeCurrent) {
        this->makeCurrent();
    }
}

EasyEvent::IOLoop::~IOLoop() noexcept {

#ifdef EASY_EVENT_USE_EPOLL
    if (_epollFd != -1) {
        ::close(_epollFd);
    }
#endif

}

void EasyEvent::IOLoop::addHandler(const SelectablePtr &handler, IOEvents events) {
    assert(_handlers.find(handler->getSocket()) == _handlers.end());

#ifdef EASY_EVENT_USE_EPOLL
    epoll_event ev = {0, {0}};
    ev.events = events | IO_EVENT_ERROR;
    ev.data.fd = handler->getSocket();
    int result = epoll_ctl(_epollFd, EPOLL_CTL_ADD, handler->getSocket(), &ev);
    if (result != 0) {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "add epoll handler");
    }
#else
    struct pollfd pollFd;
    pollFd.fd = handler->getSocket();
    pollFd.events = events | IO_EVENT_ERROR;
    pollFd.revents = 0;
    _pollFdSet.emplace_back(pollFd);
#endif

    _handlers[handler->getSocket()] = handler;
}

void EasyEvent::IOLoop::updateHandler(const SelectablePtr &handler, IOEvents events) {
    assert(_handlers.find(handler->getSocket()) != _handlers.end());

#ifdef EASY_EVENT_USE_EPOLL
    epoll_event ev = {0, {0}};
    ev.events = events | IO_EVENT_ERROR;
    ev.data.fd = handler->getSocket();
    int result = epoll_ctl(_epollFd, EPOLL_CTL_MOD, handler->getSocket(), &ev);
    if (result != 0) {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "update epoll handler");
    }
#else
    for (size_t i = 0; i != _pollFdSet.size(); ++i) {
        if (_pollFdSet[i].fd == handler->getSocket()) {
            _pollFdSet[i].events = events | IO_EVENT_ERROR;
            break;
        }
    }
#endif
}

void EasyEvent::IOLoop::removeHandler(const SelectablePtr &handler) {
    assert(_handlers.find(handler->getSocket()) != _handlers.end());

#ifdef EASY_EVENT_USE_EPOLL
    epoll_event ev = {0, {0}};
    int result = epoll_ctl(_epollFd, EPOLL_CTL_DEL, handler->getSocket(), &ev);
    if (result != 0) {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "remove epoll handler");
    }
#else
    for (size_t i = 0; i != _pollFdSet.size(); ++i) {
        if (_pollFdSet[i].fd == handler->getSocket()) {
            if (i != _pollFdSet.size() - 1) {
                _pollFdSet[i] = _pollFdSet[_pollFdSet.size() - 1];
            }
            _pollFdSet.pop_back();
            break;
        }
    }
#endif

    _handlers.erase(handler->getSocket());
}

void EasyEvent::IOLoop::start() {
    std::error_code ec;
    if (_running) {
        ec = EventErrors::IOLoopAlreadyStarted;
        throwError(ec, "IOLoop");
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

#ifdef EASY_EVENT_USE_EPOLL
    epoll_event events[256];
#endif

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

#ifdef EASY_EVENT_USE_EPOLL
        int numEvents = epoll_wait(_epollFd, events, 256, (int)pollTimeout.milliSeconds());
        if (numEvents < 0) {
            if (errno == EINTR) {
                continue;
            }
            ec.assign(errno, getSocketErrorCategory());
            throwError(ec, "epoll wait");
        }
        for (int i = 0; i < numEvents; ++i) {
            int fd = events[i].data.fd;
            auto iter = _handlers.find(fd);
            if (iter == _handlers.end()) {
                continue;
            }
            try {
                iter->second->handleEvents((IOEvents)events[i].events);
            } catch (std::exception& e) {
                LOG_ERROR(_logger) << "Exception in handler: " << e.what();
            } catch (...) {
                LOG_ERROR(_logger) << "Unknown error in handler";
            }
        }
#else
        _updatedFdSet = _pollFdSet;
        int numEvents = poll(&_updatedFdSet[0], _updatedFdSet.size(), (int)pollTimeout.milliSeconds());
        if (numEvents < 0) {
            if (errno == EINTR) {
                continue;
            }
            ec.assign(errno, getSocketErrorCategory());
            throwError(ec, "poll");
        }
        for (auto it = _updatedFdSet.begin(); it != _updatedFdSet.end() && numEvents > 0; ++it) {
            if (it->revents == 0) {
                continue;
            }
            --numEvents;
            auto iter = _handlers.find(it->fd);
            if (iter == _handlers.end()) {
                continue;
            }
            try {
                iter->second->handleEvents((IOEvents)it->revents);
            } catch (std::exception& e) {
                LOG_ERROR(_logger) << "Exception in handler: " << e.what();
            } catch (...) {
                LOG_ERROR(_logger) << "Unknown error in handler";
            }
        }
#endif

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

#ifdef EASY_EVENT_USE_EPOLL

int EasyEvent::IOLoop::doEpollCreate() {
#if defined(EPOLL_CLOEXEC)
    int fd = epoll_create1(EPOLL_CLOEXEC);
#else
    int fd = -1;
    errno = EINVAL;
#endif
    if (fd == -1 && (errno == EINVAL || errno == ENOSYS)) {
        fd = epoll_create(EpollSize);
        if (fd != -1) {
            ::fcntl(fd, F_SETFD, FD_CLOEXEC);
        }
    }

    if (fd == -1) {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "create epoll");
    }

    return fd;
}

#endif
