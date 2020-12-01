//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_COMMON_CONCURRENTQUEUE_H
#define EASYEVENT_COMMON_CONCURRENTQUEUE_H

#include "EasyEvent/Common/Config.h"


namespace EasyEvent {

    template <typename ValueT>
    class ConcurrentQueue {
    public:
        ConcurrentQueue() = default;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

        bool empty() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _queue.empty();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _queue.size();
        }

        bool enqueue(ValueT &&val) {
            std::lock_guard<std::mutex> lock(_mut);
            if (_terminated || _stopped) {
                return false;
            }
            _queue.emplace_back(std::forward<ValueT>(val));
            _cond.notify_one();
            return true;
        }

        void forceEnqueue(ValueT &&val) {
            std::lock_guard<std::mutex> lock(_mut);
            _queue.emplace_back(std::forward<ValueT>(val));
            if (!_terminated && !_stopped) {
                _cond.notify_one();
            }
        }

        bool dequeue(ValueT &val) {
            bool res{false};
            std::unique_lock<std::mutex> lock(_mut);
            do {
                if (_terminated) {
                    break;
                }
                if (!_queue.empty()) {
                    val = std::move(_queue.front());
                    _queue.pop_front();
                    res = true;
                    break;
                }
                if (_stopped) {
                    break;
                }
                _cond.wait(lock);
            } while (true);
            return res;
        }

        std::shared_ptr<ValueT> dequeue() {
            std::shared_ptr<ValueT> res = nullptr;
            std::unique_lock<std::mutex> lock(_mut);
            do {
                if (_terminated) {
                    break;
                }
                if (!_queue.empty()) {
                    res = std::make_shared<ValueT>(std::move(_queue.front()));
                    _queue.pop_front();
                    break;
                }
                if (_stopped) {
                    break;
                }
                _cond.wait(lock);
            } while (true);
            return res;
        }

        bool tryDequeue(ValueT &val) {
            std::lock_guard<std::mutex> lock(_mut);
            if (_terminated) {
                return false;
            }
            if (!_queue.empty()) {
                val = std::move(_queue.front());
                _queue.pop_front();
                return true;
            }
            return false;
        }

        std::shared_ptr<ValueT> tryDequeue() {
            std::lock_guard<std::mutex> lock(_mut);
            if (_terminated) {
                return nullptr;
            }
            if (!_queue.empty()) {
                auto res = std::make_shared<ValueT>(std::move(_queue.front()));
                _queue.pop_front();
                return res;
            }
            return nullptr;
        }

        void stop() {
            std::lock_guard<std::mutex> lock(_mut);
            _stopped = true;
            _cond.notify_all();
        }

        bool stopped() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _stopped;
        }

        void terminate() {
            std::lock_guard<std::mutex> lock(_mut);
            _terminated = true;
            _cond.notify_all();
        }

        bool terminated() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _terminated;
        }

        void reset() {
            std::lock_guard<std::mutex> lock(_mut);
            _stopped = false;
            _terminated = false;
        }
    protected:
        mutable std::mutex _mut;
        std::condition_variable _cond;
        std::deque<ValueT> _queue;
        volatile bool _stopped{false};
        volatile bool _terminated{false};
    };


    template <typename ValueT>
    class ConcurrentQueue<std::shared_ptr<ValueT>> {
    public:
        ConcurrentQueue() = default;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

        bool empty() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _queue.empty();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _queue.size();
        }

        bool enqueue(std::shared_ptr<ValueT> val) {
            std::lock_guard<std::mutex> lock(_mut);
            if (_terminated || _stopped) {
                return false;
            }
            _queue.emplace_back(std::move(val));
            _cond.notify_one();
            return true;
        }

        void forceEnqueue(std::shared_ptr<ValueT> val) {
            std::lock_guard<std::mutex> lock(_mut);
            _queue.emplace_back(std::move(val));
            if (!_terminated && !_stopped) {
                _cond.notify_one();
            }
        }

        bool dequeue(std::shared_ptr<ValueT>& val) {
            bool res{false};
            std::unique_lock<std::mutex> lock(_mut);

            do {
                if (_terminated) {
                    val = nullptr;
                    break;
                }
                if (!_queue.empty()) {
                    val = std::move(_queue.front());
                    _queue.pop_front();
                    res = true;
                    break;
                }
                if (_stopped) {
                    val = nullptr;
                    break;
                }
                _cond.wait(lock);
            } while (true);
            return res;
        }

        std::shared_ptr<ValueT> dequeue() {
            std::shared_ptr<ValueT> res = nullptr;
            std::unique_lock<std::mutex> lock(_mut);
            do {
                if (_terminated) {
                    break;
                }
                if (!_queue.empty()) {
                    res = std::move(_queue.front());
                    _queue.pop_front();
                    break;
                }
                if (_stopped) {
                    break;
                }
                _cond.wait(lock);
            } while (true);
            return res;
        }

        bool tryDequeue(ValueT &val) {
            std::lock_guard<std::mutex> lock(_mut);
            if (_terminated) {
                return false;
            }
            if (!_queue.empty()) {
                val = std::move(_queue.front());
                _queue.pop_front();
                return true;
            }
            return false;
        }

        std::shared_ptr<ValueT> tryDequeue() {
            std::lock_guard<std::mutex> lock(_mut);
            if (_terminated) {
                return nullptr;
            }
            if (!_queue.empty()) {
                auto res = std::move(_queue.front());
                _queue.pop_front();
                return res;
            }
            return nullptr;
        }

        void stop() {
            std::lock_guard<std::mutex> lock(_mut);
            _stopped = true;
            _cond.notify_all();
        }

        bool stopped() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _stopped;
        }

        void terminate() {
            std::lock_guard<std::mutex> lock(_mut);
            _terminated = true;
            _cond.notify_all();
        }

        bool terminated() const {
            std::lock_guard<std::mutex> lock(_mut);
            return _terminated;
        }

        void reset() {
            std::lock_guard<std::mutex> lock(_mut);
            _stopped = false;
            _terminated = false;
        }
    protected:
        mutable std::mutex _mut;
        std::condition_variable _cond;
        std::deque<std::shared_ptr<ValueT>> _queue;
        volatile bool _stopped{false};
        volatile bool _terminated{false};
    };

}


#endif //EASYEVENT_COMMON_CONCURRENTQUEUE_H
