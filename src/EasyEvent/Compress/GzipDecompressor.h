//
// Created by yuwenyong.vincent on 2021/9/5.
//

#ifndef EASYEVENT_COMPRESS_GZIPDECOMPRESSOR_H
#define EASYEVENT_COMPRESS_GZIPDECOMPRESSOR_H

#include "EasyEvent/Compress/Zlib.h"

namespace EasyEvent {

    class EASY_EVENT_API GzipDecompressor {
    public:
        GzipDecompressor()
                : _decompressObj(16 + Zlib::MaxWBits) {

        }

        ByteArray decompress(const std::byte *data, size_t len, size_t maxLength=0) {
            return _decompressObj.decompress(data, len, maxLength);
        }

        ByteArray decompress(const ByteArray &data, size_t maxLength=0) {
            return _decompressObj.decompress(data, maxLength);
        }

        ByteArray decompress(const std::string &data, size_t maxLength=0) {
            return _decompressObj.decompress(data, maxLength);
        }

        ByteArray decompress(const std::string_view &data, size_t maxLength=0) {
            return _decompressObj.decompress(data, maxLength);
        }

        std::string decompressToString(const std::byte *data, size_t len, size_t maxLength=0) {
            return _decompressObj.decompressToString(data, len, maxLength);
        }

        std::string decompressToString(const ByteArray &data, size_t maxLength=0) {
            return _decompressObj.decompressToString(data, maxLength);
        }

        std::string decompressToString(const std::string &data, size_t maxLength=0) {
            return _decompressObj.decompressToString(data, maxLength);
        }

        std::string decompressToString(const std::string_view &data, size_t maxLength=0) {
            return _decompressObj.decompressToString(data, maxLength);
        }

        ByteArray flush() {
            return _decompressObj.flush();
        }

        std::string flushToString() {
            return _decompressObj.flushToString();
        }

        const ByteArray& getUnconsumedTail() const {
            return _decompressObj.getUnconsumedTail();
        }
    protected:
        DecompressObj _decompressObj;
    };

}

#endif //EASYEVENT_COMPRESS_GZIPDECOMPRESSOR_H
