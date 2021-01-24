//
// Created by yuwenyong on 2021/1/9.
//

#include "EasyEvent/Logging/LogRecord.h"
#include "EasyEvent/Logging/Sink.h"


void EasyEvent::LogRecord::writeAsync() {
    Assert(_sink != nullptr);
    _sink->writeQueued(this);
}