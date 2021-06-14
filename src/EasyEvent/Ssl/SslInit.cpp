//
// Created by yuwenyong.vincent on 2021/6/6.
//

#include "EasyEvent/Ssl/SslInit.h"


class EasyEvent::SslInit::DoInit {
public:
    DoInit() {
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
        ::SSL_library_init();
        ::SSL_load_error_strings();
        ::OpenSSL_add_all_algorithms();

        _mutexes.resize(::CRYPTO_num_locks());
        for (size_t i = 0; i < _mutexes.size(); ++i) {
            _mutexes[i] = std::make_shared<std::mutex>();
        }

        ::CRYPTO_set_locking_callback(&DoInit::OpenSslLockingFunc);
#endif

#if (OPENSSL_VERSION_NUMBER < 0x10000000L)
        ::CRYPTO_set_id_callback(&DoInit::OpenSslIdFunc);
#endif

#if !defined(SSL_OP_NO_COMPRESSION) && (OPENSSL_VERSION_NUMBER >= 0x00908000L)
        _nullCompressionMethods = sk_SSL_COMP_new_null();
#endif

    }

    ~DoInit() {
#if !defined(SSL_OP_NO_COMPRESSION) && (OPENSSL_VERSION_NUMBER >= 0x00908000L)
        sk_SSL_COMP_free(_nullCompressionMethods);
#endif

#if (OPENSSL_VERSION_NUMBER < 0x10000000L)
        ::CRYPTO_set_id_callback(0);
#endif

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
        ::CRYPTO_set_locking_callback(0);
        ::ERR_free_strings();
        ::EVP_cleanup();
        ::CRYPTO_cleanup_all_ex_data();
#endif

#if (OPENSSL_VERSION_NUMBER < 0x10000000L)
        ::ERR_remove_state(0);
#elif (OPENSSL_VERSION_NUMBER < 0x10100000L)
        ::ERR_remove_thread_state(NULL);
#endif

#if (OPENSSL_VERSION_NUMBER >= 0x10002000L) && (OPENSSL_VERSION_NUMBER < 0x10100000L) && !defined(SSL_OP_NO_COMPRESSION)
        ::SSL_COMP_free_compression_methods();
#endif

#if !defined(OPENSSL_IS_BORINGSSL)
        ::CONF_modules_unload(1);
#endif

#if !defined(OPENSSL_NO_ENGINE) && (OPENSSL_VERSION_NUMBER < 0x10100000L)
        ::ENGINE_cleanup();
#endif
    }

#if !defined(SSL_OP_NO_COMPRESSION) && (OPENSSL_VERSION_NUMBER >= 0x00908000L)
    STACK_OF(SSL_COMP)* getNullCompressionMethods() const {
        return _nullCompressionMethods;
    }
#endif

private:
#if (OPENSSL_VERSION_NUMBER < 0x10000000L)
    static unsigned long OpenSslIdFunc() {
#   if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
        return ::GetCurrentThreadId();
#   else
        void* id = &errno;
        Assert(sizeof(unsigned long) >= sizeof(void*));
        return reinterpret_cast<unsigned long>(id);
#   endif
    }
#endif

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
    static void OpenSslLockingFunc(int mode, int n, const char* /*file*/, int /*line*/) {
        if (mode & CRYPTO_LOCK) {
            instance()->_mutexes[n]->lock();
        }
        else {
            instance()->_mutexes[n]->unlock();
        }
    }

    std::vector<std::shared_ptr<std::mutex> > _mutexes;
#endif

#if !defined(SSL_OP_NO_COMPRESSION) && (OPENSSL_VERSION_NUMBER >= 0x00908000L)
    STACK_OF(SSL_COMP)* _nullCompressionMethods;
#endif
};


#if !defined(SSL_OP_NO_COMPRESSION) && (OPENSSL_VERSION_NUMBER >= 0x00908000L)
STACK_OF(SSL_COMP)* EasyEvent::SslInit::getNullCompressionMethods() {
    return instance()->getNullCompressionMethods();
}
#endif

std::shared_ptr<EasyEvent::SslInit::DoInit> EasyEvent::SslInit::instance() {
    static std::shared_ptr<DoInit> init = std::make_shared<DoInit>();
    return init;
}

EasyEvent::SslInit EasyEvent::SslInit::gInstance;