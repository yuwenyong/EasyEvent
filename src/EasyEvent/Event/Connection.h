//
// Created by yuwenyong on 2020/12/12.
//

#ifndef EASYEVENT_EVENT_CONNECTION_H
#define EASYEVENT_EVENT_CONNECTION_H

#include "EasyEvent/Event/EventBase.h"
#include "EasyEvent/Event/IOLoop.h"
#include "EasyEvent/Common/Buffer.h"


namespace EasyEvent {

    class EASY_EVENT_API Connection: public Selectable, public std::enable_shared_from_this<Connection> {
    protected:
        struct MakeSharedTag {};
    public:
        Connection(IOLoop* ioLoop, size_t maxReadBufferSize, size_t maxWriteBufferSize, MakeSharedTag tag);

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        virtual ~Connection() noexcept = default;

        IOLoop* getIOLoop() {
            return _ioLoop;
        }

        Logger* getLogger() {
            return _ioLoop->getLogger();
        }

        size_t getMaxReadBufferSize() const {
            return _maxReadBufferSize;
        }

        size_t getMaxWriteBufferSize() const {
            return _maxWriteBufferSize;
        }

        void handleEvents(IOEvents events) override;

        virtual void connect(const Address& address, Task<void(std::error_code)>&& callback) = 0;

        void readUntilRegex(const std::string& regex, Task<void(std::string)>&& callback, size_t maxBytes=0);

        void readUntil(std::string delimiter, Task<void(std::string)>&& callback, size_t maxBytes=0);

        void readBytes(size_t numBytes, Task<void(std::string)>&& callback, bool partial=false);

        void write(const void* data, size_t size, Task<void()>&& callback= nullptr);

        void write(const char* data, Task<void()>&& callback= nullptr) {
            write(data, strlen(data), std::move(callback));
        }

        void write(const std::string& data, Task<void()>&& callback= nullptr) {
            write(data.data(), data.size(), std::move(callback));
        }

        void write(const std::vector<int8>& data, Task<void()>&& callback= nullptr) {
            write(data.data(), data.size(), std::move(callback));
        }

        void write(const std::vector<uint8>& data, Task<void()>&& callback= nullptr) {
            write(data.data(), data.size(), std::move(callback));
        }

        void write(std::string&& data, Task<void()>&& callback= nullptr);

        void write(std::vector<int8>&& data, Task<void()>&& callback= nullptr);

        void write(std::vector<uint8>&& data, Task<void()>&& callback= nullptr);

        void setCloseCallback(Task<void(std::error_code)>&& callback) {
            _closeCallback  = std::move(callback);
            maybeAddErrorListener();
        }

        void close(const std::error_code& error={});

        virtual bool reading() const;

        virtual bool writing() const;

        bool closed() const {
            return _closed;
        }

        virtual void setNoDelay(bool value) = 0;

        virtual std::string getLocalIP() const = 0;

        virtual unsigned short getLocalPort() const = 0;

        virtual Address getLocalAddress() const = 0;

        virtual std::string getRemoteIP() const = 0;

        virtual unsigned short getRemotePort() const = 0;

        virtual Address getRemoteAddress() const = 0;


        static constexpr size_t DefaultMaxReadBufferSize = 104857600;
        static constexpr size_t DefaultReadBufferCapacity = 8192;
        static constexpr size_t DefaultReadChunkSize = 4096;
        static constexpr size_t InitialWriteBufferSize = 4096;

    protected:
        class LocalAddErrorListener {
        public:
            LocalAddErrorListener(Connection* conn)
                : _conn(conn) {

            }

            ~LocalAddErrorListener() {
                _conn->maybeAddErrorListener();
            }
        protected:
            Connection* _conn;
        };

        virtual ssize_t writeToFd(const void* data, size_t size, std::error_code& ec) = 0;

        virtual ssize_t readFromFd(void* buf, size_t size, std::error_code& ec) = 0;

        virtual int getFdError(std::error_code& ec) = 0;


        void maybeRunCloseCallback();

        size_t readToBufferLoop();

        virtual void handleRead();

        void setReadCallback(Task<void(std::string)>&& task);

        void runReadCallback(size_t size);

        void tryInlineRead();

        size_t readToBuffer();

        void readFromBuffer(size_t pos);

        size_t findReadPos();

        void checkMaxBytes(size_t size) const {
            if (_readMaxBytes > 0 && size > _readMaxBytes) {
                std::error_code ec = EventErrors::UnsatisfiableRead;
                throwError(ec, "Connection");
            }
        }

        virtual void handleWrite();

        std::string consume(size_t size) {
            if (size == 0) {
                return "";
            }
            Assert(size <= _readBuffer.getActiveSize());
            std::string s = {(char*)_readBuffer.getReadPointer(), (char*)_readBuffer.getReadPointer() + size};
            _readBuffer.readCompleted(size);
            return s;
        }

        void checkClosed() const {
            if (closed()) {
                std::error_code ec = EventErrors::ConnectionClosed;
                throwError(ec, "Connection");
            }
        }

        void maybeAddErrorListener();

        void addIOState(IOEvents state);

        virtual void handleConnect() = 0;

        bool isWouldBlock(const std::error_code& ec) const {
            return ec == SocketErrors::WouldBlock || ec == SocketErrors::TryAgain;
        }

        virtual bool isConnReset(const std::error_code& ec) const {
            return ec == SocketErrors::ConnectionReset ||
                ec == SocketErrors::ConnectionAborted ||
                ec == SocketErrors::BrokenPipe ||
                ec == SocketErrors::TimedOut;
        }

        bool isInProgress(const std::error_code& ec) const {
            return ec == SocketErrors::InProgress;
        }

        void checkWriteBuffer(size_t size) {
            if (_maxWriteBufferSize != 0 && _writeBuffer.size() + size > _maxWriteBufferSize) {
                std::error_code ec = EventErrors::ConnectionBufferFull;
                throwError(ec, "Connection");
            }
        }

        IOLoop* _ioLoop;
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
        Task<void()> _writeCallback;
        Task<void(std::error_code)> _closeCallback;
        Task<void(std::error_code)> _connectCallback;
        bool _connecting{false};
        bool _closed{false};
        IOEvents _state{IO_EVENT_NONE};
        int _pendingCallbacks{0};
    };

    using ConnectionPtr = std::shared_ptr<Connection>;
    using ConnectionHolder = Holder<Connection>;

}

#endif //EASYEVENT_EVENT_CONNECTION_H
