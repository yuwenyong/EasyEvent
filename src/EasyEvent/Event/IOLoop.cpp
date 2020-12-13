//
// Created by yuwenyong on 2020/11/24.
//

#include "EasyEvent/Event/IOLoop.h"
#include "EasyEvent/Event/SignalBlocker.h"
#include "EasyEvent/Logging/LogStream.h"


thread_local EasyEvent::IOLoop* EasyEvent::IOLoop::_current = nullptr;

EasyEvent::IOLoop::IOLoop(Logger *logger, bool installSignalHandlers, bool makeCurrent)
    : _logger(logger)
    , _waker(std::make_shared<Interrupter>()) {

#if defined(EASY_EVENT_USE_EPOLL)
    _epollFd = doEpollCreate();
#endif

    addHandler(_waker, IO_EVENT_READ);

    if (installSignalHandlers) {
        this->installSignalHandlers();
    }

    if (makeCurrent) {
        this->makeCurrent();
    }
}

EasyEvent::IOLoop::~IOLoop() noexcept {

    for (auto& handle: _signalHandles) {
        SignalCtrl::instance().remove(handle);
    }

#if defined(EASY_EVENT_USE_EPOLL)
    if (_epollFd != -1) {
        ::close(_epollFd);
    }
#endif
    if (_resolveThread) {
        _resolveQueries.terminate();
        _resolveThread->join();
    }
}

void EasyEvent::IOLoop::addHandler(const SelectablePtr &handler, IOEvents events) {
    Assert(_handlers.find(handler->getFD()) == _handlers.end());

#if defined(EASY_EVENT_USE_SELECT)
    if ((events & IO_EVENT_READ) != 0) {
        Verify(_readFdSet.set(handler->getFD()));
    }
    if ((events & IO_EVENT_WRITE) != 0) {
        Verify(_writeFdSet.set(handler->FD()));
    }
    Verify(_errorFdSet.set(handler->getFD()));
#elif defined(EASY_EVENT_USE_EPOLL)
    epoll_event ev = {0, {0}};
    ev.events = events | IO_EVENT_ERROR;
    ev.data.fd = handler->getFD();
    int result = epoll_ctl(_epollFd, EPOLL_CTL_ADD, handler->getFD(), &ev);
    if (result != 0) {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "add epoll handler");
    }
#else
    struct pollfd pollFd;
    pollFd.fd = handler->getFD();
    pollFd.events = events | IO_EVENT_ERROR;
    pollFd.revents = 0;
    _pollFdSet.emplace_back(pollFd);
#endif

    _handlers[handler->getFD()] = handler;
}

void EasyEvent::IOLoop::updateHandler(const SelectablePtr &handler, IOEvents events) {
    Assert(_handlers.find(handler->getFD()) != _handlers.end());

#if defined(EASY_EVENT_USE_SELECT)
    if ((events & IO_EVENT_READ) != 0) {
        _readFdSet.set(handler->getFD());
    } else {
        _readFdSet.clr(handler->getFD());
    }
    if ((events & IO_EVENT_WRITE) != 0) {
        _writeFdSet.set(handler->getFD());
    } else {
        _writeFdSet.clr(handler->getFD());
    }
#elif defined(EASY_EVENT_USE_EPOLL)
    epoll_event ev = {0, {0}};
    ev.events = events | IO_EVENT_ERROR;
    ev.data.fd = handler->getFD();
    int result = epoll_ctl(_epollFd, EPOLL_CTL_MOD, handler->getFD(), &ev);
    if (result != 0) {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "update epoll handler");
    }
#else
    for (size_t i = 0; i != _pollFdSet.size(); ++i) {
        if (_pollFdSet[i].fd == handler->getFD()) {
            _pollFdSet[i].events = events | IO_EVENT_ERROR;
            break;
        }
    }
#endif
}

void EasyEvent::IOLoop::removeHandler(const SelectablePtr &handler) {
    Assert(_handlers.find(handler->getFD()) != _handlers.end());

#if defined(EASY_EVENT_USE_SELECT)
    _readFdSet.clr(handler->getFD());
    _writeFdSet.clr(handler->getFD());
    Verify(_errorFdSet.clr(handler->getFD()));
#elif defined(EASY_EVENT_USE_EPOLL)
    epoll_event ev = {0, {0}};
    int result = epoll_ctl(_epollFd, EPOLL_CTL_DEL, handler->getFD(), &ev);
    if (result != 0) {
        std::error_code ec(errno, getSocketErrorCategory());
        throwError(ec, "remove epoll handler");
    }
#else
    for (size_t i = 0; i != _pollFdSet.size(); ++i) {
        if (_pollFdSet[i].fd == handler->getFD()) {
            if (i != _pollFdSet.size() - 1) {
                _pollFdSet[i] = _pollFdSet[_pollFdSet.size() - 1];
            }
            _pollFdSet.pop_back();
            break;
        }
    }
#endif

    _handlers.erase(handler->getFD());
}

