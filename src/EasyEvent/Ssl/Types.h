//
// Created by yuwenyong.vincent on 2021/6/6.
//

#ifndef EASYEVENT_SSL_TYPES_H
#define EASYEVENT_SSL_TYPES_H

#include "EasyEvent/Common/Config.h"
#include "EasyEvent/Common/Error.h"
#include "EasyEvent/Common/Utility.h"

#include <openssl/conf.h>
#include <openssl/ssl.h>
#if !defined(OPENSSL_NO_ENGINE)
# include <openssl/engine.h>
#endif // !defined(OPENSSL_NO_ENGINE)
#include <openssl/dh.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>


namespace EasyEvent {

    enum class SslErrors {
        WantWrite = SSL_ERROR_WANT_WRITE,
        WantRead = SSL_ERROR_WANT_READ,
        ErrorZeroReturn = SSL_ERROR_ZERO_RETURN,
    };

    class EASY_EVENT_API SslErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getSslErrorCategory();


    inline std::error_code make_error_code(SslErrors err) {
        return {static_cast<int>(err), getSslErrorCategory()};
    }

    enum class SslStreamErrors {
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) && !defined(OPENSSL_IS_BORINGSSL)
        StreamTruncated = ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ),
#else
        StreamTruncated = 1,
#endif
        UnspecifiedSystemError = 2,
        UnexpectedResult = 3,
        ConnectionReset = 4,
    };

    class EASY_EVENT_API SslStreamErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getSslStreamErrorCategory();


    inline std::error_code make_error_code(SslStreamErrors err) {
        return {static_cast<int>(err), getSslStreamErrorCategory()};
    }

    enum class SslProtoVersion {
        /// Generic SSL version 2.
        SslV2,
        /// SSL version 2 client.
        SslV2Client,
        /// SSL version 2 server.
        SslV2Server,
        /// Generic SSL version 3.
        SslV3,
        /// SSL version 3 client.
        SslV3Client,
        /// SSL version 3 server.
        SslV3Server,
        /// Generic TLS version 1.
        TlsV1,
        /// TLS version 1 client.
        TlsV1Client,
        /// TLS version 1 server.
        TlsV1Server,
        /// Generic SSL/TLS.
        SslV23,
        /// SSL/TLS client.
        SslV23Client,
        /// SSL/TLS server.
        SslV23Server,
        /// Generic TLS version 1.1.
        TlsV11,
        /// TLS version 1.1 client.
        TlsV11Client,
        /// TLS version 1.1 server.
        TlsV11Server,
        /// Generic TLS version 1.2.
        TlsV12,
        /// TLS version 1.2 client.
        TlsV12Client,
        /// TLS version 1.2 server.
        TlsV12Server,
        /// Generic TLS version 1.3.
        TlsV13,
        /// TLS version 1.3 client.
        TlsV13Client,
        /// TLS version 1.3 server.
        TlsV13Server,
        /// Generic TLS.
        Tls,
        /// TLS client.
        TlsClient,
        /// TLS server.
        TlsServer
    };

    enum class SslVerifyMode: int {
        CertNone = SSL_VERIFY_NONE,
        CertOptional = SSL_VERIFY_PEER,
        CertRequired = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT
    };

    enum class SslOptions: long {
        DefaultWorkarounds = SSL_OP_ALL,
        SingleDhUse = SSL_OP_SINGLE_DH_USE,
        NoSslV2 = SSL_OP_NO_SSLv2,
        NoSslV3 = SSL_OP_NO_SSLv3,
        NoTlsV1 = SSL_OP_NO_TLSv1,
# if defined(SSL_OP_NO_TLSv1_1)
        NoTlsV11 = SSL_OP_NO_TLSv1_1,
# else // defined(SSL_OP_NO_TLSv1_1)
        NoTlsV11 = 0x10000000L,
# endif // defined(SSL_OP_NO_TLSv1_1)
# if defined(SSL_OP_NO_TLSv1_2)
        NoTlsV12 = SSL_OP_NO_TLSv1_2,
# else // defined(SSL_OP_NO_TLSv1_2)
        NoTlsV12 = 0x08000000L,
# endif // defined(SSL_OP_NO_TLSv1_2)
# if defined(SSL_OP_NO_TLSv1_3)
        NoTlsV13 = SSL_OP_NO_TLSv1_3,
# else // defined(SSL_OP_NO_TLSv1_3)
        NoTlsV13 = 0x20000000L,
# endif // defined(SSL_OP_NO_TLSv1_3)
# if defined(SSL_OP_NO_COMPRESSION)
        NoCompression = SSL_OP_NO_COMPRESSION,
# else // defined(SSL_OP_NO_COMPRESSION)
        NoCompression = 0x20000L,
# endif // defined(SSL_OP_NO_COMPRESSION)
    };

    inline SslOptions operator|(SslOptions lhs, SslOptions rhs) {
        return static_cast<SslOptions>(static_cast<long>(lhs) | static_cast<long>(rhs));
    }

    inline SslOptions& operator|=(SslOptions& lhs, SslOptions rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    inline SslOptions operator&(SslOptions lhs, SslOptions rhs) {
        return static_cast<SslOptions>(static_cast<long>(lhs) & static_cast<long>(rhs));
    }

    inline SslOptions operator&=(SslOptions& lhs, SslOptions rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    inline SslOptions operator^(SslOptions lhs, SslOptions rhs) {
        return static_cast<SslOptions>(static_cast<long>(lhs) ^ static_cast<long>(rhs));
    }

    inline SslOptions& operator^=(SslOptions& lhs, SslOptions rhs) {
        lhs = lhs | rhs;
        return lhs;
    }


    class EASY_EVENT_API SslVerifyContext: private NonCopyable {
    public:
        typedef X509_STORE_CTX* NativeHandleType;

        explicit SslVerifyContext(NativeHandleType handle)
            : _handle(handle) {

        }

        NativeHandleType nativeHandle() {
            return _handle;
        }

    private:
        NativeHandleType _handle;
    };

    class EASY_EVENT_API SslVerifyCallbackBase {
    public:
        virtual ~SslVerifyCallbackBase() = default;

        virtual bool call(bool preverified, SslVerifyContext& ctx) = 0;
    };

    template <typename VerifyCallback>
    class SslVerifyCallback: public SslVerifyCallbackBase {
    public:
        explicit SslVerifyCallback(VerifyCallback&& callback)
            : _callback(std::forward<VerifyCallback>(callback)){

        }

        bool call(bool preverified, SslVerifyContext& ctx) override {
            return _callback(preverified, ctx);
        }

    private:
        VerifyCallback _callback;
    };

    class EASY_EVENT_API HostNameVerification {
    public:
        typedef bool ResultType;

        explicit HostNameVerification(const std::string& host)
            : _host(host){
        }

        bool operator()(bool preverified, SslVerifyContext& ctx) const;

    private:
        std::string _host;
    };

    enum class SslPasswordPurpose {
        ForReading,
        ForWriting
    };

    class EASY_EVENT_API SslPasswordCallbackBase {
    public:
        virtual ~SslPasswordCallbackBase() = default;

        virtual std::string call(std::size_t size, SslPasswordPurpose purpose) = 0;
    };

    template <typename PasswordCallback>
    class SslPasswordCallback : public SslPasswordCallbackBase {
    public:
        explicit SslPasswordCallback(PasswordCallback callback)
            : _callback(std::forward<PasswordCallback>(callback)) {

        }

        std::string call(std::size_t size, SslPasswordPurpose purpose) override {
            return _callback(size, purpose);
        }

    private:
        PasswordCallback _callback;
    };

    enum class SslServerOrClient {
        Client,
        Server
    };

}

namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::SslErrors>: public true_type {};

    template <>
    struct is_error_code_enum<EasyEvent::SslStreamErrors>: public true_type {};
}

#endif //EASYEVENT_SSL_TYPES_H
