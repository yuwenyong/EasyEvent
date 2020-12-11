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
