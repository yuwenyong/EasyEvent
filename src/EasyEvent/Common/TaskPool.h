//
// Created by yuwenyong on 2020/11/7.
//

#ifndef EASYEVENT_TASKPOOL_H
#define EASYEVENT_TASKPOOL_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/ConcurrentQueue.h"


namespace EasyEvent {


    class EASY_EVENT_API TaskPool {
    public:
        TaskPool(const TaskPool&) = delete;
        TaskPool& operator=(const TaskPool&) = delete;

        ~TaskPool();

        bool start() {
            return start(std::thread::hardware_concurrency());
        }

        bool start(size_t threadCount);

        template <typename FuncT>
        std::future<typename std::result_of<FuncT()>::type> submit(FuncT &&func) {
            using ResultType = typename std::result_of<FuncT()>::type;
            std::packaged_task<ResultType ()> task(std::forward<FuncT>(func));
            std::future<ResultType > res(task.get_future());
            if (_tasks.enqueue(makeTask(std::move(task)))) {
                return res;
            } else {
                return {};
            }
        }

        bool empty() const {
            return _tasks.empty();
        }

        void stop() {
            _tasks.stop();
        }

        bool stopped() const {
            return _tasks.stopped();
        }

        void terminate() {
            _tasks.terminate();
        }

        bool terminated() const {
            return _tasks.terminated();
        }

        void reset() {
            return _tasks.reset();
        }

        void wait() {
            for (auto &thread: _threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            _threads.clear();
        }
    protected:

        class TaskBase {
        public:
            virtual void execute() = 0;
            virtual ~TaskBase() = default;
        };

        template <typename FuncT>
        class Task: public TaskBase {
        public:
            explicit Task(FuncT &&func): _func(std::forward<FuncT>(func)) {}

            void execute() override {
                _func();
            }
        protected:
            FuncT _func;
        };

        void process();

        template <typename FuncT>
        static std::unique_ptr<TaskBase> makeTask(FuncT &&func) {
            return std::make_unique<Task<FuncT>>(std::forward<FuncT>(func));
        }

        ConcurrentQueue<std::unique_ptr<TaskBase>> _tasks;
        std::vector<std::thread> _threads;
    };


    template <typename ValueT>
    bool isReady(const std::future<ValueT> &fut) {
        return fut.wait_for(std::chrono::milliseconds::zero()) == std::future_status::ready;
    }
}


#endif //EASYEVENT_TASKPOOL_H
