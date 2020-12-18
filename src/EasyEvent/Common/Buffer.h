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

        Buffer(Buffer&& rhs) noexcept
            : Buffer() {
            rhs.swap(*this);
        }

        Buffer& operator=(Buffer&& rhs) noexcept {
            rhs.swap(*this);
            return *this;
        }

        ~Buffer() {
            if (_storage) {
                ::free(_storage);
            }
        }

        void swap(Buffer& rhs) {
            std::swap(_storage, rhs._storage);
            std::swap(_capacity, rhs._capacity);
            std::swap(_wpos, rhs._wpos);
            std::swap(_rpos, rhs._rpos);
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
            if (_rpos == _wpos) {
                reset();
            }
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


    class EASY_EVENT_API WriteBuffer: private NonCopyable {
    public:
        using SecondaryBuffer = std::variant<std::string, std::vector<uint8>, std::vector<int8>>;

        WriteBuffer()
            :WriteBuffer(4096) {

        }

        explicit WriteBuffer(size_t initialSize)
            : _primaryBuffer(initialSize) {

        }

        WriteBuffer(WriteBuffer&& rhs) noexcept
            : WriteBuffer() {
            rhs.swap(*this);
        }

        WriteBuffer& operator=(WriteBuffer&& rhs) noexcept {
            rhs.swap(*this);
            return *this;
        }

        void swap(WriteBuffer& rhs) {
            _primaryBuffer.swap(rhs._primaryBuffer);
            std::swap(_secondaryBuffers, rhs._secondaryBuffers);
            std::swap(_firstPos, rhs._firstPos);
            std::swap(_size, rhs._size);
        }

        size_t size() const {
            return _size;
        }

        bool empty() const {
            return _size == 0;
        }

        void write(std::string&& data);

        void write(std::vector<uint8>&& data);

        void write(std::vector<int8>&& data);

        void write(const std::string& data) {
            write(data.data(), data.size());
        }

        void write(const std::vector<uint8>& data) {
            write(data.data(), data.size());
        }

        void write(const std::vector<int8>& data) {
            write(data.data(), data.size());
        }

        void write(void const* data, size_t size);

        void advance(size_t size);

        void* getReadPtr();

        size_t getReadSize() const;

        static constexpr size_t MoveBufferThreshold = 64;
    private:
        Buffer _primaryBuffer;
        std::deque<SecondaryBuffer> _secondaryBuffers;
        size_t _firstPos{0};
        size_t _size{0};
    };

}

#endif //EASYEVENT_COMMON_BUFFER_H
