//
// Created by yuwenyong on 2020/10/27.
//


#include "EasyEvent/EasyEvent.h"

using namespace EasyEvent;

void test(IOLoop* ioLoop, Logger* logger) {
    static int i = 0;
    LOG_ERROR(logger) << "This is a test log " << i++ << ";";
    ioLoop->callLater(Time::seconds(2), [ioLoop, logger]() {
        test(ioLoop, logger);
    });
}

int main (int argc, char **argv) {
    Logger* logger = Log::instance().createLogger("Test", LOG_LEVEL_DEBUG, LOGGER_FLAGS_ASYNC);
    assert(logger != nullptr);
    logger->addSink(ConsoleSink::create(true, LOG_LEVEL_DEBUG, (SinkFlags)(SINK_FLAGS_DEFAULT|SINK_FLAGS_PREFIX_LOGGER_NAME)));
//    logger->addSink(FileSink::create("./test.log", true));
//    logger->addSink(RotatingFileSink::create("./rtest.log", 1024, 3));
//    logger->addSink(TimedRotatingFileSink::create("./trtest.log", TimedRotatingWhen::Minute));

    std::error_code ec = SocketErrors::Interrupted;
    LOG_ERROR(logger) << ec << "(" << ec.message() << ")";

    auto addrs = Resolver::getAddresses("", 2, EnableBoth, false, true);
    for (auto& addr: addrs) {
        LOG_INFO(logger) << addr;
    }

    IOLoop ioLoop(logger, true, true);
    ioLoop.addCallback([logger]() {
        int i = 1;
        LOG_DEBUG(logger) << "Task " << i;
    });
    ioLoop.addCallback([logger]() {
        int i = 2;
        LOG_INFO(logger) << "Task " << i;
    });
    ioLoop.addCallback([logger]() {
        int i = 3;
        LOG_WARN(logger) << "Task " << i;
    });
    ioLoop.addCallback([logger]() {
        int i = 4;
        LOG_ERROR(logger) << "Task " << i;
    });
    ioLoop.addCallback([logger]() {
        LOG_CRITICAL(logger) << "Last task";
    });
    ioLoop.resolve("localhost", 200, EnableBoth, false,
                   [logger](std::vector<Address> addrs, std::error_code ec) {
        if (!ec) {
            for (auto& addr: addrs) {
                LOG_INFO(logger) << "Local: " << addr;
            }
        } else {
            LOG_ERROR(logger) << ec;
        }
    });

    ioLoop.resolve("www.baidu.com", 200, EnableBoth, true,
                   [logger](std::vector<Address> addrs, std::error_code ec) {
                       if (!ec) {
                           for (auto& addr: addrs) {
                               LOG_INFO(logger) << "Baidu: " << addr;
                           }
                       } else {
                           LOG_ERROR(logger) << ec;
                       }
                   });

    ioLoop.callLater(Time::seconds(2), [logger, &ioLoop]() {
        test(&ioLoop, logger);
    });
    ioLoop.start();
    return 0;
}
