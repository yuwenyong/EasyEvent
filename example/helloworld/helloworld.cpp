//
// Created by yuwenyong on 2020/10/27.
//


#include "EasyEvent/EasyEvent.h"

using namespace EasyEvent;


int main (int argc, char **argv) {
    std::error_code ec = {};
    Logger* logger = Log::instance().createLogger("Test", LOG_LEVEL_DEBUG, LOGGER_FLAGS_ASYNC, ec);
    assert(logger != nullptr);
    logger->addSink(ConsoleSink::create(true, LOG_LEVEL_DEBUG, (SinkFlags)(SINK_FLAGS_DEFAULT|SINK_FLAGS_PREFIX_LOGGER_NAME)));
//    logger->addSink(FileSink::create("./test.log", true));
//    logger->addSink(RotatingFileSink::create("./rtest.log", 1024, 3));
//    logger->addSink(TimedRotatingFileSink::create("./trtest.log", TimedRotatingWhen::Minute));
    EasyEvent::TaskPool taskPool;
    taskPool.start(1);
    auto v1 = taskPool.submit([logger]() {
        int i = 1;
        LOG_DEBUG(logger) << "Task " << i;
        return 5;
    });
    auto v2 = taskPool.submit([logger]() {
        int i = 2;
        LOG_INFO(logger) << "Task " << i;
        return 6;
    });
    auto v3 = taskPool.submit([logger]() {
        int i = 3;
        LOG_WARN(logger) << "Task " << i;
    });

    taskPool.submit([logger]() {
        int i = 4;
        LOG_ERROR(logger) << "Task " << i;
        return 8;
    });
    auto result = taskPool.post([logger]() {
        LOG_CRITICAL(logger) << "Last task";
    });
    assert(result);
    taskPool.stop();
    LOG_INFO(logger) << "Task 1: " << v1.get();
    LOG_INFO(logger) << "Task 2: " << v2.get();
    LOG_INFO(logger) << "Task 3: " << (isReady(v3) ? 1 : 0);
    LOG_CRITICAL(logger) << "Last ....";
    taskPool.wait();

    std::unique_ptr<int> a = std::make_unique<int>(6);
    Task<void ()> t = [logger, a=std::move(a)]() {
        LOG_INFO(logger) << "Custom task: " << *a;
        return 0;
    };
    t();

//    int  i = 0;
//    while (true) {
//        LOG_ERROR(logger) << "This is a test log " << ++i << ";";
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//        if (i > 100) {
//            break;
//        }
//    }
    return 0;
}
