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

void EasyEvent::WriteBuffer::write(std::string &&data) {
    _size += data.size();
    if (_secondaryBuffers.empty() &&
        data.size() <= MoveBufferThreshold &&
        _primaryBuffer.getRemainingSpace() >= MoveBufferThreshold) {
        _primaryBuffer.write(data.data(), data.size());
    } else {
        _secondaryBuffers.emplace_back(std::move(data));
    }
}

void EasyEvent::WriteBuffer::write(std::vector<int8> &&data) {
    _size += data.size();
    if (_secondaryBuffers.empty() &&
        data.size() <= MoveBufferThreshold &&
        _primaryBuffer.getRemainingSpace() >= MoveBufferThreshold) {
        _primaryBuffer.write(data.data(), data.size());
    } else {
        _secondaryBuffers.emplace_back(std::move(data));
    }
}

void EasyEvent::WriteBuffer::write(std::vector<uint8> &&data) {
    _size += data.size();
    if (_secondaryBuffers.empty() &&
        data.size() <= MoveBufferThreshold &&
        _primaryBuffer.getRemainingSpace() >= MoveBufferThreshold) {
        _primaryBuffer.write(data.data(), data.size());
    } else {
        _secondaryBuffers.emplace_back(std::move(data));
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

void EasyEvent::WriteBuffer::advance(size_t size) {
    Assert(0 < size && size <= _size);
    _size -= size;
    if (_primaryBuffer.getActiveSize() > 0) {
        if (_primaryBuffer.getActiveSize() < size) {
            size -= _primaryBuffer.getActiveSize();
            _primaryBuffer.reset();
        } else {
            _primaryBuffer.readCompleted(size);
            size = 0;
        }
    }
    while (size > 0) {
        std::visit([&size, this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                if (arg.size() - _firstPos <= size) {
                    size -= (arg.size() - _firstPos);
                    _firstPos = 0;
                } else {
                    _firstPos += size;
                    size = 0;
                }
            } else if constexpr (std::is_same_v<T, std::vector<uint8>>) {
                if (arg.size() - _firstPos <= size) {
                    size -= (arg.size() - _firstPos);
                    _firstPos = 0;
                } else {
                    _firstPos += size;
                    size = 0;
                }
            } else if constexpr (std::is_same_v<T, std::vector<int8>>) {
                if (arg.size() - _firstPos <= size) {
                    size -= (arg.size() - _firstPos);
                    _firstPos = 0;
                } else {
                    _firstPos += size;
                    size = 0;
                }
            } else {
                static_assert(FailType<T>{});
            }
        }, _secondaryBuffers.front());
        Assert(_firstPos == 0 || size == 0);
        if (_firstPos == 0) {
            _secondaryBuffers.pop_front();
        }
    }
    Assert(size == 0);
}

void * EasyEvent::WriteBuffer::getReadPtr() {
    if (empty()) {
        return nullptr;
    }
    if (_primaryBuffer.getActiveSize()) {
        return _primaryBuffer.getReadPointer();
    }
    Assert(!_secondaryBuffers.empty());
    void* ptr = nullptr;
    std::visit([&ptr, this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            ptr = arg.data() + _firstPos;
        } else if constexpr (std::is_same_v<T, std::vector<uint8>>) {
            ptr = arg.data() + _firstPos;
        } else if constexpr (std::is_same_v<T, std::vector<int8>>) {
            ptr = arg.data() + _firstPos;
        } else {
            static_assert(FailType<T>{});
        }
    }, _secondaryBuffers.front());
    Assert(ptr != nullptr);
    return ptr;
}

size_t EasyEvent::WriteBuffer::getReadSize() const {
    if (empty()) {
        return 0;
    }
    if (_primaryBuffer.getActiveSize()) {
        return _primaryBuffer.getActiveSize();
    }
    Assert(!_secondaryBuffers.empty());
    size_t size = 0;
    std::visit([&size, this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            size = arg.size() - _firstPos;
        } else if constexpr (std::is_same_v<T, std::vector<uint8>>) {
            size = arg.size() - _firstPos;
        } else if constexpr (std::is_same_v<T, std::vector<int8>>) {
            size = arg.size() - _firstPos;
        } else {
            static_assert(FailType<T>{});
        }
    }, _secondaryBuffers.front());
    Assert(size > 0);
    return size;
}