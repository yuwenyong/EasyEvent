//
// Created by yuwenyong on 2020/11/24.
//

#ifndef EASYEVENT_EVENT_IOLOOP_H
#define EASYEVENT_EVENT_IOLOOP_H

#include "EasyEvent/Event/EvtCommon.h"
#include "EasyEvent/Common/Time.h"
#include "EasyEvent/Common/Task.h"
#include "EasyEvent/Event/SocketInit.h"


namespace EasyEvent {

    class Logger;

    class EASY_EVENT_API IOLoop {
    public:
        explicit IOLoop(Logger* logger=nullptr, bool makeCurrent=false)
            : _logger(logger) {
            if (makeCurrent) {
                this->makeCurrent();
            }
        }

        void close(bool allSockets=false);

        void addHandler(const SelectablePtr& handler, IOEvents events);

        void updateHandler(const SelectablePtr& handler, IOEvents events);

        void removeHandler(const SelectablePtr& handler);

        void start();

        void stop() {

        }

        Time time() const {
            return Time::now();
        }

        // callLater
        // callAt
        // removeTimeout

        void addCallback(Task<void()>&& callback) {

        }

        void addCallbackFromSignal(Task<void()>&& callback) {
            addCallback(std::move(callback));
        }

        void makeCurrent() {
            _current = this;
        }

        static IOLoop* current() {
            return _current;
        }

        static void clearCurrent() {
            _current = nullptr;
        }
    protected:
        void handleCallbackException(std::exception& e);

        SocketInit _sockInit;
        Logger* _logger;
        std::unordered_map<SocketType, SelectablePtr> _handlers;
        std::unordered_set<SocketType> _discardEvents;
        std::mutex _mutex;
        std::vector<Task<void()>> _callbacks;
        // Timeout?
        // cancel count?
        bool _running{false};
        bool _stopped{false};
        bool _closing{false};

        thread_local static IOLoop* _current;
    };

}

#endif //EASYEVENT_EVENT_IOLOOP_H
