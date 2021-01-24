//
// Created by yuwenyong on 2020/11/23.
//

#ifndef EASYEVENT_EVENT_EVENT_H
#define EASYEVENT_EVENT_EVENT_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Errors.h"
#include "EasyEvent/Shared/Loggers.h"

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
#   include <winerror.h>
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   include <mswsock.h>
#   pragma comment(lib, "ws2_32.lib")
#   pragma comment(lib, "mswsock.lib")
#else
#   include <sys/ioctl.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <poll.h>
#   include <fcntl.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <net/if.h>
#endif

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
#   define EASY_EVENT_USE_SELECT
#elif EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_LINUX
#   include <linux/version.h>
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,45)
#       define EASY_EVENT_USE_EPOLL
#       include <sys/epoll.h>
#   else
#       define EASY_EVENT_USE_POLL
#   endif
#elif EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_APPLE || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#   define EASY_EVENT_USE_KQUEUE
#   include <sys/event.h>
#   if defined(__NetBSD__)
#       include <sys/param.h>
#   endif
#else
#   define EASY_EVENT_USE_POLL
#endif


#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
#   define EVENT_NATIVE_ERROR(e) e
#   define EVENT_SOCKET_ERROR(e) WSA ## e
#   define EVENT_NETDB_ERROR(e) WSA ## e
#   define EVENT_GETADDRINFO_ERROR(e) WSA ## e
#   define WIN_OR_POSIX(e_win, e_posix) e_win

typedef SOCKET SocketType;
typedef int SockLenType;
typedef unsigned long IoctlArgType;
const SOCKET InvalidSocket = INVALID_SOCKET;
const int SocketErrorRetVal = SOCKET_ERROR;
const int MaxAddrV4StrLen = 256;
const int MaxAddrV6StrLen = 256;
#else
#   define EVENT_NATIVE_ERROR(e) e
#   define EVENT_SOCKET_ERROR(e) e
#   define EVENT_NETDB_ERROR(e) e
#   define EVENT_GETADDRINFO_ERROR(e) e
#   define WIN_OR_POSIX(e_win, e_posix) e_posix

typedef int SocketType;
typedef socklen_t SockLenType;
typedef int IoctlArgType;
const int InvalidSocket = -1;
const int SocketErrorRetVal = -1;
const int MaxAddrV4StrLen = INET_ADDRSTRLEN;
#   if defined(INET6_ADDRSTRLEN)
const int MaxAddrV6StrLen = INET6_ADDRSTRLEN + 1 + IF_NAMESIZE;
#   else // defined(INET6_ADDRSTRLEN)
const int MaxAddrV6StrLen = 256;
#   endif
#endif

#ifndef IPV6_V6ONLY
#   define IPV6_V6ONLY 27
#endif


namespace EasyEvent {

