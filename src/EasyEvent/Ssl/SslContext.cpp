//
// Created by yuwenyong.vincent on 2021/6/13.
//

#include "EasyEvent/Ssl/SslContext.h"

EasyEvent::SslContext::SslContext(SslProtoVersion protoVersion)
        : _handle(nullptr) {

    ::ERR_clear_error();

    switch (protoVersion) {
        // SSL v2.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) || defined(OPENSSL_NO_SSL2)
        case SslProtoVersion::SslV2:
        case SslProtoVersion::SslV2Client:
        case SslProtoVersion::SslV2Server:
            throwError(UserErrors::InvalidArgument, "SslContext");
            break;
#else // (OPENSSL_VERSION_NUMBER >= 0x10100000L) || defined(OPENSSL_NO_SSL2)
            case SslProtoVersion::SslV2:
            _handle = ::SSL_CTX_new(::SSLv2_method());
            break;
        case SslProtoVersion::SslV2Client:
            _handle = ::SSL_CTX_new(::SSLv2_client_method());
            break;
        case SslProtoVersion::SslV2Server:
            _handle = ::SSL_CTX_new(::SSLv2_server_method());
            break;
#endif // (OPENSSL_VERSION_NUMBER >= 0x10100000L) || defined(OPENSSL_NO_SSL2)

            // SSL v3.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER)
            case SslProtoVersion::SslV3:
            _handle = ::SSL_CTX_new(::TLS_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, SSL3_VERSION);
                SSL_CTX_set_max_proto_version(_handle, SSL3_VERSION);
            }
            break;
        case SslProtoVersion::SslV3Client:
            _handle = ::SSL_CTX_new(::TLS_client_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, SSL3_VERSION);
                SSL_CTX_set_max_proto_version(_handle, SSL3_VERSION);
            }
            break;
        case SslProtoVersion::SslV3Server:
            _handle = ::SSL_CTX_new(::TLS_server_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, SSL3_VERSION);
                SSL_CTX_set_max_proto_version(_handle, SSL3_VERSION);
            }
            break;
#elif defined(OPENSSL_NO_SSL3)
            case SslProtoVersion::SslV3:
        case SslProtoVersion::SslV3Client:
        case SslProtoVersion::SslV3Server:
            throwError(UserErrors::InvalidArgument, "SslContext");
            break;
#else // defined(OPENSSL_NO_SSL3)
        case SslProtoVersion::SslV3:
            _handle = ::SSL_CTX_new(::SSLv3_method());
            break;
        case SslProtoVersion::SslV3Client:
            _handle = ::SSL_CTX_new(::SSLv3_client_method());
            break;
        case SslProtoVersion::SslV3Server:
            _handle = ::SSL_CTX_new(::SSLv3_server_method());
            break;
#endif // defined(OPENSSL_NO_SSL3)

            // TLS v1.0.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER)
            case SslProtoVersion::TlsV1:
            _handle = ::SSL_CTX_new(::TLS_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_VERSION);
            }
            break;
        case SslProtoVersion::TlsV1Client:
            _handle = ::SSL_CTX_new(::TLS_client_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_VERSION);
            }
            break;
        case SslProtoVersion::TlsV1Server:
            _handle = ::SSL_CTX_new(::TLS_server_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_VERSION);
            }
            break;
#elif defined(SSL_TXT_TLSV1)
        case SslProtoVersion::TlsV1:
            _handle = ::SSL_CTX_new(::TLSv1_method());
            break;
        case SslProtoVersion::TlsV1Client:
            _handle = ::SSL_CTX_new(::TLSv1_client_method());
            break;
        case SslProtoVersion::TlsV1Server:
            _handle = ::SSL_CTX_new(::TLSv1_server_method());
            break;
#else // defined(SSL_TXT_TLSV1)
            case SslProtoVersion::TlsV1:
        case SslProtoVersion::TlsV1Client:
        case SslProtoVersion::TlsV1Server:
            throwError(UserErrors::InvalidArgument, "SslContext");
            break;
