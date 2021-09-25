//
// Created by yuwenyong.vincent on 2021/7/31.
//

#ifndef EASYEVENT_COMPRESS_ZLIB_H
#define EASYEVENT_COMPRESS_ZLIB_H

#include "EasyEvent/Common/Config.h"
#include <zlib.h>

#define DEFLATED   8
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#define DEF_BUF_SIZE (16*1024)


namespace EasyEvent {

    enum class ZlibErrors {
        OutOfMemory = 1,
        BadCompressionLevel,
        VersionMismatch,
        BufError,
        StreamError,
        DataError,
        InvalidOption,
        UnknownError,
    };

    class EASY_EVENT_API ZlibErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getZlibErrorCategory();


    inline std::error_code make_error_code(ZlibErrors err) {
        return {static_cast<int>(err), getZlibErrorCategory()};
    }

    enum class CompressLevel: int {
        NoCompression = Z_NO_COMPRESSION,
        BestSpeed = Z_BEST_SPEED,
        BestCompression = Z_BEST_COMPRESSION,
        Default = Z_DEFAULT_COMPRESSION,
    };

    enum class Strategy: int {
        DefaultStrategy = Z_DEFAULT_STRATEGY,
        Filtered = Z_FILTERED,
        HuffmanOnly = Z_HUFFMAN_ONLY,
#ifdef Z_RLE
        Rle = Z_RLE,
#endif
#ifdef Z_FIXED
        Fixed = Z_FIXED,
#endif
    };

    enum class FlushMode: int {
        NoFlush = Z_NO_FLUSH,
        PartialFlush = Z_PARTIAL_FLUSH,
        SyncFlush = Z_SYNC_FLUSH,
        FullFlush = Z_FULL_FLUSH,
        Finish = Z_FINISH,
#ifdef Z_BLOCK
        Block = Z_BLOCK,
#endif
    };

    class EASY_EVENT_API Zlib {
    public:
        static unsigned long adler32(const std::byte *data, size_t len, unsigned long value=1);

        static unsigned long adler32(const ByteArray &data, unsigned long value=1) {
            return adler32(data.data(), data.size(), value);
        }

        static unsigned long adler32(const char *data, unsigned long value=1) {
            return adler32((const std::byte *)data, strlen(data), value);
        }

        static unsigned long adler32(const std::string &data, unsigned long value=1) {
            return adler32((const std::byte *)data.data(), data.size(), value);
        }

        static unsigned long adler32(const std::string_view &data, unsigned long value=1) {
            return adler32((const std::byte *)data.data(), data.size(), value);
        }

        static unsigned long crc32(const std::byte *data, size_t len, unsigned long value=0);

        static unsigned long crc32(const ByteArray &data, unsigned long value=0) {
            return crc32(data.data(), data.size(), value);
        }

        static unsigned long crc32(const char *data, unsigned long value=0) {
            return crc32((const std::byte *)data, strlen(data), value);
        }

        static unsigned long crc32(const std::string &data, unsigned long value=0) {
            return crc32((const std::byte *)data.data(), data.size(), value);
        }

        static unsigned long crc32(const std::string_view &data, unsigned long value=0) {
            return crc32((const std::byte *)data.data(), data.size(), value);
        }

        static ByteArray compress(const std::byte *data, size_t len, CompressLevel level=CompressLevel::Default);

        static ByteArray compress(const ByteArray &data, CompressLevel level=CompressLevel::Default) {
            return compress(data.data(), data.size(), level);
        }

        static ByteArray compress(const char *data, CompressLevel level=CompressLevel::Default) {
            return compress((const std::byte *)data, strlen(data), level);
        }

        static ByteArray compress(const std::string &data, CompressLevel level=CompressLevel::Default) {
            return compress((const std::byte *)data.data(), data.size(), level);
        }

        static ByteArray compress(const std::string_view &data, CompressLevel level=CompressLevel::Default) {
            return compress((const std::byte *)data.data(), data.size(), level);
        }

        static std::string compressToString(const std::byte *data, size_t len, CompressLevel level=CompressLevel::Default);

