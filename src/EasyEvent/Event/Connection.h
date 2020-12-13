//
// Created by yuwenyong on 2020/12/12.
//

#ifndef EASYEVENT_EVENT_CONNECTION_H
#define EASYEVENT_EVENT_CONNECTION_H

#include "EasyEvent/Event/Event.h"
#include "EasyEvent/Event/IOLoop.h"
#include "EasyEvent/Event/SocketOps.h"
#include "EasyEvent/Common/Buffer.h"


namespace EasyEvent {

    class EASY_EVENT_API Connection: public Selectable, public std::enable_shared_from_this<Connection> {
    public:
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        Connection(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize=0, size_t maxWriteBufferSize=0);

        ~Connection() noexcept override;

        void handleEvents(IOEvents events) override;

        SocketType getSocket() const override;

        void closeSocket() override;

        void closeSocket(std::error_code& ec) {
            SocketOps::Close(_socket, false, ec);
            if (!ec) {
                _socket = InvalidSocket;
            }
        }

        void readUntilRegex(const std::string& regex, Task<void(std::string)>&& task, size_t maxBytes,
                            std::error_code& ec);

        void readUntilRegex(const std::string& regex, Task<void(std::string)>&& task, size_t maxBytes=0) {
            std::error_code ec;
            readUntilRegex(regex, std::move(task), maxBytes, ec);
            throwError(ec, "Connection");
        }

        static constexpr size_t DefaultMaxReadBufferSize = 104857600;
        static constexpr size_t DefaultReadBufferCapacity = 8192;
        static constexpr size_t DefaultReadChunkSize = 4096;
        static constexpr size_t InitialWriteBufferSize = 4096;

    protected:
        void setReadCallback(Task<void(std::string)>&& task, std::error_code& ec);

        void runReadCallback(size_t size);

        void tryInlineRead(std::error_code& ec);

        size_t findReadPos(std::error_code& ec);

        void checkMaxBytes(size_t size, std::error_code& ec) const {
            if (_readMaxBytes > 0 && size > _readMaxBytes) {
                ec = EventErrors::UnsatisfiableRead;
            }
        }

        std::string consume(size_t size) {
            if (size == 0) {
                return "";
            }
            Assert(size <= _readBuffer.getActiveSize());
            std::string s = {(char*)_readBuffer.getReadPointer(), (char*)_readBuffer.getReadPointer() + size};
            _readBuffer.readCompleted(size);
            return s;
        }

        IOLoop* _ioLoop;
        Logger* _logger;
        SocketType _socket;
        size_t _maxReadBufferSize;
        size_t _maxWriteBufferSize;
        std::error_code _error;
        Buffer _readBuffer;
        WriteBuffer _writeBuffer;
        std::string _readDelimiter;
        std::optional<std::regex> _readRegex;
        size_t _readMaxBytes{0};
        size_t _readBytes{0};
        bool _readPartial{false};
        Task<void(std::string)> _readCallback;
        bool _connecting{false};
        bool _closed{false};
        IOEvents _state{IO_EVENT_NONE};
        int _pendingCallbacks{0};
    };

}


#endif //EASYEVENT_EVENT_CONNECTION_H