#endif // defined(SSL_TXT_TLSV1)

            // TLS v1.1.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER)
            case SslProtoVersion::TlsV11:
            _handle = ::SSL_CTX_new(::TLS_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_1_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_1_VERSION);
            }
            break;
        case SslProtoVersion::TlsV11Client:
            _handle = ::SSL_CTX_new(::TLS_client_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_1_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_1_VERSION);
            }
            break;
        case SslProtoVersion::TlsV11Server:
            _handle = ::SSL_CTX_new(::TLS_server_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_1_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_1_VERSION);
            }
            break;
#elif defined(SSL_TXT_TLSV1_1)
        case SslProtoVersion::TlsV11:
            _handle = ::SSL_CTX_new(::TLSv1_1_method());
            break;
        case SslProtoVersion::TlsV11Client:
            _handle = ::SSL_CTX_new(::TLSv1_1_client_method());
            break;
        case SslProtoVersion::TlsV11Server:
            _handle = ::SSL_CTX_new(::TLSv1_1_server_method());
            break;
#else // defined(SSL_TXT_TLSV1_1)
            case SslProtoVersion::TlsV11:
        case SslProtoVersion::TlsV11Client:
        case SslProtoVersion::TlsV11Server:
            throwError(UserErrors::InvalidArgument, "SslContext");
            break;
#endif // defined(SSL_TXT_TLSV1_1)

            // TLS v1.2.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER)
            case SslProtoVersion::TlsV12:
            _handle = ::SSL_CTX_new(::TLS_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_2_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_2_VERSION);
            }
            break;
        case SslProtoVersion::TlsV12Client:
            _handle = ::SSL_CTX_new(::TLS_client_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_2_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_2_VERSION);
            }
            break;
        case SslProtoVersion::TlsV12Server:
            _handle = ::SSL_CTX_new(::TLS_server_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_2_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_2_VERSION);
            }
            break;
#elif defined(SSL_TXT_TLSV1_2)
        case SslProtoVersion::TlsV12:
            _handle = ::SSL_CTX_new(::TLSv1_2_method());
            break;
        case SslProtoVersion::TlsV12Client:
            _handle = ::SSL_CTX_new(::TLSv1_2_client_method());
            break;
        case SslProtoVersion::TlsV12Server:
            _handle = ::SSL_CTX_new(::TLSv1_2_server_method());
            break;
#else // defined(SSL_TXT_TLSV1_2)
            case SslProtoVersion::TlsV12:
        case SslProtoVersion::TlsV12Client:
        case SslProtoVersion::TlsV12Server:
            throwError(UserErrors::InvalidArgument, "SslContext");
            break;
#endif // defined(SSL_TXT_TLSV1_2)

            // TLS v1.3.
#if (OPENSSL_VERSION_NUMBER >= 0x10101000L) && !defined(LIBRESSL_VERSION_NUMBER)
            case SslProtoVersion::TlsV13:
            _handle = ::SSL_CTX_new(::TLS_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_3_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_3_VERSION);
            }
            break;
        case SslProtoVersion::TlsV13Client:
            _handle = ::SSL_CTX_new(::TLS_client_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_3_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_3_VERSION);
            }
            break;
        case SslProtoVersion::TlsV13Server:
            _handle = ::SSL_CTX_new(::TLS_server_method());
            if (_handle) {
                SSL_CTX_set_min_proto_version(_handle, TLS1_3_VERSION);
                SSL_CTX_set_max_proto_version(_handle, TLS1_3_VERSION);
            }
            break;
#else // (OPENSSL_VERSION_NUMBER >= 0x10101000L) && !defined(LIBRESSL_VERSION_NUMBER)
        case SslProtoVersion::TlsV13:
        case SslProtoVersion::TlsV13Client:
        case SslProtoVersion::TlsV13Server:
            throwError(UserErrors::InvalidArgument, "SslContext");
            break;
