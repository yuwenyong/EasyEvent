//
// Created by yuwenyong.vincent on 2021/6/6.
//

#include "EasyEvent/Ssl/SslBase.h"
#include "EasyEvent/Ssl/SslInit.h"
#include "EasyEvent/Event/Address.h"


const char * EasyEvent::SslErrorCategory::name() const noexcept {
    return "ssl error";
}

std::string EasyEvent::SslErrorCategory::message(int ev) const {
    const char* s = ::ERR_reason_error_string(ev);
    return s ? s : "ssl error";
}

const std::error_category& EasyEvent::getSslErrorCategory() {
    static const SslErrorCategory errCategory{};
    return errCategory;
}

const char * EasyEvent::SslStreamErrorCategory::name() const noexcept {
    return "ssl stream error";
}

std::string EasyEvent::SslStreamErrorCategory::message(int ev) const {
    switch (static_cast<SslStreamErrors>(ev)) {
        case SslStreamErrors::StreamTruncated:
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) && !defined(OPENSSL_IS_BORINGSSL)
            return ::ERR_reason_error_string(ev);
#else
            return "stream truncated";
#endif
        case SslStreamErrors::UnspecifiedSystemError:
            return "unspecified system error";
        case SslStreamErrors::UnexpectedResult:
            return "unexpected result";
        case SslStreamErrors::ConnectionReset:
            return "connection reset";
        default:
            return "ssl stream error";
    }
}

const std::error_category& EasyEvent::getSslStreamErrorCategory() {
    static const SslStreamErrorCategory errCategory{};
    return errCategory;
}


bool EasyEvent::HostNameVerification::operator()(bool preverified, SslVerifyContext &ctx) const {
    if (!preverified) {
        return false;
    }

    int depth = X509_STORE_CTX_get_error_depth(ctx.nativeHandle());
    if (depth > 0) {
        return true;
    }

    const bool isAddress = Address::isAddress(_host.c_str());

    X509* cert = X509_STORE_CTX_get_current_cert(ctx.nativeHandle());

    if (isAddress) {
        return X509_check_ip_asc(cert, _host.c_str(), 0) == 1;
    } else {
        char* peername = nullptr;
        const int result = X509_check_host(cert,_host.c_str(), _host.size(), 0, &peername);
        OPENSSL_free(peername);
        return result == 1;
    }
}