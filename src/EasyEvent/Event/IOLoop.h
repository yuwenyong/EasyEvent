//
// Created by yuwenyong on 2020/11/24.
//

#ifndef EASYEVENT_EVENT_IOLOOP_H
#define EASYEVENT_EVENT_IOLOOP_H

#include "EasyEvent/Event/Event.h"
#include "EasyEvent/Common/Time.h"
#include "EasyEvent/Common/Task.h"
#include "EasyEvent/Common/ConcurrentQueue.h"
#include "EasyEvent/Event/SocketInit.h"
#include "EasyEvent/Event/TimerQueue.h"
#include "EasyEvent/Event/Interrupter.h"
#include "EasyEvent/Event/Resolver.h"
#include "EasyEvent/Event/SignalCtrl.h"


namespace EasyEvent {

    class Logger;

    class EASY_EVENT_API IOLoop {
    public:
        IOLoop(const IOLoop&) = delete;
        IOLoop& operator=(const IOLoop&) = delete;

        explicit IOLoop(Logger* logger=nullptr, bool installSignalHandlers=false, bool makeCurrent=false);

        ~IOLoop() noexcept;

        Logger* getLogger() const {
            return _logger;
        }

        void addHandler(const SelectablePtr& handler, IOEvents events);

        void updateHandler(const SelectablePtr& handler, IOEvents events);

        void removeHandler(const SelectablePtr& handler);

        void start();

        void stop();

        Time time() const {
            return TimerQueue::time();
        }

        TimerHandle callLater(Time delay, Task<void()>&& callback) {
            return _timers.addTimer(time() + delay, std::move(callback));
        }

        TimerHandle callAt(Time when, Task<void()>&& callback) {
            return _timers.addTimer(when, std::move(callback));
        }

        void removeTimeout(const TimerHandle& timeout) {
            _timers.removeTimer(timeout);
        }

        void addCallback(Task<void()>&& callback);

        ResolveHandle resolve(std::string host, unsigned short port, ProtocolSupport protocol, bool preferIPv6,
                              ResolveQuery::CallbackType&& callback);

        void makeCurrent() {
            _current = this;
        }

        static IOLoop* current() {
            return _current;
        }

        static void clearCurrent() {
            _current = nullptr;
        }

    private:
        void installSignalHandlers();

        void startResolveThread();

#if defined(EASY_EVENT_USE_EPOLL)
        enum {
            EpollSize = 20000,
        };

        int doEpollCreate();
#endif

        SocketInit _sockInit;
        Logger* _logger;

#if defined(EASY_EVENT_USE_SELECT)
        WinFdSetAdapter _readFdSet;
        WinFdSetAdapter _writeFdSet;
        WinFdSetAdapter _errorFdSet;
#elif defined(EASY_EVENT_USE_EPOLL)
        int _epollFd;
#else
        std::vector<struct pollfd> _pollFdSet;
#endif

        std::unordered_map<SocketType, SelectablePtr> _handlers;
        std::mutex _mutex;
        std::vector<Task<void()>> _callbacks;
        TimerQueue _timers;
        std::shared_ptr<Interrupter> _waker;
        std::vector<SignalHandle> _signalHandles;
        std::thread::id _threadIdent;

        volatile bool _running{false};
        volatile bool _stopped{false};

        ConcurrentQueue<ResolveQueryPtr> _resolveQueries;
        std::unique_ptr<std::thread> _resolveThread;

        thread_local static IOLoop* _current;
    };

}

#endif //EASYEVENT_EVENT_IOLOOP_H
