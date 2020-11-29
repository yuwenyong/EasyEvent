//
// Created by yuwenyong on 2020/11/28.
//

#ifndef EASYEVENT_EVENT_ADDRESS_H
#define EASYEVENT_EVENT_ADDRESS_H

#include "EasyEvent/Event/EvtCommon.h"


namespace EasyEvent {

    enum ProtocolSupport {
        EnableIPv4,
        EnableIPv6,
        EnableBoth
    };

    using IPv4Bytes = std::array<unsigned char, 4>;
    using IPv6Bytes = std::array<unsigned char, 16>;

    class EASY_EVENT_API Address {
    public:
        Address() = default;

        Address(unsigned short port, ProtocolSupport protocol)
            : Address() {
            if (protocol == EnableIPv6) {
                _addr.saIn6.sin6_family = AF_INET6;
                _addr.saIn6.sin6_port = htons(port);
            } else {
                _addr.saIn.sin_family = AF_INET;
                _addr.saIn.sin_port = htons(port);
            }
        }

        Address(const IPv4Bytes& bytes, unsigned short port)
            : Address() {
            _addr.saIn.sin_family = AF_INET;
            _addr.saIn.sin_port = htons(port);
            std::memcpy(&_addr.saIn.sin_addr.s_addr, bytes.data(), 4);
        }

        Address(uint32_t addr, unsigned short port)
            : Address() {
            _addr.saIn.sin_family = AF_INET;
            _addr.saIn.sin_port = htons(port);
            _addr.saIn.sin_addr.s_addr = htonl(addr);
        }

        Address(const IPv6Bytes& bytes, unsigned long scope, unsigned short port)
            : Address() {
            _addr.saIn6.sin6_family = AF_INET6;
            _addr.saIn6.sin6_port = htons(port);
            _addr.saIn6.sin6_scope_id = (uint32_t)scope;
            std::memcpy(_addr.saIn6.sin6_addr.s6_addr, bytes.data(), 16);
        }

        Address(const char* addr, unsigned short port, ProtocolSupport protocol=EnableBoth);

        bool operator<(const Address& rhs) const;

        bool operator==(const Address& rhs) const;

        bool operator>(const Address& rhs) const {
            return rhs < *this;
        }

        bool operator<=(const Address& rhs) const {
            return !(rhs > *this);
        }

        bool operator>=(const Address& rhs) const {
            return !(*this < rhs);
        }

        bool operator!=(const Address& rhs) const {
            return !(*this == rhs);
        }

        bool isUnspecified() const {
            return _addr.saStorage.ss_family == AF_UNSPEC;
        }

        bool isValid() const {
            return _addr.saStorage.ss_family != AF_UNSPEC;
        }

        bool isIPv4() const {
            return _addr.saStorage.ss_family == AF_INET;
        }

        bool isIPv6() const {
            return _addr.saStorage.ss_family == AF_INET6;
        }

        size_t getStorageSize() const {
            size_t size = 0;
            if (_addr.saStorage.ss_family == AF_INET) {
                size = sizeof(sockaddr_in);
            } else if (_addr.saStorage.ss_family == AF_INET6) {
                size = sizeof(sockaddr_in6);
            }
            return size;
        }

        unsigned short getPort(std::error_code& ec) const {
            if (_addr.saStorage.ss_family != AF_INET && _addr.saStorage.ss_family != AF_INET6) {
                ec = std::make_error_code(std::errc::not_supported);
                return 0;
            }
            ec.assign(0, ec.category());
            if (_addr.saStorage.ss_family == AF_INET) {
                return ntohs(_addr.saIn.sin_port);
            } else {
                return ntohs(_addr.saIn6.sin6_port);
            }
        }

        unsigned short getPort() const {
            std::error_code ec;
            auto ret = getPort(ec);
            throwError(ec);
            return ret;
        }

        void setPort(unsigned short port, std::error_code& ec) {
            if (_addr.saStorage.ss_family != AF_INET && _addr.saStorage.ss_family != AF_INET6) {
                ec = std::make_error_code(std::errc::not_supported);
                return;
            }
            ec = {0, ec.category()};
            if (_addr.saStorage.ss_family == AF_INET) {
                _addr.saIn.sin_port = htons(port);
            } else {
                _addr.saIn6.sin6_port = htons(port);
            }
        }

        void setPort(unsigned short port) {
            std::error_code ec;
            setPort(port, ec);
            throwError(ec);
        }

        IPv4Bytes getIPv4Bytes(std::error_code& ec) const {
            IPv4Bytes bytes;
            if (_addr.saStorage.ss_family != AF_INET) {
                ec = std::make_error_code(std::errc::not_supported);
                return bytes;
            }
            ec.assign(0, ec.category());
            std::memcpy(bytes.data(), &_addr.saIn.sin_addr.s_addr, 4);
            return bytes;
        }