        static std::string compressToString(const ByteArray &data, CompressLevel level=CompressLevel::Default) {
            return compressToString(data.data(), data.size(), level);
        }

        static std::string compressToString(const char* data, CompressLevel level=CompressLevel::Default) {
            return compressToString((const std::byte *)data, strlen(data), level);
        }

        static std::string compressToString(const std::string &data, CompressLevel level=CompressLevel::Default) {
            return compressToString((const std::byte *)data.data(), data.length(), level);
        }

        static std::string compressToString(const std::string_view &data, CompressLevel level=CompressLevel::Default) {
            return compressToString((const std::byte *)data.data(), data.length(), level);
        }

        static ByteArray decompress(const std::byte *data, size_t len, int wbits=MaxWBits);

        static ByteArray decompress(const ByteArray &data, int wbits=MaxWBits) {
            return decompress(data.data(), data.size(), wbits);
        }

        static ByteArray decompress(const std::string &data, int wbits=MaxWBits) {
            return decompress((const std::byte *)data.data(), data.size(), wbits);
        }

        static ByteArray decompress(const std::string_view &data, int wbits=MaxWBits) {
            return decompress((const std::byte *)data.data(), data.size(), wbits);
        }

        static std::string decompressToString(const std::byte *data, size_t len, int wbits=MaxWBits);

        static std::string decompressToString(const ByteArray &data, int wbits=MaxWBits) {
            return decompressToString(data.data(), data.size(), wbits);
        }

        static std::string decompressToString(const std::string &data, int wbits=MaxWBits) {
            return decompressToString((const std::byte *)data.data(), data.size(), wbits);
        }

        static std::string decompressToString(const std::string_view &data, int wbits=MaxWBits) {
            return decompressToString((const std::byte *)data.data(), data.size(), wbits);
        }

        static void handleError(z_stream *zst, int err, const char *location);

        static void arrangeInputBuffer(z_stream *zst, size_t *remains) {
            zst->avail_in = std::min(*remains, (size_t)UINT_MAX);
            *remains -= zst->avail_in;
        }

        template<typename BufferType>
        static size_t arrangeOutputBuffer(z_stream *zst, BufferType &buffer, size_t length) {
            return arrangeOutputBufferWithMaximum(zst, buffer, length, std::numeric_limits<size_t>::max());
        }

        template<typename BufferType>
        static size_t arrangeOutputBufferWithMaximum(z_stream *zst, BufferType &buffer, size_t length, size_t maxLength) {
            size_t occupied;
            if (buffer.empty()) {
                buffer.resize(length);
                occupied = 0;
            } else {
                occupied = (size_t)(zst->next_out - (Bytef *)buffer.data());
                if (occupied == length) {
                    size_t newLength;
                    Assert(length <= maxLength);
                    if (length == maxLength) {
                        return 0;
                    }
                    if (length <= (maxLength >> 1)) {
                        newLength = length << 1;
                    } else {
                        newLength = maxLength;
                    }
                    buffer.resize(newLength);
                    length = newLength;
                }
            }
            zst->avail_out = (unsigned int)std::min((length - occupied), (size_t)UINT_MAX);
            zst->next_out = (Bytef *)buffer.data() + occupied;
            return length;
        }

        static constexpr int MaxWBits = MAX_WBITS;
        static constexpr int Deflated = DEFLATED;
        static constexpr int DefMemLevel = DEF_MEM_LEVEL;
        static constexpr size_t DefBufSize = DEF_BUF_SIZE;
    };

    class EASY_EVENT_API CompressObj {
    public:
        explicit CompressObj(CompressLevel level, int method=Zlib::Deflated, int wbits=Zlib::MaxWBits,
                             int memLevel=Zlib::DefMemLevel, Strategy strategy=Strategy::DefaultStrategy);

        CompressObj(const CompressObj &rhs);

        CompressObj(CompressObj &&rhs) noexcept ;

        CompressObj& operator=(const CompressObj &rhs);

        CompressObj& operator=(CompressObj &&rhs) noexcept;

