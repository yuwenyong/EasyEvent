//
// Created by yuwenyong.vincent on 2021/6/19.
//

#include "EasyEvent/Ssl/SslSocket.h"
#include "EasyEvent/Event/SocketOps.h"


EasyEvent::SslSocket::SslSocket(SslContextPtr context, SocketType socket, bool owned, SslServerOrClient socketType)
    : _context(std::move(context))
    , _socket(socket)
    , _owned(owned) {

    ERR_clear_error();

    _ssl = ::SSL_new(context->nativeHandle());
    if (!_ssl) {
        std::error_code ec(static_cast<int>(::ERR_get_error()), getSslErrorCategory());
        throwError(ec, "SslSocket");
    }

    SSL_set_fd(_ssl, static_cast<int>(socket));

    ::SSL_set_mode(_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
    ::SSL_set_mode(_ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
#if defined(SSL_MODE_RELEASE_BUFFERS)
    ::SSL_set_mode(_ssl, SSL_MODE_RELEASE_BUFFERS);
#endif // defined(SSL_MODE_RELEASE_BUFFERS)

    BIO_set_nbio(SSL_get_rbio(_ssl), 1);
    BIO_set_nbio(SSL_get_wbio(_ssl), 1);

    if (socketType == SslServerOrClient::Client) {
        SSL_set_connect_state(_ssl);
    } else {
        SSL_set_accept_state(_ssl);
    }
}

EasyEvent::SslSocket::~SslSocket() noexcept {
    if (_ssl && SSL_get_app_data(_ssl)) {
        delete static_cast<SslVerifyCallbackBase*>(SSL_get_app_data(_ssl));
        SSL_set_app_data(_ssl, nullptr);
    }
    if (_ssl) {
        ::SSL_free(_ssl);
    }
    if (_socket != InvalidSocket && _owned) {
        std::error_code ignoredError;
        SocketOps::Close(_socket, true, ignoredError);
    }
}

void EasyEvent::SslSocket::setVerifyMode(SslVerifyMode verifyMode) {
    std::error_code ec;
    setVerifyMode(verifyMode, ec);
    throwError(ec, "setVerifyMode");
}

void EasyEvent::SslSocket::setVerifyMode(SslVerifyMode verifyMode, std::error_code &ec) {
    ::SSL_set_verify(_ssl, static_cast<int>(verifyMode), ::SSL_get_verify_callback(_ssl));
    ec = std::error_code();
}

void EasyEvent::SslSocket::doSetVerifyCallback(SslVerifyCallbackBase *callback, std::error_code &ec) {
    if (SSL_get_app_data(_ssl)) {
        delete static_cast<SslVerifyCallbackBase*>(SSL_get_app_data(_ssl));
    }

    SSL_set_app_data(_ssl, callback);

    ::SSL_set_verify(_ssl, ::SSL_get_verify_mode(_ssl), &SslSocket::verifyCallbackFunction);
    ec = std::error_code();
}

void EasyEvent::SslSocket::doHandshake() {
    std::error_code ec;
    doHandshake(ec);
    throwError(ec, "doHandshake");
}

void EasyEvent::SslSocket::doHandshake(std::error_code &ec) {
    ::ERR_clear_error();

    int result = ::SSL_do_handshake(_ssl);
    setErrorCode(result, ec);
}

ssize_t EasyEvent::SslSocket::write(const void *data, size_t size) {
    std::error_code ec;
    ssize_t bytes = write(data, size, ec);
    throwError(ec, "write");
    return bytes;
}

ssize_t EasyEvent::SslSocket::write(const void *data, size_t size, std::error_code &ec) {
    if (size == 0) {
        ec = std::error_code();
        return 0;
    }
    ::ERR_clear_error();
    int result = ::SSL_write(_ssl, data, size < INT_MAX ? static_cast<int>(size) : INT_MAX);

    setErrorCode(result, ec);
    if (!ec) {
        return result;
    } else if (ec == SslErrors::WantWrite || ec == SslErrors::WantRead) {
        return 0;
    } else {
        return -1;
    }
}

ssize_t EasyEvent::SslSocket::read(void *data, size_t size) {
    std::error_code ec;
    ssize_t bytes = read(data, size, ec);
    throwError(ec, "read");
    return bytes;
}

ssize_t EasyEvent::SslSocket::read(void *data, size_t size, std::error_code &ec) {
    if (size == 0) {
        ec = std::error_code();
        return 0;
    }
    ::ERR_clear_error();
    int result = ::SSL_read(_ssl, data, size < INT_MAX ? static_cast<int>(size) : INT_MAX);
    setErrorCode(result, ec);
    if (!ec) {
        return result;
    } else if (ec == SslErrors::WantWrite || ec == SslErrors::WantRead) {
        return 0;
    } else if (ec == SslErrors::ErrorZeroReturn) {
        if ((::SSL_get_shutdown(_ssl) & SSL_RECEIVED_SHUTDOWN) == 0) {
            ec = SslStreamErrors::StreamTruncated;
        }
        return -1;
    } else {
        return -1;
    }
}

void EasyEvent::SslSocket::close() {
    std::error_code ec;
    close(ec);
    throwError(ec, "close");
}

void EasyEvent::SslSocket::close(std::error_code &ec) {
    if (_ssl && SSL_get_app_data(_ssl)) {
        delete static_cast<SslVerifyCallbackBase*>(SSL_get_app_data(_ssl));
        SSL_set_app_data(_ssl, nullptr);
    }
    if (_ssl) {
        ::SSL_free(_ssl);
        _ssl = nullptr;
    }
    if (_socket != InvalidSocket && _owned) {
        SocketOps::Close(_socket, false, ec);
        if (!ec) {
            _socket = InvalidSocket;
        } else {
            return;
        }
    }
    ec = std::error_code();
}

int EasyEvent::SslSocket::verifyCallbackFunction(int preverified, X509_STORE_CTX *ctx) {
    if (ctx) {
        if (SSL* ssl = static_cast<SSL*>(::X509_STORE_CTX_get_ex_data(ctx, ::SSL_get_ex_data_X509_STORE_CTX_idx()))) {
            if (SSL_get_app_data(ssl)) {
                auto* callback = static_cast<SslVerifyCallbackBase*>(SSL_get_app_data(ssl));

                SslVerifyContext verifyCtx(ctx);
                return callback->call(preverified != 0, verifyCtx) ? 1 : 0;
            }
        }
    }
    return 0;
}

void EasyEvent::SslSocket::setErrorCode(int result, std::error_code &ec) {
    int sslError = ::SSL_get_error(_ssl, result);
    int sysError = static_cast<int>(::ERR_get_error());

    if (sslError == SSL_ERROR_NONE) {
        ec = std::error_code();
    } else if (sslError == SSL_ERROR_SSL) {
        ec = std::error_code(sysError, getSslErrorCategory());
    } else if (sslError == SSL_ERROR_SYSCALL) {
        if (sysError == 0) {
            if (result == 0) {
                ec = SslStreamErrors::ConnectionReset;
            } else if (result == -1) {
                ec = SslStreamErrors::UnspecifiedSystemError;
            } else {
                ec = SslStreamErrors::UnexpectedResult;
            }

        } else {
            ec = std::error_code(sysError, getSslErrorCategory());
        }
    } else if (sslError == SSL_ERROR_WANT_WRITE) {
        ec = SslErrors::WantWrite;
    } else if (sslError == SSL_ERROR_WANT_READ) {
        ec = SslErrors::WantRead;
    } else if (sslError == SSL_ERROR_ZERO_RETURN) {
        ec = SslErrors::ErrorZeroReturn;
    } else {
        ec = SslStreamErrors::UnexpectedResult;
    }
}