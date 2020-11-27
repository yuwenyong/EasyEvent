//
// Created by yuwenyong on 2020/11/24.
//

#include "EasyEvent/Event/IOLoop.h"
#include "EasyEvent/Logging/LogStream.h"


thread_local EasyEvent::IOLoop* EasyEvent::IOLoop::_current = nullptr;

void EasyEvent::IOLoop::close(bool allSockets) {

}

void EasyEvent::IOLoop::addHandler(const SelectablePtr &handler, IOEvents events) {

}

void EasyEvent::IOLoop::updateHandler(const SelectablePtr &handler, IOEvents events) {

}

void EasyEvent::IOLoop::removeHandler(const SelectablePtr &handler) {

}

void EasyEvent::IOLoop::start() {

}

void EasyEvent::IOLoop::handleCallbackException(std::exception &e) {
    LOG_ERROR(_logger) << "Exception in callback: " << e.what();
}