//
// Created by yuwenyong on 2020/12/11.
//

#include "EasyEvent/Common/Buffer.h"


void EasyEvent::Buffer::prepare(size_t size) {
    if (getRemainingSpace() < size) {
        normalize();
        if (getRemainingSpace() < size) {
            size_t newSize = _capacity * 2;
            if (newSize < getActiveSize() + size) {
                newSize = getActiveSize() + size;
            }
            _storage = (uint8*)::realloc(_storage, newSize);
            _capacity = newSize;
        }
    }
}


void EasyEvent::WriteBuffer::write(void const*data, size_t size) {
    _size += size;
    if (_secondaryBuffers.empty() && size <= _primaryBuffer.getRemainingSpace()) {
        _primaryBuffer.write(data, size);
    } else {
        std::string tmp((const char*)data, size);
        _secondaryBuffers.emplace_back(std::move(tmp));
    }
}