#endif // (OPENSSL_VERSION_NUMBER >= 0x10101000L) && !defined(LIBRESSL_VERSION_NUMBER)

            // Any supported SSL/TLS version.
        case SslProtoVersion::SslV23:
            _handle = ::SSL_CTX_new(::SSLv23_method());
            break;
        case SslProtoVersion::SslV23Client:
            _handle = ::SSL_CTX_new(::SSLv23_client_method());
            break;
        case SslProtoVersion::SslV23Server:
            _handle = ::SSL_CTX_new(::SSLv23_server_method());
            break;

            // Any supported TLS version.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER)
            case SslProtoVersion::Tls:
            _handle = ::SSL_CTX_new(::TLS_method());
            if (_handle)
                SSL_CTX_set_min_proto_version(_handle, TLS1_VERSION);
            break;
        case SslProtoVersion::TlsClient:
            _handle = ::SSL_CTX_new(::TLS_client_method());
            if (_handle)
                SSL_CTX_set_min_proto_version(_handle, TLS1_VERSION);
            break;
        case SslProtoVersion::TlsServer:
            _handle = ::SSL_CTX_new(::TLS_server_method());
            if (_handle)
                SSL_CTX_set_min_proto_version(_handle, TLS1_VERSION);
            break;
#else // (OPENSSL_VERSION_NUMBER >= 0x10100000L)
        case SslProtoVersion::Tls:
            _handle = ::SSL_CTX_new(::SSLv23_method());
            if (_handle)
                SSL_CTX_set_options(_handle, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
            break;
        case SslProtoVersion::TlsClient:
            _handle = ::SSL_CTX_new(::SSLv23_client_method());
            if (_handle)
                SSL_CTX_set_options(_handle, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
            break;
        case SslProtoVersion::TlsServer:
            _handle = ::SSL_CTX_new(::SSLv23_server_method());
            if (_handle)
                SSL_CTX_set_options(_handle, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
            break;
#endif // (OPENSSL_VERSION_NUMBER >= 0x10100000L)

        default:
            _handle = ::SSL_CTX_new(0);
            break;
    }

    if (_handle == nullptr) {
        std::error_code ec(static_cast<int>(::ERR_get_error()), getSslErrorCategory());
        throwError(ec, "SslContext");
    }

    setOptions(SslOptions::NoCompression);
}

EasyEvent::SslContext::~SslContext() noexcept {
    if (_handle) {
#if ((OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER))
        void* cbUserdata = ::SSL_CTX_get_default_passwd_cb_userdata(_handle);
#else // (OPENSSL_VERSION_NUMBER >= 0x10100000L)
        void* cbUserdata = _handle->default_passwd_callback_userdata;
#endif // (OPENSSL_VERSION_NUMBER >= 0x10100000L)
        if (cbUserdata) {
            auto* callback = static_cast<SslPasswordCallbackBase*>(cbUserdata);
            delete callback;
#if ((OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER))
            ::SSL_CTX_set_default_passwd_cb_userdata(_handle, nullptr);
#else // (OPENSSL_VERSION_NUMBER >= 0x10100000L)
            _handle->default_passwd_callback_userdata = nullptr;
#endif // (OPENSSL_VERSION_NUMBER >= 0x10100000L)
        }

        if (SSL_CTX_get_app_data(_handle)) {
            auto* callback = static_cast<SslVerifyCallbackBase*>(SSL_CTX_get_app_data(_handle));
            delete callback;
            SSL_CTX_set_app_data(_handle, nullptr);
        }
        ::SSL_CTX_free(_handle);
    }
}

void EasyEvent::SslContext::clearOptions(SslOptions options) {
    std::error_code ec;
    clearOptions(options, ec);
    throwError(ec, "clearOptions");
}

void EasyEvent::SslContext::clearOptions(SslOptions options, std::error_code &ec) {
#if (OPENSSL_VERSION_NUMBER >= 0x009080DFL) && (OPENSSL_VERSION_NUMBER != 0x00909000L)
#   if !defined(SSL_OP_NO_COMPRESSION)
    if (static_cast<long>(options & SslOptions::NoCompression) != 0L) {
#       if (OPENSSL_VERSION_NUMBER >= 0x00908000L)
        _handle->comp_methods = SSL_COMP_get_compression_methods();
#       endif // (OPENSSL_VERSION_NUMBER >= 0x00908000L)
        options ^= SslOptions::NoCompression;
    }
#   endif // !defined(SSL_OP_NO_COMPRESSION)

    ::SSL_CTX_clear_options(_handle, static_cast<long>(options));

    ec = std::error_code();
#else // (OPENSSL_VERSION_NUMBER >= 0x009080DFL) && (OPENSSL_VERSION_NUMBER != 0x00909000L)
  (void)options;
  ec = UserErrors::NotSupported;
#endif
}

void EasyEvent::SslContext::setOptions(SslOptions options) {
    std::error_code ec;
    setOptions(options, ec);
    throwError(ec, "setOptions");
}

void EasyEvent::SslContext::setOptions(SslOptions options, std::error_code &ec) {
#if !defined(SSL_OP_NO_COMPRESSION)
    if (static_cast<long>(options & SslOptions::NoCompression) != 0L) {
#if (OPENSSL_VERSION_NUMBER >= 0x00908000L)
    _handle->comp_methods = SslInit::getNullCompressionMethods();
#endif // (OPENSSL_VERSION_NUMBER >= 0x00908000L)
    options ^= SslOptions::NoCompression;
  }
#endif // !defined(SSL_OP_NO_COMPRESSION)

    ::SSL_CTX_set_options(_handle, static_cast<long>(options));
    ec = std::error_code();
}

void EasyEvent::SslContext::setVerifyMode(SslVerifyMode verifyMode) {
    std::error_code ec;
    setVerifyMode(verifyMode, ec);
    throwError(ec, "setVerifyMode");
}

void EasyEvent::SslContext::setVerifyMode(SslVerifyMode verifyMode, std::error_code &ec) {
    ::SSL_CTX_set_verify(_handle, static_cast<int>(verifyMode), ::SSL_CTX_get_verify_callback(_handle));
    ec = std::error_code();
}

void EasyEvent::SslContext::loadVerifyFile(const std::string &filename) {
    std::error_code ec;
    loadVerifyFile(filename, ec);
    throwError(ec, "loadVerifyFile");
}

void EasyEvent::SslContext::loadVerifyFile(const std::string &filename, std::error_code &ec) {
    ::ERR_clear_error();

    if (::SSL_CTX_load_verify_locations(_handle, filename.c_str(), nullptr) != 1) {
        ec = std::error_code(static_cast<int>(::ERR_get_error()), getSslErrorCategory());
        return;
    }

    ec = std::error_code();
}

void EasyEvent::SslContext::setDefaultVerifyPaths() {
    std::error_code ec;
    setDefaultVerifyPaths(ec);
    throwError(ec, "setDefaultVerifyPaths");
}

void EasyEvent::SslContext::setDefaultVerifyPaths(std::error_code &ec) {
    ::ERR_clear_error();

    if (::SSL_CTX_set_default_verify_paths(_handle) != 1) {
        ec = std::error_code(static_cast<int>(::ERR_get_error()), getSslErrorCategory());
        return;
    }

    ec = std::error_code();
}

void EasyEvent::SslContext::addVerifyPath(const std::string &path) {
    std::error_code ec;
    addVerifyPath(path, ec);
    throwError(ec, "addVerifyPath");
}

void EasyEvent::SslContext::addVerifyPath(const std::string &path, std::error_code &ec) {
    ::ERR_clear_error();

    if (::SSL_CTX_load_verify_locations(_handle, nullptr, path.c_str()) != 1) {
        ec = std::error_code(static_cast<int>(::ERR_get_error()), getSslErrorCategory());
        return;
    }

    ec = std::error_code();
}

void EasyEvent::SslContext::useCertificateChainFile(const std::string &filename) {
    std::error_code ec;
    useCertificateChainFile(filename, ec);
    throwError(ec, "useCertificateChainFile");
}

void EasyEvent::SslContext::useCertificateChainFile(const std::string &filename, std::error_code &ec) {
    ::ERR_clear_error();

    if (::SSL_CTX_use_certificate_chain_file(_handle, filename.c_str()) != 1) {
        ec = std::error_code(static_cast<int>(::ERR_get_error()), getSslErrorCategory());
        return;
    }

    ec = std::error_code();
}

void EasyEvent::SslContext::usePrivateKeyFile(const std::string &filename) {
    std::error_code ec;
    usePrivateKeyFile(filename, ec);
    throwError(ec, "usePrivateKeyFile");
}

void EasyEvent::SslContext::usePrivateKeyFile(const std::string &filename, std::error_code &ec) {
    ::ERR_clear_error();

    if (::SSL_CTX_use_PrivateKey_file(_handle, filename.c_str(), SSL_FILETYPE_PEM) != 1) {
        ec = std::error_code(static_cast<int>(::ERR_get_error()), getSslErrorCategory());
        return;
    }

    ec = std::error_code();
}

void EasyEvent::SslContext::doSetVerifyCallback(SslVerifyCallbackBase *callback, std::error_code &ec) {
    if (SSL_CTX_get_app_data(_handle)) {
        delete static_cast<SslVerifyCallbackBase*>(SSL_CTX_get_app_data(_handle));
    }

    SSL_CTX_set_app_data(_handle, callback);

    ::SSL_CTX_set_verify(_handle, ::SSL_CTX_get_verify_mode(_handle), &SslContext::verifyCallbackFunction);
    ec = std::error_code();
}

int EasyEvent::SslContext::verifyCallbackFunction(int preverified, X509_STORE_CTX *ctx) {
    if (ctx) {
        if (SSL* ssl = static_cast<SSL*>(::X509_STORE_CTX_get_ex_data(ctx, ::SSL_get_ex_data_X509_STORE_CTX_idx()))) {
            if (SSL_CTX* handle = ::SSL_get_SSL_CTX(ssl)) {
                if (SSL_CTX_get_app_data(handle)) {
                    auto* callback = static_cast<SslVerifyCallbackBase*>(SSL_CTX_get_app_data(handle));

                    SslVerifyContext verifyCtx(ctx);
                    return callback->call(preverified != 0, verifyCtx) ? 1 : 0;
                }
            }
        }
    }
    return 0;
}

void EasyEvent::SslContext::doSetPasswordCallback(SslPasswordCallbackBase *callback, std::error_code &ec) {
#if ((OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER))
    void* oldCallback = ::SSL_CTX_get_default_passwd_cb_userdata(_handle);
    ::SSL_CTX_set_default_passwd_cb_userdata(_handle, callback);
#else // (OPENSSL_VERSION_NUMBER >= 0x10100000L)
    void* oldCallback = _handle->default_passwd_callback_userdata;
    _handle->default_passwd_callback_userdata = callback;
#endif // (OPENSSL_VERSION_NUMBER >= 0x10100000L)

    if (oldCallback) {
        delete static_cast<SslPasswordCallbackBase*>(oldCallback);
    }

    SSL_CTX_set_default_passwd_cb(_handle, &SslContext::passwordCallbackFunction);
    ec = std::error_code();
}

int EasyEvent::SslContext::passwordCallbackFunction(char *buf, int size, int purpose, void *data) {
    if (data) {
        auto* callback = static_cast<SslPasswordCallbackBase*>(data);

        std::string passwd = callback->call(static_cast<std::size_t>(size),
                                            purpose ? SslPasswordPurpose::ForWriting : SslPasswordPurpose::ForReading);

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
        strcpy_s(buf, size, passwd.c_str());
#else
        *buf = '\0';
        if (size > 0) {
            strncat(buf, passwd.c_str(), size - 1);
        }
#endif

        return static_cast<int>(std::strlen(buf));
    }

    return 0;
}