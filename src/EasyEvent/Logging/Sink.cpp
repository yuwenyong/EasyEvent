//
// Created by yuwenyong on 2020/11/14.
//

#include "EasyEvent/Logging/Sink.h"
#include "EasyEvent/Logging/Logger.h"


EasyEvent::Sink::~Sink() {
    close();
}

void EasyEvent::Sink::write(LogRecord *record) {
    if (!shouldLog(record)) {
        return;
    }
    if (_multiThread) {
        std::lock_guard<std::mutex> lock(*_mutex);
        doWrite(record);
    } else {
        doWrite(record);
    }
}

void EasyEvent::Sink::onClose() {

}

void EasyEvent::Sink::doClose() {
    if (!_closed) {
        if (_queue) {
            _queue->stop();
            _queue->wait();
            _queue.reset();
        }
        onClose();
        _closed = true;
    }
}

void EasyEvent::Sink::doWrite(LogRecord *record) {
    if (_closed) {
        return;
    }
    if (_async) {
        writeAsync(record);
    } else {
        writeSync(record);
    }
}

void EasyEvent::Sink::writeSync(LogRecord *record) {
    std::string text = _formatter.format(record);
    onWrite(record, text);
}

void EasyEvent::Sink::writeAsync(LogRecord *record) {
    ensureQueueCreated();
    auto clonedRecord = record->clone(this);
    _queue->post([record=std::move(clonedRecord)]() mutable {
        record->writeAsync();
    });
}

void EasyEvent::Sink::ensureQueueCreated() {
    if (!_queue) {
        _queue = std::make_unique<TaskPool>();
        _queue->start(1);
    }
}