        ~CompressObj() {
            clear();
        }

        ByteArray compress(const std::byte *data, size_t len);

        ByteArray compress(const ByteArray &data) {
            return compress(data.data(), data.size());
        }

        ByteArray compress(const char *data) {
            return compress((const std::byte *)data, strlen(data));
        }

        ByteArray compress(const std::string &data) {
            return compress((const std::byte *)data.data(), data.size());
        }

        ByteArray compress(const std::string_view &data) {
            return compress((const std::byte *)data.data(), data.size());
        }

        std::string compressToString(const std::byte *data, size_t len);

        std::string compressToString(const ByteArray &data) {
            return compressToString(data.data(), data.size());
        }

        std::string compressToString(const char *data) {
            return compressToString((const std::byte *)data, strlen(data));
        }

        std::string compressToString(const std::string &data) {
            return compressToString((const std::byte *)data.data(), data.size());
        }

        std::string compressToString(const std::string_view &data) {
            return compressToString((const std::byte *)data.data(), data.size());
        }

        ByteArray flush(FlushMode flushMode=FlushMode::Finish);

        std::string flushToString(FlushMode flushMode=FlushMode::Finish);
    protected:
        void initialize();

        void clear();

        void clearStream() {
            _zst.opaque = NULL;
            _zst.zalloc = (alloc_func) NULL;
            _zst.zfree = (free_func) Z_NULL;
            _zst.next_in = NULL;
            _zst.avail_in = 0;
        }

        bool _inited;
        CompressLevel _level;
        int _method;
        int _wbits;
        int _memLevel;
        Strategy _strategy;
        z_stream _zst;
    };


    class EASY_EVENT_API DecompressObj {
    public:
        explicit DecompressObj(int wbits=Zlib::MaxWBits);

        DecompressObj(const DecompressObj &rhs);

        DecompressObj(DecompressObj &&rhs) noexcept;

        DecompressObj& operator=(const DecompressObj &rhs);

        DecompressObj& operator=(DecompressObj &&rhs) noexcept;

        ~DecompressObj() {
            clear();
        }

        ByteArray decompress(const std::byte *data, size_t len, size_t maxLength=0);

        ByteArray decompress(const ByteArray &data, size_t maxLength=0) {
            return decompress(data.data(), data.size(), maxLength);
        }

        ByteArray decompress(const std::string &data, size_t maxLength=0) {
            return decompress((const std::byte *)data.data(), data.size(), maxLength);
        }

        ByteArray decompress(const std::string_view &data, size_t maxLength=0) {
            return decompress((const std::byte *)data.data(), data.size(), maxLength);
        }

        std::string decompressToString(const std::byte *data, size_t len, size_t maxLength=0);

        std::string decompressToString(const ByteArray &data, size_t maxLength=0) {
            return decompressToString(data.data(), data.size(), maxLength);
        }

        std::string decompressToString(const std::string &data, size_t maxLength=0) {
            return decompressToString((const std::byte *)data.data(), data.size(), maxLength);
        }

        std::string decompressToString(const std::string_view &data, size_t maxLength=0) {
            return decompressToString((const std::byte *)data.data(), data.size(), maxLength);
        }

        ByteArray flush();

        std::string flushToString();

        const ByteArray& getUnusedData() const {
            return _unusedData;
        }

        const ByteArray& getUnconsumedTail() const {
            return _unconsumedTail;
        }
    protected:
        void saveUnconsumedInput(const std::byte *data, size_t len, int err);

        void initialize();

        void clear();

        void clearStream() {
            _zst.opaque = NULL;
            _zst.zalloc = (alloc_func) NULL;
            _zst.zfree = (free_func) Z_NULL;
            _zst.next_in = NULL;
            _zst.avail_in = 0;
        }

        bool _inited;
        int _wbits;
        z_stream _zst;
        ByteArray _unusedData;
        ByteArray _unconsumedTail;
    };

}

namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::ZlibErrors>: public true_type {};

}

#endif //EASYEVENT_COMPRESS_ZLIB_H
