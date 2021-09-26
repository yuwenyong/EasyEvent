//
// Created by yuwenyong.vincent on 2021/7/31.
//

#include "EasyEvent/Compress/Zlib.h"
#include "EasyEvent/Common/Error.h"

const char * EasyEvent::ZlibErrorCategory::name() const noexcept {
    return "zlib error";
}

std::string EasyEvent::ZlibErrorCategory::message(int ev) const {
    switch (static_cast<ZlibErrors>(ev)) {
        case ZlibErrors::OutOfMemory:
            return "Out of memory";
        case ZlibErrors::BadCompressionLevel:
            return "Bad compression level";
        case ZlibErrors::VersionMismatch:
            return "library version mismatch";
        case ZlibErrors::BufError:
            return "incomplete or truncated stream";
        case ZlibErrors::StreamError:
            return "inconsistent stream data";
        case ZlibErrors::DataError:
            return "invalid input data";
        case ZlibErrors::InvalidOption:
            return "invalid initialization option";
        case ZlibErrors::UnknownError:
            return "unknown zlib error";
        default:
            return "unknown zlib error";
    }
}

const std::error_category& EasyEvent::getZlibErrorCategory() {
    static const ZlibErrorCategory errCategory{};
    return errCategory;
}

unsigned long EasyEvent::Zlib::adler32(const std::byte *data, size_t len, unsigned long value) {
    while (len > UINT_MAX) {
        value = ::adler32(value, (Bytef *)data, UINT_MAX);
        data += (size_t)UINT_MAX;
        len -= (size_t)UINT_MAX;
    }
    return ::adler32(value, (Bytef *)data, (unsigned int)len);
}

unsigned long EasyEvent::Zlib::crc32(const std::byte *data, size_t len, unsigned long value) {
    while (len > UINT_MAX) {
        value = ::crc32(value, (const Bytef *)data, UINT_MAX);
        data += (size_t)UINT_MAX;
        len -= (size_t)UINT_MAX;
    }
    return ::crc32(value, (const Bytef *)data, (unsigned int)len);
}

