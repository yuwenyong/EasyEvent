//
// Created by yuwenyong on 2021/1/23.
//

#include "EasyEvent/Shared/Loggers.h"
#include "EasyEvent/Logging/Log.h"


const std::string EasyEvent::SysLoggerName = "EasyEvent.Sys";
const std::string EasyEvent::AppLoggerName = "EasyEvent.App";


EasyEvent::Logger * EasyEvent::SysLogger() {
    static Logger* logger = Log::instance().getLogger(SysLoggerName);
    return logger;
}

EasyEvent::Logger * EasyEvent::AppLogger() {
    static Logger* logger = Log::instance().getLogger(AppLoggerName);
    return logger;
}