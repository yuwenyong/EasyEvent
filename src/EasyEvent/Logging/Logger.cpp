//
// Created by yuwenyong on 2020/11/7.
//

#include "EasyEvent/Logging/Logger.h"
#include "EasyEvent/Logging/Sink.h"


void EasyEvent::Logger::_write(LogMessage *message) {
    for (auto &sink: _sinks) {
        sink->write(message);
    }
}