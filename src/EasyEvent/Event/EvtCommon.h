//
// Created by yuwenyong on 2020/11/23.
//

#ifndef EASYEVENT_EVENT_EVTCOMMON_H
#define EASYEVENT_EVENT_EVTCOMMON_H

#include "EasyEvent/Common/Config.h"

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
#   include <winerror.h>
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "ws2_32.lib")
#else
#   include <sys/ioctl.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <poll.h>
#   include <fcntl.h>
#   include <sys/socket.h>
#   include <sys/uio.h>
#   include <sys/un.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <net/if.h>
#endif

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
#   define EASY_EVENT_USE_SELECT
#elif EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_LINUX
#   define EASY_EVENT_USE_EPOLL
#   include <sys/epoll.h>
#else
#   define EASY_EVENT_E_POLL
#endif


#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
#   define NATIVE_ERROR(e) e
#   define SOCKET_ERROR(e) WSA ## e
#   define NETDB_ERROR(e) WSA ## e
#   define GETADDRINFO_ERROR(e) WSA ## e
#   define WIN_OR_POSIX(e_win, e_posix) e_win

typedef SOCKET SocketType;
const SOCKET InvalidSocket = INVALID_SOCKET;
const int SocketErrorRetVal = SOCKET_ERROR;
#else
#   define NATIVE_ERROR(e) e
#   define SOCKET_ERROR(e) e
#   define NETDB_ERROR(e) e
#   define GETADDRINFO_ERROR(e) e
#   define WIN_OR_POSIX(e_win, e_posix) e_posix

typedef int SocketType;
const int InvalidSocket = -1;
const int SocketErrorRetVal = -1;
#endif


namespace EasyEvent {

    enum class SocketErrors {
        /// Permission denied.
        AccessDenied = SOCKET_ERROR(EACCES),
        /// Address family not supported by protocol.
        AddressFamilyNotSupported = SOCKET_ERROR(EAFNOSUPPORT),
        /// Address already in use.
        AddressInUse = SOCKET_ERROR(EADDRINUSE),
        /// Transport endpoint is already connected.
        AlreadyConnected = SOCKET_ERROR(EISCONN),
        /// Operation already in progress.
        AlreadyStarted = SOCKET_ERROR(EALREADY),
        /// Broken pipe.
        BrokenPipe = WIN_OR_POSIX(NATIVE_ERROR(ERROR_BROKEN_PIPE),NATIVE_ERROR(EPIPE)),
        /// A connection has been aborted.
        ConnectionAborted = SOCKET_ERROR(ECONNABORTED),
        /// Connection refused.
        ConnectionRefused = SOCKET_ERROR(ECONNREFUSED),
        /// Connection reset by peer.
        ConnectionReset = SOCKET_ERROR(ECONNRESET),
        /// Bad file descriptor.
        BadDescriptor = SOCKET_ERROR(EBADF),
        /// Bad address.
        Fault = SOCKET_ERROR(EFAULT),
        /// No route to host.
        HostUnreachable = SOCKET_ERROR(EHOSTUNREACH),
        /// Operation now in progress.
        InProgress = SOCKET_ERROR(EINPROGRESS),
        /// Interrupted system call.
        Interrupted = SOCKET_ERROR(EINTR),
        /// Invalid argument.
        InvalidArgument = SOCKET_ERROR(EINVAL),
        /// Message too long.
        MessageSize = SOCKET_ERROR(EMSGSIZE),
        /// The name was too long.
        NameTooLong = SOCKET_ERROR(ENAMETOOLONG),
        /// Network is down.
        NetworkDown = SOCKET_ERROR(ENETDOWN),
        /// Network dropped connection on reset.
        NetworkReset = SOCKET_ERROR(ENETRESET),
        /// Network is unreachable.
        NetworkUnreachable = SOCKET_ERROR(ENETUNREACH),
        /// Too many open files.
        NoDescriptors = SOCKET_ERROR(EMFILE),
        /// No buffer space available.
        NoBufferSpace = SOCKET_ERROR(ENOBUFS),
        /// Cannot allocate memory.
        NoMemory = WIN_OR_POSIX(NATIVE_ERROR(ERROR_OUTOFMEMORY),NATIVE_ERROR(ENOMEM)),
        /// Operation not permitted.
        NoPermission = WIN_OR_POSIX(NATIVE_ERROR(ERROR_ACCESS_DENIED),NATIVE_ERROR(EPERM)),
        /// Protocol not available.
        NoProtocolOption = SOCKET_ERROR(ENOPROTOOPT),
        /// No such device.
        NoSuchDevice = WIN_OR_POSIX(NATIVE_ERROR(ERROR_BAD_UNIT),NATIVE_ERROR(ENODEV)),
        /// Transport endpoint is not connected.
        NotConnected = SOCKET_ERROR(ENOTCONN),
        /// Socket operation on non-socket.
        NotSocket = SOCKET_ERROR(ENOTSOCK),
        /// Operation cancelled.
        OperationAborted = WIN_OR_POSIX(NATIVE_ERROR(ERROR_OPERATION_ABORTED),NATIVE_ERROR(ECANCELED)),
        /// Operation not supported.
        OperationNotSupported = SOCKET_ERROR(EOPNOTSUPP),
        /// Cannot send after transport endpoint shutdown.
        ShutDown = SOCKET_ERROR(ESHUTDOWN),
        /// Connection timed out.
        TimedOut = SOCKET_ERROR(ETIMEDOUT),
        /// Resource temporarily unavailable.
        TryAgain = WIN_OR_POSIX(NATIVE_ERROR(ERROR_RETRY),NATIVE_ERROR(EAGAIN)),
        /// The socket is marked non-blocking and the requested operation would block.
        WouldBlock = SOCKET_ERROR(EWOULDBLOCK)
    };

    class EASY_EVENT_API SocketErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getSocketErrorCategory();


    inline std::error_code make_error_code(SocketErrors err) {
        return {static_cast<int>(err), getSocketErrorCategory()};
    }

#if defined(EASY_EVENT_USE_SELECT)
    enum IOEvents: uint16 {
        IO_EVENT_NONE = 0,
        IO_EVENT_READ = 0x01,
        IO_EVENT_WRITE = 0x02,
        IO_EVENT_ERROR = 0x04,
    };
#elif defined(EASY_EVENT_USE_EPOLL)
    enum class IOEvents: uint16 {
        IO_EVENT_NONE = 0,
        IO_EVENT_READ = EPOLLIN,
        IO_EVENT_WRITE = EPOLLOUT,
        IO_EVENT_ERROR = EPOLLERR | EPOLLHUP,
    };
#else
    enum class IOEvents: uint16 {
        IO_EVENT_NONE = 0,
        IO_EVENT_READ = POLLIN,
        IO_EVENT_WRITE = POLLOUT,
        IO_EVENT_ERROR = POLLERR | POLLHUP,
    };
#endif

    class EASY_EVENT_API Selectable {
    public:
        virtual void handleEvents(IOEvents events) = 0;
        virtual SocketType getSocket() const = 0;
        virtual void closeSocket() = 0;
        virtual ~Selectable() = default;
    };

    using SelectablePtr = std::shared_ptr<Selectable>;

    class EASY_EVENT_API SelectableData {
    public:
        virtual ~SelectableData() = default;
    };

}

namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::SocketErrors>: public true_type {};

}

#endif //EASYEVENT_EVENT_EVTCOMMON_H
