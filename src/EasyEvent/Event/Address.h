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

    class EASY_EVENT_API Address {
    public:
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

        bool isEmpty() const {
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

        unsigned short getPort() const {
            if (_addr.saStorage.ss_family == AF_INET) {
                return ntohs(_addr.saIn.sin_port);
            } else if (_addr.saStorage.ss_family == AF_INET6) {
                return ntohs(_addr.saIn6.sin6_port);
            } else {
                return 0;
            }
        }

        void setPort(unsigned short port) {
            if (_addr.saStorage.ss_family == AF_INET) {
                _addr.saIn.sin_port = htons(port);
            } else if (_addr.saStorage.ss_family == AF_INET6) {
                _addr.saIn6.sin6_port = htons(port);
            }
        }

        static std::vector<Address> getLoopbackAddresses(ProtocolSupport protocol, unsigned short port=0);

        static std::vector<Address> getWildAddresses(ProtocolSupport protocol, unsigned short port=0);
    private:
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


}

#endif //EASYEVENT_EVENT_ADDRESS_H
