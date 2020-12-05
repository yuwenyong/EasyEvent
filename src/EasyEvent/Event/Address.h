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
        Address() {
            std::memset(&_addr.saStorage, 0, sizeof(sockaddr_storage));
            _addr.saStorage.ss_family = AF_UNSPEC;
        }

        Address(const struct sockaddr* addr, size_t addrLen)
                : Address() {
            std::memcpy(&_addr.saStorage, addr, addrLen);
        }

        Address(const struct sockaddr* addr, size_t addrLen, unsigned short port)
                : Address() {
            std::memcpy(&_addr.saStorage, addr, addrLen);
            if (_addr.saStorage.ss_family == AF_INET) {
                _addr.saIn.sin_port = htons(port);
            } else if (_addr.saStorage.ss_family == AF_INET6) {
                _addr.saIn6.sin6_port = htons(port);
            }
        }

        Address(const struct sockaddr* addr, size_t addrLen, int family, unsigned short port)
            : Address() {
            std::memcpy(&_addr.saStorage, addr, addrLen);
            if (family == AF_INET) {
                _addr.saIn.sin_port = htons(port);
            } else if (family == AF_INET6) {
                _addr.saIn6.sin6_port = htons(port);
            }
        }

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

        const struct sockaddr* getStoragePtr() const {
            return &_addr.sa;
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

        IPv4Bytes getBytesIPv4(std::error_code& ec) const {
            IPv4Bytes bytes;
            if (_addr.saStorage.ss_family != AF_INET) {
                ec = std::make_error_code(std::errc::not_supported);
                return bytes;
            }
            ec.assign(0, ec.category());
            std::memcpy(bytes.data(), &_addr.saIn.sin_addr.s_addr, 4);
            return bytes;
        }

        IPv4Bytes getBytesIPv4() const {
            std::error_code ec;
            auto  ret = getBytesIPv4(ec);
            throwError(ec);
            return ret;
        }

        uint32_t getUIntIPv4(std::error_code& ec) const {
            if (_addr.saStorage.ss_family != AF_INET) {
                ec = std::make_error_code(std::errc::not_supported);
                return 0;
            }
            ec.assign(0, ec.category());
            return ntohl(_addr.saIn.sin_addr.s_addr);
        }

        uint32 getUIntIPv4() const {
            std::error_code ec;
            auto  ret = getUIntIPv4(ec);
            throwError(ec);
            return ret;
        }

        void setUIntIPv4(uint32_t addr, std::error_code& ec) {
            if (_addr.saStorage.ss_family != AF_INET) {
                ec = std::make_error_code(std::errc::not_supported);
                return;
            }

        }

        IPv6Bytes getBytesIPv6(std::error_code& ec) const {
            IPv6Bytes bytes;
            if (_addr.saStorage.ss_family != AF_INET6) {
                ec = std::make_error_code(std::errc::not_supported);
                return bytes;
            }
            ec.assign(0, ec.category());
            std::memcpy(bytes.data(), _addr.saIn6.sin6_addr.s6_addr, 16);
            return bytes;
        }

        IPv6Bytes getBytesIPv6() const {
            std::error_code ec;
            auto  ret = getBytesIPv6(ec);
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

        std::string toString(std::error_code& ec) const {
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

        std::string toString() const {
            std::error_code ec;
            auto ret = toString(ec);
            throwError(ec);
            return ret;
        }

        static std::vector<Address> loopbackAddresses(ProtocolSupport protocol, unsigned short port=0);

        static std::vector<Address> anyAddresses(ProtocolSupport protocol, unsigned short port=0);

        static bool isAddressIPv4(const char* addr);

        static bool isAddressIPv6(const char* addr);

        static bool isAddress(const char* addr) {
            return isAddressIPv4(addr) || isAddressIPv6(addr);
        }

        static Address loopbackAddressIPv4(unsigned short port) {
            Address addr;
            addr._addr.saIn.sin_family = AF_INET;
            addr._addr.saIn.sin_port = htons(port);
            addr._addr.saIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            return addr;
        }

        static Address loopbackAddressIPv6(unsigned short port) {
            Address addr;
            addr._addr.saIn6.sin6_family = AF_INET6;
            addr._addr.saIn6.sin6_port = htons(port);
            addr._addr.saIn6.sin6_addr = in6addr_loopback;
            return addr;
        }

        static Address anyAddressIPv4(unsigned short port) {
            return Address(port, EnableIPv4);
        }

        static Address anyAddressIPv6(unsigned short port) {
            return Address(port, EnableIPv6);
        }

        static Address makeAddressIPv4(const char* str, unsigned short port, std::error_code& ec);

        static Address makeAddressIPv4(const char* str, unsigned short port) {
            std::error_code ec;
            auto ret = makeAddressIPv4(str, port, ec);
            throwError(ec);
            return ret;
        }

        static Address makeAddressIPv6(const char* str, unsigned short port, std::error_code& ec);

        static Address makeAddressIPv6(const char* str, unsigned short port) {
            std::error_code ec;
            auto ret = makeAddressIPv6(str, port, ec);
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
        void initAddressIPv4(const char* addr, unsigned short port);

        void initAddressIPv6(const char* addr, unsigned short port);

        union {
            sockaddr sa;
            sockaddr_in saIn;
            sockaddr_in6 saIn6;
            sockaddr_storage saStorage;
        } _addr;
    };

    template <typename Elem, typename Traits>
    std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const Address& addr) {
        return os << addr.toString().c_str();
    }

}

#endif //EASYEVENT_EVENT_ADDRESS_H