        IPv4Bytes getIPv4Bytes() const {
            std::error_code ec;
            auto  ret = getIPv4Bytes(ec);
            throwError(ec);
            return ret;
        }

        uint32_t getIPv4UInt(std::error_code& ec) const {
            if (_addr.saStorage.ss_family != AF_INET) {
                ec = std::make_error_code(std::errc::not_supported);
                return 0;
            }
            ec.assign(0, ec.category());
            return ntohl(_addr.saIn.sin_addr.s_addr);
        }

        uint32 getIPv4UInt() const {
            std::error_code ec;
            auto  ret = getIPv4UInt(ec);
            throwError(ec);
            return ret;
        }

        IPv6Bytes getIPv6Bytes(std::error_code& ec) const {
            IPv6Bytes bytes;
            if (_addr.saStorage.ss_family != AF_INET6) {
                ec = std::make_error_code(std::errc::not_supported);
                return bytes;
            }
            ec.assign(0, ec.category());
            std::memcpy(bytes.data(), _addr.saIn6.sin6_addr.s6_addr, 16);
            return bytes;
        }

        IPv6Bytes getIPv6Bytes() const {
            std::error_code ec;
            auto  ret = getIPv6Bytes(ec);
            throwError(ec);
            return ret;
        }

        unsigned long getScopeId(std::error_code& ec) const {
            if (_addr.saStorage.ss_family != AF_INET6) {
                ec = std::make_error_code(std::errc::not_supported);
                return 0;
            }
            ec.assign(0, ec.category());
            return _addr.saIn6.sin6_scope_id;
        }

        unsigned long getScopeId() const {
            std::error_code ec;
            auto  ret = getScopeId(ec);
            throwError(ec);
            return ret;
        }

        void setScopeId(unsigned long scopeId, std::error_code& ec) {
            if (_addr.saStorage.ss_family != AF_INET6) {
                ec = std::make_error_code(std::errc::not_supported);
                return;
            }
            ec.assign(0, ec.category());
            _addr.saIn6.sin6_scope_id = (uint32_t)scopeId;
        }

        void setScopeId(unsigned long scopeId) {
            std::error_code ec;
            setScopeId(scopeId, ec);
            throwError(ec);
        }

        std::string getAddrString(std::error_code& ec) const;

        std::string getAddrString() const {
            std::error_code ec;
            auto ret = getAddrString(ec);
            throwError(ec);
            return ret;
        }

        std::string getString(std::error_code& ec) const {
            std::string addr = getAddrString(ec);
            if (ec) {
                return {};
            }
            unsigned short port = getPort(ec);
            if (ec) {
                return {};
            }
            std::ostringstream s;
            s << addr << ':' << port;
            return s.str();
        }

        std::string getString() const {
            std::error_code ec;
            auto ret = getString(ec);
            throwError(ec);
            return ret;
        }

        static std::vector<Address> getLoopbackAddresses(ProtocolSupport protocol, unsigned short port=0);

        static std::vector<Address> getWildAddresses(ProtocolSupport protocol, unsigned short port=0);

        static bool isIPv4Address(const char* addr);

        static bool isIPv6Address(const char* addr);

        static bool isIPAddress(const char* addr) {
            return isIPv4Address(addr) || isIPv6Address(addr);
        }

        static Address anyIPv4Address(unsigned short port) {
            return Address(port, EnableIPv4);
        }

        static Address anyIPv6Address(unsigned short port) {
            return Address(port, EnableIPv6);
        }

        static Address makeIPv4Address(const char* str, unsigned short port, std::error_code& ec);

        static Address makeIPv4Address(const char* str, unsigned short port) {
            std::error_code ec;
            auto ret = makeIPv4Address(str, port, ec);
            throwError(ec);
            return ret;
        }

        static Address makeIPv6Address(const char* str, unsigned short port, std::error_code& ec);

        static Address makeIPv6Address(const char* str, unsigned short port) {
            std::error_code ec;
            auto ret = makeIPv6Address(str, port, ec);
            throwError(ec);
            return ret;
        }

        static Address makeAddress(const char* str, unsigned short port, std::error_code& ec);

        static Address makeAddress(const char* str, unsigned short port) {
            std::error_code ec;
            auto ret = makeAddress(str, port, ec);
            throwError(ec);
            return ret;
        }
    private:
        void initIPv4(const char* addr, unsigned short port);

        void initIPv6(const char* addr, unsigned short port);

        union SockAddr {

            SockAddr() {
                std::memset(&saStorage, 0, sizeof(sockaddr_storage));
                saStorage.ss_family = AF_UNSPEC;
            }

            sockaddr sa;
            sockaddr_in saIn;
            sockaddr_in6 saIn6;
            sockaddr_storage saStorage;
        };

        SockAddr _addr;
    };

    template <typename Elem, typename Traits>
    std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const Address& addr) {
        return os << addr.getString().c_str();
    }

}

#endif //EASYEVENT_EVENT_ADDRESS_H