    enum class SocketErrors {
        /// Permission denied.
        AccessDenied = EVENT_SOCKET_ERROR(EACCES),
        /// Address family not supported by protocol.
        AddressFamilyNotSupported = EVENT_SOCKET_ERROR(EAFNOSUPPORT),
        /// Address already in use.
        AddressInUse = EVENT_SOCKET_ERROR(EADDRINUSE),
        /// Transport endpoint is already connected.
        AlreadyConnected = EVENT_SOCKET_ERROR(EISCONN),
        /// Operation already in progress.
        AlreadyStarted = EVENT_SOCKET_ERROR(EALREADY),
        /// Broken pipe.
        BrokenPipe = WIN_OR_POSIX(EVENT_NATIVE_ERROR(ERROR_BROKEN_PIPE),EVENT_NATIVE_ERROR(EPIPE)),
        /// A connection has been aborted.
        ConnectionAborted = EVENT_SOCKET_ERROR(ECONNABORTED),
        /// Connection refused.
        ConnectionRefused = EVENT_SOCKET_ERROR(ECONNREFUSED),
        /// Connection reset by peer.
        ConnectionReset = EVENT_SOCKET_ERROR(ECONNRESET),
        /// Bad file descriptor.
        BadDescriptor = EVENT_SOCKET_ERROR(EBADF),
        /// Bad address.
        Fault = EVENT_SOCKET_ERROR(EFAULT),
        /// No route to host.
        HostUnreachable = EVENT_SOCKET_ERROR(EHOSTUNREACH),
        /// Operation now in progress.
        InProgress = EVENT_SOCKET_ERROR(EINPROGRESS),
        /// Interrupted system call.
        Interrupted = EVENT_SOCKET_ERROR(EINTR),
        /// Invalid argument.
        InvalidArgument = EVENT_SOCKET_ERROR(EINVAL),
        /// Message too long.
        MessageSize = EVENT_SOCKET_ERROR(EMSGSIZE),
        /// The name was too long.
        NameTooLong = EVENT_SOCKET_ERROR(ENAMETOOLONG),
        /// Network is down.
        NetworkDown = EVENT_SOCKET_ERROR(ENETDOWN),
        /// Network dropped connection on reset.
        NetworkReset = EVENT_SOCKET_ERROR(ENETRESET),
        /// Network is unreachable.
        NetworkUnreachable = EVENT_SOCKET_ERROR(ENETUNREACH),
        /// Too many open files.
        NoDescriptors = EVENT_SOCKET_ERROR(EMFILE),
        /// No buffer space available.
        NoBufferSpace = EVENT_SOCKET_ERROR(ENOBUFS),
        /// Cannot allocate memory.
        NoMemory = WIN_OR_POSIX(EVENT_NATIVE_ERROR(ERROR_OUTOFMEMORY),EVENT_NATIVE_ERROR(ENOMEM)),
        /// Operation not permitted.
        NoPermission = WIN_OR_POSIX(EVENT_NATIVE_ERROR(ERROR_ACCESS_DENIED),EVENT_NATIVE_ERROR(EPERM)),
        /// Protocol not available.
        NoProtocolOption = EVENT_SOCKET_ERROR(ENOPROTOOPT),
        /// No such device.
        NoSuchDevice = WIN_OR_POSIX(EVENT_NATIVE_ERROR(ERROR_BAD_UNIT),EVENT_NATIVE_ERROR(ENODEV)),
        /// Transport endpoint is not connected.
        NotConnected = EVENT_SOCKET_ERROR(ENOTCONN),
        /// Socket operation on non-socket.
        NotSocket = EVENT_SOCKET_ERROR(ENOTSOCK),
        /// Operation cancelled.
        OperationAborted = WIN_OR_POSIX(EVENT_NATIVE_ERROR(ERROR_OPERATION_ABORTED),EVENT_NATIVE_ERROR(ECANCELED)),
        /// Operation not supported.
        OperationNotSupported = EVENT_SOCKET_ERROR(EOPNOTSUPP),
        /// Cannot send after transport endpoint shutdown.
        ShutDown = EVENT_SOCKET_ERROR(ESHUTDOWN),
        /// Connection timed out.
        TimedOut = EVENT_SOCKET_ERROR(ETIMEDOUT),
        /// Resource temporarily unavailable.
        TryAgain = WIN_OR_POSIX(EVENT_NATIVE_ERROR(ERROR_RETRY),EVENT_NATIVE_ERROR(EAGAIN)),
        /// The socket is marked non-blocking and the requested operation would block.
        WouldBlock = EVENT_SOCKET_ERROR(EWOULDBLOCK)
    };

    enum class NetDBErrors {
        /// Host not found (authoritative).
        HostNotFound = EVENT_NETDB_ERROR(HOST_NOT_FOUND),
        /// Host not found (non-authoritative).
        HostNotFoundTryAgain = EVENT_NETDB_ERROR(TRY_AGAIN),
        /// The query is valid but does not have associated address data.
        NoData = EVENT_NETDB_ERROR(NO_DATA),
        /// A non-recoverable error occurred.
        NoRecovery = EVENT_NETDB_ERROR(NO_RECOVERY)
    };

