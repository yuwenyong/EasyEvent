//
// Created by yuwenyong on 2020/12/11.
//

#ifndef EASYEVENT_COMMON_BUFFER_H
#define EASYEVENT_COMMON_BUFFER_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/NonCopyable.h"


namespace EasyEvent {

    class EASY_EVENT_API Buffer: private NonCopyable {
    public:
        Buffer()
            : Buffer(4096) {

        }

        explicit Buffer(size_t capacity)
            : _capacity(capacity) {
            _storage = (uint8*)malloc(capacity);
        }

        Buffer(Buffer&& rhs) noexcept {
            _storage = rhs._storage;
            _capacity = rhs._capacity;
            _wpos = rhs._wpos;
            _rpos = rhs._rpos;
            rhs._storage = nullptr;
            rhs._capacity = 0;
            rhs.reset();
        }

        Buffer& operator=(Buffer&& rhs) noexcept {
            if (_storage) {
                ::free(_storage);
            }
            _storage = rhs._storage;
            _capacity = rhs._capacity;
            _wpos = rhs._wpos;
            _rpos = rhs._rpos;

            rhs._storage = nullptr;
            rhs._capacity = 0;
            rhs.reset();
            return *this;
        }

        ~Buffer() {
            if (_storage) {
                ::free(_storage);
            }
        }

        void reset() {
            _wpos = 0;
            _rpos = 0;
        }

        void prepare(size_t size);

        void normalize() {
            if (_rpos && _rpos != _wpos) {
                memmove(getBasePointer(), getReadPointer(), getActiveSize());
                _wpos -= _rpos;
                _rpos = 0;
            }
        }

        uint8* getBasePointer() {
            Assert(_storage != nullptr);
            return _storage;
        }

        uint8* getReadPointer() {
            return getBasePointer() + _rpos;
        }

        uint8* getWritePointer() {
            return getBasePointer() + _wpos;
        }

        void readCompleted(size_t bytes) {
            _rpos += bytes;
            Assert(_capacity >= _rpos);
        }

        void writeCompleted(size_t bytes) {
            _wpos += bytes;
            Assert(_capacity >= _wpos);
        }

        size_t getActiveSize() const {
            Assert(_wpos >= _rpos);
            return _wpos - _rpos;
        }

        size_t getRemainingSpace() const {
            Assert(_capacity >= _wpos);
            return _capacity - _wpos;
        }

        void write(void const* data, size_t size) {
            if (size) {
                prepare(size);
                ::memcpy(getWritePointer(), data, size);
                writeCompleted(size);
            }
        }

        size_t read(void* buf, size_t bufSize) {
            size_t size = bufSize < getActiveSize() ? bufSize : getActiveSize();
            if (size) {
                ::memcpy(buf, getReadPointer(), size);
                readCompleted(size);
            }
            return size;
        }

    private:
        uint8* _storage;
        size_t _capacity;
        size_t _wpos{0};
        size_t _rpos{0};
    };

}

#endif //EASYEVENT_COMMON_BUFFER_H
