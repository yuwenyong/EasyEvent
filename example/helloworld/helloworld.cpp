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

void testJson(Logger* logger) {
    JsonValue root{JsonType::ObjectValue};
    root["str"] = "Hello world";
    root["int"] = 3000;
    root["double"] = 10001.1023;
    root["bool"] = false;
    root["null"] = nullptr;
    root["array"] = JsonValue(JsonType::ArrayValue);
    root["array"][0] = 1;
    root["array"][1] = "stt";
    root["array"][2] = JsonValue(JsonType::ObjectValue);
    root["array"][2]["t"] = "ttt";
    root["object"] = JsonValue(JsonType::ObjectValue);
    root["object"]["key"] = "ttt";
    root["object"]["obj"] = JsonValue(JsonType::ObjectValue);
    root["object"]["obj"]["s"] = "test";
    root["object"]["obj"]["a"] = JsonValue(JsonType::ArrayValue);
    root["object"]["obj"]["a"][0] = "ttt";
    root["object"]["zzz"] = true;
    JsonValue cp, nu;
    LOG_ERROR(logger) << root;
    cp = root;
    nu = root;
    nu["object"]["key"] = "t2";
    LOG_ERROR(logger) << cp;
    if (cp == root) {
        LOG_ERROR(logger) << "cp == root";
    }  else {
        LOG_ERROR(logger) << "cp != root";
    }
    if (nu == root) {
        LOG_ERROR(logger)<< "nu == root";
    }  else {
        LOG_ERROR(logger) << "nu != root";
    }
    std::stringstream ss;
    JsonValue result;
    ss << cp;
    ss >> result;
    LOG_ERROR(logger) << "result: " << result;
}

int main (int argc, char **argv) {
//    Log::instance().configure("/home/yuwenyong/docs/logconf.json");
    Logger* logger = Log::instance().getLogger("HelloWorld.child");
    logger->setLevel(LOG_LEVEL_DEBUG);

//    logger->addSink(FileSink::create("./test.log", true));
//    logger->addSink(RotatingFileSink::create("./rtest.log", 1024, 3));
//    logger->addSink(TimedRotatingFileSink::create("./trtest.log", TimedRotatingWhen::Minute));

//    testJson(logger);

    auto addrs = Resolver::getAddresses("", 2, EnableBoth, false, true);
    for (auto& addr: addrs) {
        LOG_INFO(logger) << addr;
    }

    LOG_INFO(logger) << "Thread Id:" << std::this_thread::get_id();

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