ByteArray EasyEvent::Zlib::compress(const std::byte *data, size_t len, CompressLevel level) {
    ByteArray retVal;
    size_t oBufLen = DefBufSize, iBufLen = len;
    int err, flush;
    z_stream zst;

    zst.opaque = NULL;
    zst.zalloc = (alloc_func)NULL;
    zst.zfree = (free_func)Z_NULL;
    zst.next_in = (Bytef *)data;
    err = deflateInit(&zst, static_cast<int>(level));

    switch (err) {
        case Z_OK:
            break;
        case Z_STREAM_ERROR:
            throwError(ZlibErrors::OutOfMemory, "Zlib::compress");
            break;
        default:
            deflateEnd(&zst);
            handleError(&zst, err, "Zlib::compress");
            break;
    }

    do {
        arrangeInputBuffer(&zst, &iBufLen);
        flush = iBufLen == 0 ? Z_FINISH : Z_NO_FLUSH;

        do {
            oBufLen = arrangeOutputBuffer(&zst, retVal, oBufLen);
            if (oBufLen == 0) {
                deflateEnd(&zst);
                throwError(ZlibErrors::OutOfMemory, "Zlib::compress");
            }

            err = deflate(&zst, flush);

            if (err == Z_STREAM_ERROR) {
                deflateEnd(&zst);
                handleError(&zst, err, "Zlib::compress");
            }
        } while (zst.avail_out == 0);
        Assert(zst.avail_in == 0);

    } while (flush != Z_FINISH);
    Assert(err == Z_STREAM_END);

    err = deflateEnd(&zst);
    if (err != Z_OK) {
        handleError(&zst, err, "Zlib::compress");
    }
    retVal.resize((size_t)(zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

std::string EasyEvent::Zlib::compressToString(const std::byte *data, size_t len, CompressLevel level) {
    std::string retVal;
    size_t oBufLen = DefBufSize, iBufLen = len;
    int err, flush;
    z_stream zst;

    zst.opaque = NULL;
    zst.zalloc = (alloc_func)NULL;
    zst.zfree = (free_func)Z_NULL;
    zst.next_in = (Bytef *)data;
    err = deflateInit(&zst, static_cast<int>(level));

    switch (err) {
        case Z_OK:
            break;

        case Z_STREAM_ERROR:
            throwError(ZlibErrors::OutOfMemory, "Zlib::compress");
            break;

        default:
            deflateEnd(&zst);
            handleError(&zst, err, "Zlib::compress");
            break;
    }

    do {
        arrangeInputBuffer(&zst, &iBufLen);
        flush = iBufLen == 0 ? Z_FINISH : Z_NO_FLUSH;

        do {
            oBufLen = arrangeOutputBuffer(&zst, retVal, oBufLen);
            if (oBufLen == 0) {
                deflateEnd(&zst);
                throwError(ZlibErrors::OutOfMemory, "Zlib::compress");
            }

            err = deflate(&zst, flush);

            if (err == Z_STREAM_ERROR) {
                deflateEnd(&zst);
                handleError(&zst, err, "Zlib::compress");
            }
        } while (zst.avail_out == 0);
        Assert(zst.avail_in == 0);

    } while (flush != Z_FINISH);
    Assert(err == Z_STREAM_END);

    err = deflateEnd(&zst);
    if (err != Z_OK) {
        handleError(&zst, err, "Zlib::compress");
    }
    retVal.resize((size_t)(zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

ByteArray EasyEvent::Zlib::decompress(const std::byte *data, size_t len, int wbits) {
    ByteArray retVal;
    size_t oBufLen = DefBufSize, iBufLen = len;
    int err, flush;
    z_stream zst;

    zst.opaque = NULL;
    zst.zalloc = (alloc_func)NULL;
    zst.zfree = (free_func)Z_NULL;
    zst.avail_in = 0;
    zst.next_in = (Bytef *)data;
    err = inflateInit2(&zst, wbits);

    switch (err) {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            throwError(ZlibErrors::OutOfMemory, "Zlib::decompress");
            break;
        default:
            inflateEnd(&zst);
            handleError(&zst, err, "Zlib::decompress");
            break;
    }

    do {
        arrangeInputBuffer(&zst, &iBufLen);
        flush = iBufLen == 0 ? Z_FINISH : Z_NO_FLUSH;

        do {

            oBufLen = arrangeOutputBuffer(&zst, retVal, oBufLen);
            if (oBufLen == 0) {
                inflateEnd(&zst);
                throwError(ZlibErrors::OutOfMemory, "Zlib::decompress");
            }

            err = inflate(&zst, flush);

            switch (err) {
                case Z_OK:
                case Z_BUF_ERROR:
                case Z_STREAM_END:
                    break;
                case Z_MEM_ERROR:
                    inflateEnd(&zst);
                    throwError(ZlibErrors::OutOfMemory, "Zlib::decompress");
                    break;
                default:
                    inflateEnd(&zst);
                    handleError(&zst, err, "Zlib::decompress");
                    break;
            }
        } while (zst.avail_out == 0);

    } while (err != Z_STREAM_END && iBufLen != 0);

    if (err != Z_STREAM_END) {
        inflateEnd(&zst);
        handleError(&zst, err, "Zlib::decompress");
    }
    err = inflateEnd(&zst);
    if (err != Z_OK) {
        handleError(&zst, err, "Zlib::decompress");
    }
    retVal.resize((size_t)(zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

std::string EasyEvent::Zlib::decompressToString(const std::byte *data, size_t len, int wbits) {
    std::string retVal;
    size_t oBufLen = DefBufSize, iBufLen = len;
    int err, flush;
    z_stream zst;

    zst.opaque = NULL;
    zst.zalloc = (alloc_func)NULL;
    zst.zfree = (free_func)Z_NULL;
    zst.avail_in = 0;
    zst.next_in = (Bytef *)data;
    err = inflateInit2(&zst, wbits);

    switch (err) {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            throwError(ZlibErrors::OutOfMemory, "Zlib::decompress");
            break;
        default:
            inflateEnd(&zst);
            handleError(&zst, err, "Zlib::decompress");
            break;
    }

    do {
        arrangeInputBuffer(&zst, &iBufLen);
        flush = iBufLen == 0 ? Z_FINISH : Z_NO_FLUSH;

        do {

            oBufLen = arrangeOutputBuffer(&zst, retVal, oBufLen);
            if (oBufLen == 0) {
                inflateEnd(&zst);
                throwError(ZlibErrors::OutOfMemory, "Zlib::decompress");
            }

            err = inflate(&zst, flush);

            switch (err) {
                case Z_OK:
                case Z_BUF_ERROR:
                case Z_STREAM_END:
                    break;
                case Z_MEM_ERROR:
                    inflateEnd(&zst);
                    throwError(ZlibErrors::OutOfMemory, "Zlib::decompress");
                    break;
                default:
                    inflateEnd(&zst);
                    handleError(&zst, err, "Zlib::decompress");
                    break;
            }
        } while (zst.avail_out == 0);

    } while (err != Z_STREAM_END && iBufLen != 0);

    if (err != Z_STREAM_END) {
        inflateEnd(&zst);
        handleError(&zst, err, "Zlib::decompress");
    }
    err = inflateEnd(&zst);
    if (err != Z_OK) {
        handleError(&zst, err, "Zlib::decompress");
    }
    retVal.resize((size_t)(zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

void EasyEvent::Zlib::handleError(z_stream *zst, int err, const char *location) {
    UnusedParameter(zst);
    switch (err) {
        case Z_OK:
            break;
        case Z_VERSION_ERROR:
            throwError(ZlibErrors::VersionMismatch, location);
            break;
        case Z_BUF_ERROR:
            throwError(ZlibErrors::BufError, location);
            break;
        case Z_STREAM_ERROR:
            throwError(ZlibErrors::StreamError, location);
            break;
        case Z_DATA_ERROR:
            throwError(ZlibErrors::DataError, location);
            break;
        default:
            throwError(ZlibErrors::UnknownError, location);
            break;
    }
}

constexpr int EasyEvent::Zlib::MaxWBits;
constexpr int EasyEvent::Zlib::Deflated;
constexpr int EasyEvent::Zlib::DefMemLevel;
constexpr size_t EasyEvent::Zlib::DefBufSize;


EasyEvent::CompressObj::CompressObj(CompressLevel level, int method, int wbits, int memLevel, Strategy strategy)
        : _inited(false)
        , _level(level)
        , _method(method)
        , _wbits(wbits)
        , _memLevel(memLevel)
        , _strategy(strategy) {
    clearStream();
}

EasyEvent::CompressObj::CompressObj(const CompressObj &rhs)
        : _inited(false)
        , _level(rhs._level)
        , _method(rhs._method)
        , _wbits(rhs._wbits)
        , _memLevel(rhs._memLevel)
        , _strategy(rhs._strategy) {

    clearStream();

    if (rhs._inited) {
        int err = deflateCopy(&_zst, const_cast<z_stream *>(&rhs._zst));

        switch (err) {
            case Z_OK:
                _inited = true;
                break;
            case Z_MEM_ERROR:
                throwError(ZlibErrors::OutOfMemory, "CompressObj");
                break;
            case Z_STREAM_ERROR:
                throwError(ZlibErrors::StreamError, "CompressObj");
                break;
            default:
                Zlib::handleError(&_zst, err, "CompressObj");
                break;
        }
    }
}

EasyEvent::CompressObj::CompressObj(CompressObj &&rhs) noexcept
        : _inited(rhs._inited)
        , _level(rhs._level)
        , _method(rhs._method)
        , _wbits(rhs._wbits)
        , _memLevel(rhs._memLevel)
        , _strategy(rhs._strategy) {
    _zst = rhs._zst;
    if (rhs._inited) {
        rhs._inited = false;
        rhs.clearStream();
    }
}

EasyEvent::CompressObj& EasyEvent::CompressObj::operator=(const CompressObj &rhs) {
    if (this == &rhs) {
        return *this;
    }
    clear();
    _level = rhs._level;
    _method = rhs._method;
    _wbits = rhs._wbits;
    _memLevel = rhs._memLevel;
    _strategy = rhs._strategy;
    if (rhs._inited) {
        int err = deflateCopy(&_zst, const_cast<z_stream *>(&rhs._zst));

        switch (err) {
            case Z_OK:
                _inited = true;
                break;
            case Z_MEM_ERROR:
                throwError(ZlibErrors::OutOfMemory, "CompressObj");
                break;
            case Z_STREAM_ERROR:
                throwError(ZlibErrors::StreamError, "CompressObj");
                break;
            default:
                Zlib::handleError(&_zst, err, "CompressObj");
                break;
        }
    }
    return *this;
}

EasyEvent::CompressObj& EasyEvent::CompressObj::operator=(CompressObj &&rhs) noexcept {
    clear();
    _inited = rhs._inited;
    _level = rhs._level;
    _method = rhs._method;
    _wbits = rhs._wbits;
    _memLevel = rhs._memLevel;
    _strategy = rhs._strategy;
    _zst = rhs._zst;
    if (rhs._inited) {
       rhs._inited = false;
       rhs.clearStream();
    }
    return *this;
}

ByteArray EasyEvent::CompressObj::compress(const std::byte *data, size_t len) {
    initialize();

    ByteArray retVal;
    size_t iBufLen = len, oBufLen = Zlib::DefBufSize;
    int err;
    _zst.next_in = (Bytef *)data;

    do {
        Zlib::arrangeInputBuffer(&_zst, &iBufLen);

        do {
            oBufLen = Zlib::arrangeOutputBuffer(&_zst, retVal, oBufLen);
            if (oBufLen == 0) {
                throwError(ZlibErrors::OutOfMemory, "CompressObj::compress");
            }
            err = deflate(&_zst, Z_NO_FLUSH);
            if (err == Z_STREAM_ERROR) {
                Zlib::handleError(&_zst, err, "CompressObj::compress");
            }
        } while (_zst.avail_out == 0);
        Assert(_zst.avail_in == 0);

    } while (iBufLen != 0);

    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

std::string EasyEvent::CompressObj::compressToString(const std::byte *data, size_t len) {
    initialize();

    std::string retVal;
    size_t iBufLen = len, oBufLen = Zlib::DefBufSize;
    int err;
    _zst.next_in = (Bytef *)data;

    do {
        Zlib::arrangeInputBuffer(&_zst, &iBufLen);

        do {
            oBufLen = Zlib::arrangeOutputBuffer(&_zst, retVal, oBufLen);
            if (oBufLen == 0) {
                throwError(ZlibErrors::OutOfMemory, "CompressObj::compress");
            }
            err = deflate(&_zst, Z_NO_FLUSH);
            if (err == Z_STREAM_ERROR) {
                Zlib::handleError(&_zst, err, "CompressObj::compress");
            }
        } while (_zst.avail_out == 0);
        Assert(_zst.avail_in == 0);

    } while (iBufLen != 0);

    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

ByteArray EasyEvent::CompressObj::flush(FlushMode flushMode) {
    initialize();
    size_t oBufLen = Zlib::DefBufSize;
    int err;
    ByteArray retVal;
    if (flushMode == FlushMode::NoFlush) {
        return retVal;
    }
    _zst.avail_in = 0;
    do {
        oBufLen = Zlib::arrangeOutputBuffer(&_zst, retVal, oBufLen);
        if (oBufLen == 0) {
            throwError(ZlibErrors::OutOfMemory, "CompressObj::flush");
        }
        err = deflate(&_zst, static_cast<int>(flushMode));
        if (err == Z_STREAM_ERROR) {
            Zlib::handleError(&_zst, err, "CompressObj::flush");
        }
    } while (_zst.avail_out == 0);
    Assert(_zst.avail_in == 0);
    if (err == Z_STREAM_END && flushMode == FlushMode::Finish) {
        err = deflateEnd(&_zst);
        _inited = false;
        if (err != Z_OK) {
            Zlib::handleError(&_zst, err, "CompressObj::flush");
        }
    } else if (err != Z_OK && err != Z_BUF_ERROR) {
        Zlib::handleError(&_zst, err, "CompressObj::flush");
    }
    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

std::string EasyEvent::CompressObj::flushToString(FlushMode flushMode) {
    initialize();
    size_t oBufLen = Zlib::DefBufSize;
    int err;
    std::string retVal;
    if (flushMode == FlushMode::NoFlush) {
        return retVal;
    }
    _zst.avail_in = 0;
    do {
        oBufLen = Zlib::arrangeOutputBuffer(&_zst, retVal, oBufLen);
        if (oBufLen == 0) {
            throwError(ZlibErrors::OutOfMemory, "CompressObj::flush");
        }
        err = deflate(&_zst, static_cast<int>(flushMode));
        if (err == Z_STREAM_ERROR) {
            Zlib::handleError(&_zst, err, "CompressObj::flush");
        }
    } while (_zst.avail_out == 0);
    Assert(_zst.avail_in == 0);
    if (err == Z_STREAM_END && flushMode == FlushMode::Finish) {
        err = deflateEnd(&_zst);
        _inited = false;
        if (err != Z_OK) {
            Zlib::handleError(&_zst, err, "CompressObj::flush");
        }
    } else if (err != Z_OK && err != Z_BUF_ERROR) {
        Zlib::handleError(&_zst, err, "CompressObj::flush");
    }
    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

void EasyEvent::CompressObj::initialize() {
    if (!_inited) {
        int err = deflateInit2(&_zst, static_cast<int>(_level), _method, _wbits, _memLevel, static_cast<int>(_strategy));
        switch (err) {
            case Z_OK:
                _inited = true;
                break;
            case Z_MEM_ERROR:
                throwError(ZlibErrors::OutOfMemory, "CompressObj::initialize");
                break;
            case Z_STREAM_ERROR:
                throwError(ZlibErrors::InvalidOption, "CompressObj::initialize");
                break;
            default:
                Zlib::handleError(&_zst, err, "CompressObj::initialize");
                break;
        }
    }
}

void EasyEvent::CompressObj::clear() {
    if (_inited) {
        deflateEnd(&_zst);
        clearStream();
    }
}

EasyEvent::DecompressObj::DecompressObj(int wbits)
        : _inited(false)
        , _wbits(wbits) {
    clearStream();
}

EasyEvent::DecompressObj::DecompressObj(const DecompressObj &rhs)
        : _inited(false)
        , _wbits(rhs._wbits)
        , _unusedData(rhs._unusedData)
        , _unconsumedTail(rhs._unconsumedTail) {

    clearStream();

    if (rhs._inited) {
        int err = inflateCopy(&_zst, const_cast<z_stream *>(&rhs._zst));

        switch (err) {
            case Z_OK:
                _inited = true;
                break;
            case Z_STREAM_ERROR:
                throwError(ZlibErrors::StreamError, "DecompressObj");
                break;
            case Z_MEM_ERROR:
                throwError(ZlibErrors::OutOfMemory, "DecompressObj");
                break;
            default:
                Zlib::handleError(&_zst, err, "DecompressObj");
                break;
        }
    }
}

EasyEvent::DecompressObj::DecompressObj(DecompressObj &&rhs) noexcept
        : _inited(rhs._inited)
        , _wbits(rhs._wbits)
        , _unusedData(std::move(rhs._unusedData))
        , _unconsumedTail(std::move(rhs._unconsumedTail)) {
    _zst = rhs._zst;
    if (rhs._inited) {
        rhs._inited = false;
        rhs.clearStream();
    }
}

EasyEvent::DecompressObj& EasyEvent::DecompressObj::operator=(const DecompressObj &rhs) {
    if (this == &rhs) {
        return *this;
    }
    clear();

    _wbits = rhs._wbits;
    _unusedData = rhs._unusedData;
    _unconsumedTail = rhs._unconsumedTail;
    if (rhs._inited) {
        int err = inflateCopy(&_zst, const_cast<z_stream *>(&rhs._zst));
        switch (err) {
            case Z_OK:
                _inited = true;
                break;
            case Z_STREAM_ERROR:
                throwError(ZlibErrors::StreamError, "DecompressObj");
                break;
            case Z_MEM_ERROR:
                throwError(ZlibErrors::OutOfMemory, "DecompressObj");
                break;
            default:
                Zlib::handleError(&_zst, err, "DecompressObj");
                break;
        }
    }
    return *this;
}

EasyEvent::DecompressObj& EasyEvent::DecompressObj::operator=(DecompressObj &&rhs) noexcept {
    clear();
    _inited = rhs._inited;
    _wbits = rhs._wbits;
    _unusedData = std::move(rhs._unusedData);
    _unconsumedTail = std::move(rhs._unconsumedTail);
    _zst = rhs._zst;
    if (rhs._inited) {
        rhs._inited = false;
        rhs.clearStream();
    }
    return *this;
}

ByteArray EasyEvent::DecompressObj::decompress(const std::byte *data, size_t len, size_t maxLength) {
    initialize();

    int err = Z_OK;
    ByteArray retVal;
    _zst.next_in = (Bytef *)data;
    size_t iBufLen = len, oBufLen = Zlib::DefBufSize, hardLimit;
    hardLimit = maxLength == 0 ? std::numeric_limits<size_t>::max() : maxLength;

    if (maxLength > 0 && oBufLen > maxLength) {
        oBufLen = maxLength;
    }

    bool saveUnconsumed = false;
    do {
        Zlib::arrangeInputBuffer(&_zst, &iBufLen);

        do {
            oBufLen = Zlib::arrangeOutputBufferWithMaximum(&_zst, retVal, oBufLen, hardLimit);

            if (oBufLen == 0) {
                throwError(ZlibErrors::OutOfMemory, "DecompressObj::decompress");
            }

            err = inflate(&_zst, Z_SYNC_FLUSH);

            switch (err) {
                case Z_OK:
                case Z_BUF_ERROR:
                case Z_STREAM_END:
                    break;
                default:
                    saveUnconsumed = true;
                    break;
            }
        } while (!saveUnconsumed && _zst.avail_out == 0);

    } while (!saveUnconsumed && err != Z_STREAM_END && iBufLen != 0);

    saveUnconsumedInput(data, len, err);

    if (err == Z_STREAM_END) {

    } else if (err != Z_OK && err != Z_BUF_ERROR) {
        Zlib::handleError(&_zst, err, "DecompressObj::decompress");
    }

    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

std::string EasyEvent::DecompressObj::decompressToString(const std::byte *data, size_t len, size_t maxLength) {
    initialize();

    int err = Z_OK;
    std::string retVal;
    _zst.next_in = (Bytef *)data;
    size_t iBufLen = len, oBufLen = Zlib::DefBufSize, hardLimit;
    hardLimit = maxLength == 0 ? std::numeric_limits<size_t>::max() : maxLength;

    if (maxLength > 0 && oBufLen > maxLength) {
        oBufLen = maxLength;
    }

    bool saveUnconsumed = false;
    do {
        Zlib::arrangeInputBuffer(&_zst, &iBufLen);

        do {
            oBufLen = Zlib::arrangeOutputBufferWithMaximum(&_zst, retVal, oBufLen, hardLimit);

            if (oBufLen == 0) {
                throwError(ZlibErrors::OutOfMemory, "DecompressObj::decompress");
            }

            err = inflate(&_zst, Z_SYNC_FLUSH);

            switch (err) {
                case Z_OK:
                case Z_BUF_ERROR:
                case Z_STREAM_END:
                    break;
                default:
                    saveUnconsumed = true;
                    break;
            }
        } while (!saveUnconsumed && _zst.avail_out == 0);

    } while (!saveUnconsumed && err != Z_STREAM_END && iBufLen != 0);

    saveUnconsumedInput(data, len, err);

    if (err == Z_STREAM_END) {

    } else if (err != Z_OK && err != Z_BUF_ERROR) {
        Zlib::handleError(&_zst, err, "DecompressObj::decompress");
    }

    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

ByteArray EasyEvent::DecompressObj::flush() {
    initialize();

    ByteArray retVal;
    int err, flush;
    size_t oBufLen = Zlib::DefBufSize, iBufLen = _unconsumedTail.size();
    _zst.next_in = (Bytef *)_unconsumedTail.data();

    bool saveUnconsumed = false;
    do {
        Zlib::arrangeInputBuffer(&_zst, &iBufLen);
        flush = iBufLen == 0 ? Z_FINISH : Z_NO_FLUSH;

        do {
            oBufLen = Zlib::arrangeOutputBuffer(&_zst, retVal, oBufLen);

            if (oBufLen == 0) {
                throwError(ZlibErrors::OutOfMemory, "DecompressObj::flush");
            }

            err = inflate(&_zst, flush);

            switch (err) {
                case Z_OK:
                case Z_BUF_ERROR:
                case Z_STREAM_END:
                    break;
                default:
                    saveUnconsumed = true;
                    break;
            }
        } while (!saveUnconsumed && _zst.avail_out == 0);
    } while (!saveUnconsumed && err != Z_STREAM_END && iBufLen != 0);

    saveUnconsumedInput(_unconsumedTail.data(), _unconsumedTail.size(), err);
    if (err == Z_STREAM_END) {
        err = inflateEnd(&_zst);
        _inited = false;
        if (err != Z_OK) {
            Zlib::handleError(&_zst, err, "DecompressObj::flush");
        }
    }
    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

std::string EasyEvent::DecompressObj::flushToString() {
    initialize();

    std::string retVal;
    int err, flush;
    size_t oBufLen = Zlib::DefBufSize, iBufLen = _unconsumedTail.size();
    _zst.next_in = (Bytef *)_unconsumedTail.data();

    bool saveUnconsumed = false;
    do {
        Zlib::arrangeInputBuffer(&_zst, &iBufLen);
        flush = iBufLen == 0 ? Z_FINISH : Z_NO_FLUSH;

        do {
            oBufLen = Zlib::arrangeOutputBuffer(&_zst, retVal, oBufLen);

            if (oBufLen == 0) {
                throwError(ZlibErrors::OutOfMemory, "DecompressObj::flush");
            }

            err = inflate(&_zst, flush);

            switch (err) {
                case Z_OK:
                case Z_BUF_ERROR:
                case Z_STREAM_END:
                    break;
                default:
                    saveUnconsumed = true;
                    break;
            }
        } while (!saveUnconsumed && _zst.avail_out == 0);
    } while (!saveUnconsumed && err != Z_STREAM_END && iBufLen != 0);

    saveUnconsumedInput(_unconsumedTail.data(), _unconsumedTail.size(), err);
    if (err == Z_STREAM_END) {
        err = inflateEnd(&_zst);
        _inited = false;
        if (err != Z_OK) {
            Zlib::handleError(&_zst, err, "DecompressObj::flush");
        }
    }
    retVal.resize((size_t)(_zst.next_out - (Bytef *)retVal.data()));
    return retVal;
}

void EasyEvent::DecompressObj::saveUnconsumedInput(const std::byte *data, size_t len, int err) {
    if (err == Z_STREAM_END) {
        if (_zst.avail_in > 0) {
            size_t oldSize = _unusedData.size();
            auto leftSize = (size_t)(data + len - (std::byte *)_zst.next_in);
            if (leftSize > std::numeric_limits<size_t>::max() - oldSize) {
                throwError(ZlibErrors::OutOfMemory, "DecompressObj::saveUnconsumedInput");
            }

            _unusedData.insert(_unusedData.end(), (std::byte *)_zst.next_in, (std::byte *)_zst.next_in + leftSize);
            _zst.avail_in = 0;
        }
    }
    if (_zst.avail_in > 0 || !_unconsumedTail.empty()) {
        auto leftSize = (size_t)(data + len - (std::byte *)_zst.next_in);
        _unconsumedTail.assign((std::byte *)_zst.next_in, (std::byte *)_zst.next_in + leftSize);
    }
}

void EasyEvent::DecompressObj::initialize() {
    if (!_inited) {
        int err = inflateInit2(&_zst, _wbits);
        switch (err) {
            case Z_OK:
                _inited = true;
                break;
            case Z_MEM_ERROR:
                throwError(ZlibErrors::OutOfMemory, "DecompressObj::initialize");
                break;
            case Z_STREAM_ERROR:
                throwError(ZlibErrors::InvalidOption, "DecompressObj::initialize");
                break;
            default:
                Zlib::handleError(&_zst, err, "DecompressObj::initialize");
                break;
        }
    }
}

void EasyEvent::DecompressObj::clear() {
    if (_inited) {
        inflateEnd(&_zst);
        clearStream();
        _inited = false;
    }
}