void EasyEvent::IOLoop::start() {
    std::error_code ec;
    if (_running) {
        ec = EventErrors::AlreadyStarted;
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
    _resolveQueries.reset();
    _running = true;

    if (!_resolveQueries.empty()) {
        startResolveThread();
    }

    std::vector<Task<void()>> callbacks;
    std::vector<TimerPtr> readyTimers;
    Time pollTimeout;
    bool hasCallback;
    int numEvents;

#if defined(EASY_EVENT_USE_SELECT)
    WinFdSetAdapter readFdSet;
    WinFdSetAdapter writeFdSet;
    WinFdSetAdapter errorFdSet;
    std::unordered_map<SocketType, IOEvents> events;
#elif defined(EASY_EVENT_USE_EPOLL)
    epoll_event events[256];
#else
    std::vector<struct pollfd> pollFdSet;
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
                LOG_ERROR(_logger) << "Unknown error in callback.";
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
                LOG_ERROR(_logger) << "Unknown error in timer callback.";
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

#if defined(EASY_EVENT_USE_SELECT)
        readFdSet = _readFdSet;
        writeFdSet = _writeFdSet;
        errorFdSet = _errorFdSet;
        if (!pollTimeout) {
            numEvents = select(0, readFdSet, writeFdSet, errorFdSet, nullptr);
        } else {
            struct timeval tv;
            tv.tv_sec = (long)pollTimeout.seconds();
            tv.tv_usec = (long)(pollTimeout.microSeconds() - pollTimeout.seconds() * 1000000);
            numEvents = select(0, readFdSet, writeFdSet, errorFdSet, &tv);
        }
        if (numEvents < 0) {
            if (WSAGetLastError() == WSAEINTR) {
                continue;
            }
            ec.assign(WSAGetLastError(), getSocketErrorCategory());
            throwError(ec, "select");
        }
        if (numEvents > 0) {
            events.clear();
            for (u_int i = 0; i < readFdSet.size(); ++i) {
                events[readFdSet.get(i)] = IO_EVENT_READ;
            }

            for (u_int i = 0; i < writeFdSet.size(); ++i) {
                auto iter = events.find(writeFdSet.get(i));
                if (iter != events.end()) {
                    iter->second |= IO_EVENT_WRITE;
                } else {
                    events[writeFdSet.get(i)] = IO_EVENT_WRITE;
                }
            }

            for (u_int i = 0; i < errorFdSet.size(); ++i) {
                auto iter = events.find(errorFdSet.get(i));
                if (iter != events.end()) {
                    iter->second |= IO_EVENT_ERROR;
                } else {
                    events[errorFdSet.get(i)] = IO_EVENT_ERROR;
                }
            }

            for (auto &event: events) {
                auto iter = _handlers.find(event.first);
                if (iter == _handlers.end()) {
                    continue;
                }
                try {
                    iter->second->handleEvents(event.second);
                } catch (std::exception& e) {
                    LOG_ERROR(_logger) << "Exception in handler: " << e.what();
                } catch (...) {
                    LOG_ERROR(_logger) << "Unknown error in handler.";
                }
            }
        }

#elif defined(EASY_EVENT_USE_EPOLL)
        numEvents = epoll_wait(_epollFd, events, 256, (int)pollTimeout.milliSeconds());
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
                LOG_ERROR(_logger) << "Unknown error in handler.";
            }
        }
#else
        pollFdSet = _pollFdSet;
        numEvents = poll(&pollFdSet[0], pollFdSet.size(), (int)pollTimeout.milliSeconds());
        if (numEvents < 0) {
            if (errno == EINTR) {
                continue;
            }
            ec.assign(errno, getSocketErrorCategory());
            throwError(ec, "poll");
        }
        for (auto it = pollFdSet.begin(); it != pollFdSet.end() && numEvents > 0; ++it) {
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
                LOG_ERROR(_logger) << "Unknown error in handler.";
            }
        }
#endif
    }

    _resolveQueries.terminate();
    if (_resolveThread) {
        _resolveThread->join();
        _resolveThread.reset();
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

EasyEvent::ResolveHandle EasyEvent::IOLoop::resolve(std::string host, unsigned short port, ProtocolSupport protocol,
                                                    bool preferIPv6, ResolveQuery::CallbackType &&callback) {
    auto query = std::make_shared<ResolveQuery>(this, std::move(host), port, protocol, preferIPv6, std::move(callback));
    if (!query->doResolve()) {
        _resolveQueries.forceEnqueue(query);
        if (!_resolveThread && _running) {
            startResolveThread();
        }
    }
    return ResolveHandle(query);
}

void EasyEvent::IOLoop::installSignalHandlers() {
    _signalHandles.emplace_back(SignalCtrl::instance().add(SIGTERM, [this](int sigNum) {
        LOG_INFO(_logger) << "Received SIGTERM, shutting down.";
        stop();
    }));

    _signalHandles.emplace_back(SignalCtrl::instance().add(SIGINT, [this](int sigNum) {
        LOG_INFO(_logger) << "Received SIGINT, shutting down.";
        stop();
    }));

#if defined(SIGBREAK)
    _signalHandles.emplace_back(SignalCtrl::instance().add(SIGBREAK, [this](int sigNum) {
            LOG_INFO(_logger) << "Received SIGBREAK, shutting down.";
            stop();
        }));
#endif

#if defined(SIGQUIT)
    _signalHandles.emplace_back(SignalCtrl::instance().add(SIGQUIT, [this](int sigNum) {
        LOG_INFO(_logger) << "Received SIGQUIT, shutting down.";
        stop();
    }));
#endif
}

void EasyEvent::IOLoop::startResolveThread() {
    Assert(!_resolveThread);
    SignalBlocker sigBlocker;
    _resolveThread = std::make_unique<std::thread>([this]() {
        while (_running) {
            ResolveQueryPtr query;
            query = _resolveQueries.dequeue();
            if (!query) {
                break;
            }
            query->doResolveBackground();
        }
    });
}

#if defined(EASY_EVENT_USE_EPOLL)

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