    enum class AddrInfoErrors {
        /// The service is not supported for the given socket type.
        ServiceNotFound = WIN_OR_POSIX(EVENT_NATIVE_ERROR(WSATYPE_NOT_FOUND),EVENT_GETADDRINFO_ERROR(EAI_SERVICE)),
        /// The socket type is not supported.
        SocketTypeNotSupported = WIN_OR_POSIX(EVENT_NATIVE_ERROR(WSAESOCKTNOSUPPORT),EVENT_GETADDRINFO_ERROR(EAI_SOCKTYPE))
    };

    enum class EventErrors {
        AlreadyStarted = 1,
        AlreadyReading = 2,
        UnsatisfiableRead = 3,
        ReadCallbackFailed = 4,
        WriteCallbackFailed = 5,
        CloseCallbackFailed = 6,
        ConnectionClosed = 7,
        ConnectionBufferFull = 8,
        AlreadyListening = 10,
        CallbackNotFound = 11,
        AlreadyConnecting = 12,
        ConnectTimeout = 13,
    };

    class EASY_EVENT_API SocketErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    private:
        static const char* strerror_result(int, const char* s) {
            return s;
        }

        static const char* strerror_result(const char* s, const char*) {
            return s;
        }
    };

    class EASY_EVENT_API EventErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getSocketErrorCategory();

    inline const std::error_category& getNetDBErrorCategory() {
        return getSocketErrorCategory();
    }

    inline const std::error_category& getAddrInfoErrorCategory() {
        return getSocketErrorCategory();
    }

    EASY_EVENT_API const std::error_category& getEventErrorCategory();

    inline std::error_code make_error_code(SocketErrors err) {
        return {static_cast<int>(err), getSocketErrorCategory()};
    }

    inline std::error_code make_error_code(NetDBErrors err) {
        return {static_cast<int>(err), getNetDBErrorCategory()};
    }

    inline std::error_code make_error_code(AddrInfoErrors err) {
        return {static_cast<int>(err), getAddrInfoErrorCategory()};
    }

    inline std::error_code make_error_code(EventErrors err) {
        return {static_cast<int>(err), getEventErrorCategory()};
    }

#if defined(EASY_EVENT_USE_SELECT)
    enum IOEvents: unsigned int {
        IO_EVENT_NONE = 0,
        IO_EVENT_READ = 0x01,
        IO_EVENT_WRITE = 0x02,
        IO_EVENT_ERROR = 0x04,
    };
#elif defined(EASY_EVENT_USE_EPOLL)
    enum IOEvents: unsigned int {
        IO_EVENT_NONE = 0,
        IO_EVENT_READ = EPOLLIN,
        IO_EVENT_WRITE = EPOLLOUT,
        IO_EVENT_ERROR = EPOLLERR | EPOLLHUP,
    };
#elif defined(EASY_EVENT_USE_KQUEUE)
    enum IOEvents: unsigned int {
        IO_EVENT_NONE = 0,
        IO_EVENT_READ = 0x01,
        IO_EVENT_WRITE = 0x02,
        IO_EVENT_ERROR = 0x04,
    };
#else
    enum IOEvents: short {
        IO_EVENT_NONE = 0,
        IO_EVENT_READ = POLLIN,
        IO_EVENT_WRITE = POLLOUT,
        IO_EVENT_ERROR = POLLERR | POLLHUP,
    };
