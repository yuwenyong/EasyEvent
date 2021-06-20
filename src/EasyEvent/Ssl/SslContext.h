//
// Created by yuwenyong.vincent on 2021/6/13.
//

#ifndef EASYEVENT_SSL_SSLCONTEXT_H
#define EASYEVENT_SSL_SSLCONTEXT_H

#include "EasyEvent/Ssl/Types.h"
#include "EasyEvent/Ssl/SslInit.h"


namespace EasyEvent {

    class EASY_EVENT_API SslContext: NonCopyable {
    private:
        struct MakeSharedTag {};

    public:
        typedef SSL_CTX* NativeHandleType;

        explicit SslContext(SslProtoVersion protoVersion, MakeSharedTag tag);

        SslContext(SslContext&& other) noexcept {
            _handle = other._handle;
            other._handle = nullptr;
        }

        SslContext& operator=(SslContext&& other) noexcept {
            SslContext tmp(std::move(*this));
            _handle = other._handle;
            other._handle = nullptr;
            return *this;
        }

        ~SslContext() noexcept;

        NativeHandleType  nativeHandle() {
            return _handle;
        }

        void clearOptions(SslOptions options);

        void clearOptions(SslOptions options, std::error_code& ec);

        void setOptions(SslOptions options);

        void setOptions(SslOptions options, std::error_code& ec);

        void setVerifyMode(SslVerifyMode verifyMode);

        void setVerifyMode(SslVerifyMode verifyMode, std::error_code& ec);

        void loadVerifyFile(const std::string& filename);

        void loadVerifyFile(const std::string& filename, std::error_code& ec);

        void setDefaultVerifyPaths();

        void setDefaultVerifyPaths(std::error_code& ec);

        void addVerifyPath(const std::string& path);

        void addVerifyPath(const std::string& path, std::error_code& ec);

        void useCertificateChainFile(const std::string& filename);

        void useCertificateChainFile(const std::string& filename, std::error_code& ec);

        void usePrivateKeyFile(const std::string& filename);

        void usePrivateKeyFile(const std::string& filename, std::error_code& ec);

        template<class VerifyCallback>
        void setVerifyCallback(VerifyCallback&& callback) {
            std::error_code ec;
            this->template setVerifyCallback(std::forward<VerifyCallback>(callback), ec);
            throwError(ec, "setVerifyCallback");
        }

        template<class VerifyCallback>
        void setVerifyCallback(VerifyCallback&& callback, std::error_code& ec) {
            doSetVerifyCallback(new SslVerifyCallback<VerifyCallback>(std::forward<VerifyCallback>(callback)), ec);
        }

        template<class PasswordCallback>
        void setPasswordCallback(PasswordCallback callback) {
            std::error_code ec;
            this->template setPasswordCallback(std::forward<PasswordCallback>(callback), ec);
            throwError(ec, "setPasswordCallback");
        }

        template<class PasswordCallback>
        void setPasswordCallback(PasswordCallback callback, std::error_code& ec) {
            doSetPasswordCallback(new SslPasswordCallback<PasswordCallback>(std::forward<PasswordCallback>(callback)), ec);
        }

        void setPassword(const std::string& password) {
            std::error_code ec;
            setPassword(password, ec);
            throwError(ec, "setPassword");
        }

        void setPassword(const std::string& password, std::error_code& ec) {
            setPasswordCallback([password](size_t, SslPasswordPurpose) {
                return password;
            }, ec);
        }

        void setVerifyHostName(const std::string& hostName) {
            std::error_code ec;
            setVerifyHostName(hostName, ec);
            throwError(ec, "setVerifyHostName");
        }

        void setVerifyHostName(const std::string& hostName, std::error_code& ec) {
            setVerifyCallback(HostNameVerification(hostName), ec);
        }

        static std::shared_ptr<SslContext> createContext(SslProtoVersion protoVersion=SslProtoVersion::SslV23) {
            return std::make_shared<SslContext>(protoVersion, MakeSharedTag{});
        }

        static std::shared_ptr<SslContext> createDefaultContext(
                SslServerOrClient socketType, const std::string& caFile="", const std::string& certFile="",
                const std::string& keyFile="");

        static std::shared_ptr<SslContext> createDefaultClientContext() {
            return createDefaultContext(SslServerOrClient::Client);
        }

        static std::shared_ptr<SslContext> createDefaultServerContext() {
            return createDefaultContext(SslServerOrClient::Server);
        }
    protected:
        void doSetVerifyCallback(SslVerifyCallbackBase* callback, std::error_code& ec);

        static int verifyCallbackFunction(int preverified, X509_STORE_CTX* ctx);

        void doSetPasswordCallback(SslPasswordCallbackBase* callback, std::error_code& ec);

        static int passwordCallbackFunction(char* buf, int size, int purpose, void* data);

        NativeHandleType _handle;
        SslInit _init;
    };

    using SslContextPtr = std::shared_ptr<SslContext>;

}

#endif //EASYEVENT_SSL_SSLCONTEXT_H
