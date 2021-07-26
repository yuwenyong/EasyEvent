//
// Created by yuwenyong.vincent on 2021/7/26.
//

#ifndef EASYEVENT_HTTP_HTTPBASE_H
#define EASYEVENT_HTTP_HTTPBASE_H

#include "EasyEvent/Event/EventBase.h"
#include "EasyEvent/Common/Time.h"

namespace EasyEvent {

    enum class HttpHeaderErrors {
        EmptyHeaderLine,
        NoColonInHeaderLine,
        FirstHeaderLineStartWithWhitespace,
    };

    class EASY_EVENT_API HttpHeaderErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getHttpHeaderErrorCategory();


    inline std::error_code make_error_code(HttpHeaderErrors err) {
        return {static_cast<int>(err), getHttpHeaderErrorCategory()};
    }

    class EASY_EVENT_API HttpConnectionOptions {
    public:
        static constexpr size_t DefaultChunkSize = 65535;
        static constexpr size_t DefaultMaxHeaderSize = 65535;

        bool isNoKeepAlive() const {
            return _noKeepAlive;
        }

        void setNoKeepAlive(bool noKeepAlive) {
            _noKeepAlive = noKeepAlive;
        }

        size_t getChunkSize() const {
            return _chunkSize;
        }

        void setChunkSize(size_t chunkSize) {
            _chunkSize = chunkSize;
        }

        size_t getMaxHeaderSize() const {
            return _maxHeaderSize;
        }

        void setMaxHeaderSize(size_t maxHeaderSize) {
            _maxHeaderSize = maxHeaderSize;
        }

        Time getHeaderTimeout() const {
            return _headerTimeout;
        }

        void setHeaderTimeout(Time headerTimeout) {
            _headerTimeout = headerTimeout;
        }

        size_t getMaxBodySize() const {
            return _maxBodySize;
        }

        void setMaxBodySize(size_t maxBodySize) {
            _maxBodySize = maxBodySize;
        }

        Time getBodyTimeout() const {
            return _bodyTimeout;
        }

        void setBodyTimeout(Time bodyTimeout) {
            _bodyTimeout = bodyTimeout;
        }

        bool isDecompressEnabled() const {
            return _decompress;
        }

        void enableDecompress(bool enable) {
            _decompress = enable;
        }

        static const HttpConnectionOptions Default;
    private:
        bool _noKeepAlive{false};
        size_t _chunkSize{DefaultChunkSize};
        size_t _maxHeaderSize{DefaultMaxHeaderSize};
        Time _headerTimeout{};
        size_t _maxBodySize{0};
        Time _bodyTimeout{};
        bool _decompress{false};
    };

}


namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::HttpHeaderErrors>: public true_type {};

}

#endif //EASYEVENT_HTTP_HTTPBASE_H
