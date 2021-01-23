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
    private:
        struct MakeSharedTag {};
    public:
        Connection(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize, size_t maxWriteBufferSize,
                   MakeSharedTag tag)
            : Connection(ioLoop, socket, maxReadBufferSize, maxWriteBufferSize) {

        }

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        ~Connection() noexcept override;

        void handleEvents(IOEvents events) override;

        SocketType getFD() const override;

        void closeFD() override;

        void connect(const Address& address, Task<void(std::error_code)>&& callback);

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

        bool reading() const {
            return (bool)_readCallback;
        }

        bool writing() const {
            return !_writeBuffer.empty();
        }

        bool closed() const {
            return _closed;
        }

        void setNoDelay(bool value) {
            SocketOps::SetTcpNoDelay(_socket, value);
        }

        std::string getLocalIP() const {
            Address address;
            SocketOps::GetSockName(_socket, address);
            return address.getAddrString();
        }

        unsigned short getLocalPort() const {
            Address address;
            SocketOps::GetSockName(_socket, address);
            return address.getPort();
        }

        Address getLocalAddress() const {
            Address address;
            SocketOps::GetSockName(_socket, address);
            return address;
        }

        std::string getRemoteIP() const {
            Address address;
            SocketOps::GetPeerName(_socket, address);
            return address.getAddrString();
        }

        unsigned short getRemotePort() const {
            Address address;
            SocketOps::GetPeerName(_socket, address);
            return address.getPort();
        }

        Address getRemoteAddress() const {
            Address address;
            SocketOps::GetPeerName(_socket, address);
            return address;
        }

        static std::shared_ptr<Connection> create(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize=0,
                                                  size_t maxWriteBufferSize=0) {
            return std::make_shared<Connection>(ioLoop, socket, maxReadBufferSize, maxWriteBufferSize, MakeSharedTag{});
        }

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

         int getFdError(std::error_code& ec) {
            int error = 0;
            SocketOps::GetSockError(_socket, error, ec);
            return error;
        }

        Connection(IOLoop* ioLoop, SocketType socket, size_t maxReadBufferSize, size_t maxWriteBufferSize);

        ssize_t writeToFd(const void* data, size_t size, std::error_code& ec) {
            return SocketOps::Send(_socket, data, size, 0, ec);
        }

        ssize_t readFromFd(void* buf, size_t size, std::error_code& ec) {
            return SocketOps::Recv(_socket, buf, size, 0, ec);
        }

        void maybeRunCloseCallback();

        size_t readToBufferLoop();

        void handleRead();

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

        void handleWrite();

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

        void runConnectCallback(std::error_code ec);

        void handleConnect();

        bool isWouldBlock(const std::error_code& ec) const {
            return ec == SocketErrors::WouldBlock || ec == SocketErrors::TryAgain;
        }

        bool isConnReset(const std::error_code& ec) const {
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