#endif

    inline IOEvents operator|(IOEvents lhs, IOEvents rhs) {
        return static_cast<IOEvents>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
    }

    inline IOEvents& operator|=(IOEvents& lhs, IOEvents rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    inline IOEvents operator&(IOEvents lhs, IOEvents rhs) {
        return static_cast<IOEvents>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
    }

    inline IOEvents& operator&=(IOEvents& lhs, IOEvents rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    class EASY_EVENT_API Selectable {
    public:
        virtual void handleEvents(IOEvents events) = 0;
        virtual SocketType getFD() const = 0;
        virtual void closeFD() = 0;
        virtual ~Selectable() noexcept = default;
    };

    using SelectablePtr = std::shared_ptr<Selectable>;

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    class WinFdSetAdapter {
    public:
        enum {
            DefaultFdSetSize = 1024
        };

        WinFdSetAdapter()
                : _capacity(DefaultFdSetSize) {
            _fdSet = static_cast<WinFdSet*>(::operator new(sizeof(WinFdSet) - sizeof(SocketType)
                    + sizeof(SocketType) * _capacity));
            _fdSet->fd_count = 0;
        }

        ~WinFdSetAdapter() {
            ::operator delete(_fdSet);
        }

        WinFdSetAdapter(const WinFdSetAdapter& fdSetAdapter) {
            _capacity = std::max<u_int>(DefaultFdSetSize, fdSetAdapter.size());
            _fdSet = static_cast<WinFdSet*>(::operator new(sizeof(WinFdSet) - sizeof(SocketType)
                    + sizeof(SocketType) * _capacity));
            for (u_int i = 0; i < fdSetAdapter.size(); ++i) {
                _fdSet->fd_array[i] = fdSetAdapter.get(i);
            }
            _fdSet->fd_count = fdSetAdapter.size();
        }

        WinFdSetAdapter& operator=(const WinFdSetAdapter& fdSetAdapter) {
            if (this == &fdSetAdapter) {
                return *this;
            }
            _fdSet->fd_count = 0;
            reserve(fdSetAdapter.size());
            for (u_int i = 0; i < fdSetAdapter.size(); ++i) {
                _fdSet->fd_array[i] = fdSetAdapter.get(i);
            }
            _fdSet->fd_count = fdSetAdapter.size();
            return *this;
        }

        void reset() {
            _fdSet->fd_count = 0;
        }

        bool set(SocketType descriptor) {
            for (u_int i = 0; i < _fdSet->fd_count; ++i) {
                if (_fdSet->fd_array[i] == descriptor) {
                    return false;
                }
            }
            reserve(_fdSet->fd_count + 1);
            _fdSet->fd_array[_fdSet->fd_count++] = descriptor;
            return true;
        }

        bool clr(SocketType descriptor) {
            for (u_int i = 0; i < _fdSet->fd_count; ++i) {
                if (_fdSet->fd_array[i] == descriptor) {
                    if (i != _fdSet->fd_count - 1) {
                        _fdSet->fd_array[i] = _fdSet->fd_array[_fdSet->fd_count - 1];
                    }
                    --_fdSet->fd_count;
                    return true;
                }
            }
            return false;
        }

        u_int size() const {
            return _fdSet->fd_count;
        }

        SocketType get(u_int i) const {
            return _fdSet->fd_array[i];
        }

        operator fd_set*() {
            return reinterpret_cast<fd_set*>(_fdSet);
        }
    protected:
        struct WinFdSet {
            u_int fd_count;
            SocketType fd_array[1];
        };

        void reserve(u_int n) {
            if (n < _capacity) {
                return;
            }
            u_int  newCapacity = _capacity + _capacity / 2;
            if (newCapacity < n) {
                newCapacity = n;
            }
            WinFdSet* newFdSet = static_cast<WinFdSet*>(::operator new(sizeof(WinFdSet) - sizeof(SocketType)
                    + sizeof(SocketType) * newCapacity));
            newFdSet->fd_count = _fdSet->fd_count;
            for (u_int i = 0; i < _fdSet->fd_count; ++i) {
                newFdSet->fd_array[i] = _fdSet->fd_array[i];
            }
            ::operator delete(_fdSet);
            _fdSet = newFdSet;
            _capacity = newCapacity;
        }

        WinFdSet* _fdSet;
        u_int _capacity;
    };
#endif

}

namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::SocketErrors>: public true_type {};

    template <>
    struct is_error_code_enum<EasyEvent::NetDBErrors>: public true_type {};

    template <>
    struct is_error_code_enum<EasyEvent::AddrInfoErrors>: public true_type {};

    template <>
    struct is_error_code_enum<EasyEvent::EventErrors>: public true_type {};
}

#endif //EASYEVENT_EVENT_EVENT_H
