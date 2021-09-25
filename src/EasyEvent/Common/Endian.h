//
// Created by yuwenyong.vincent on 2021/8/7.
//

#ifndef EASYEVENT_COMMON_ENDIAN_H
#define EASYEVENT_COMMON_ENDIAN_H

#include "EasyEvent/Common/Config.h"

#if defined(_MSC_VER)
#   include <cstdlib>
#   define EASY_EVENT_BYTE_SWAP_2(x) _byteswap_ushort(x)
#   define EASY_EVENT_BYTE_SWAP_4(x) _byteswap_ulong(x)
#   define EASY_EVENT_BYTE_SWAP_8(x) _byteswap_uint64(x)
#elif (defined(__clang__) && __has_builtin(__builtin_bswap32) && __has_builtin(__builtin_bswap64)) \
  || (defined(__GNUC__ ) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
#   if (defined(__clang__) && __has_builtin(__builtin_bswap16)) \
        || (defined(__GNUC__) &&(__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
#           define EASY_EVENT_BYTE_SWAP_2(x) __builtin_bswap16(x)
#   else
#           define EASY_EVENT_BYTE_SWAP_2(x) __builtin_bswap32((x) << 16)
#   endif
#   define EASY_EVENT_BYTE_SWAP_4(x) __builtin_bswap32(x)
#   define EASY_EVENT_BYTE_SWAP_8(x) __builtin_bswap64(x)
#elif defined(__linux__)
#   include <byteswap.h>
#   define EASY_EVENT_BYTE_SWAP_2(x) bswap_16(x)
#   define EASY_EVENT_BYTE_SWAP_4(x) bswap_32(x)
#   define EASY_EVENT_BYTE_SWAP_8(x) bswap_64(x)
#endif

namespace EasyEvent {

    class EASY_EVENT_API Endian {
    public:
        template<typename T>
        static T StdEndianReverse(T x) noexcept {
            T tmp(x);
            std::reverse(reinterpret_cast<unsigned char*>(&tmp),
                         reinterpret_cast<unsigned char*>(&tmp) + sizeof(T));
            return tmp;
        }

        static int8_t EndianReverse(int8_t x) noexcept {
            return x;
        }

        static int16_t EndianReverse(int16_t x) noexcept {
#ifdef EASY_EVENT_BYTE_SWAP_2
            return static_cast<int16_t>(EASY_EVENT_BYTE_SWAP_2(static_cast<uint16_t>(x)));
#else
            return static_cast<int16_t>((static_cast<uint16_t>(x) << 8) | (static_cast<uint16_t>(x) >> 8));
#endif
        }

        static int32_t EndianReverse(int32_t x) noexcept {
#ifdef EASY_EVENT_BYTE_SWAP_4
            return static_cast<int32_t>(EASY_EVENT_BYTE_SWAP_4(static_cast<uint32_t>(x)));
#else
            uint32_t step16;
            step16 = static_cast<uint32_t>(x) << 16 | static_cast<uint32_t>(x) >> 16;
            return static_cast<int32_t>(
                    ((static_cast<uint32_t>(step16) << 8) & 0xff00ff00)
                    | ((static_cast<uint32_t>(step16) >> 8) & 0x00ff00ff));
#endif
        }

        static int64_t EndianReverse(int64_t x) noexcept {
#ifdef EASY_EVENT_BYTE_SWAP_8
            return static_cast<int64_t>(EASY_EVENT_BYTE_SWAP_4(static_cast<uint64_t>(x)));
#else
            uint64_t step32, step16;
            step32 = static_cast<uint64_t>(x) << 32 | static_cast<uint64_t>(x) >> 32;
            step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16
                     | (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
            return static_cast<int64_t>((step16 & 0x00FF00FF00FF00FFULL) << 8 | (step16 & 0xFF00FF00FF00FF00ULL) >> 8);
#endif
        }

        static uint8_t EndianReverse(uint8_t x) noexcept {
            return x;
        }

        static uint16_t EndianReverse(uint16_t x) noexcept {
#ifdef EASY_EVENT_BYTE_SWAP_2
            return EASY_EVENT_BYTE_SWAP_2(x);
#else
            return (x << 8) | (x >> 8);
#endif
        }

        static uint32_t EndianReverse(uint32_t x) noexcept {
#ifdef EASY_EVENT_BYTE_SWAP_4
            return EASY_EVENT_BYTE_SWAP_4(x);
#else
            uint32_t step16;
            step16 = x << 16 | x >> 16;
            return ((step16 << 8) & 0xff00ff00) | ((step16 >> 8) & 0x00ff00ff);
#endif
        }

        static uint64_t EndianReverse(uint64_t x) noexcept {
#ifdef EASY_EVENT_BYTE_SWAP_8
            return EASY_EVENT_BYTE_SWAP_4(x);
#else
            uint64_t step32, step16;
            step32 = x << 32 | x >> 32;
            step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16 | (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
            return (step16 & 0x00FF00FF00FF00FFULL) << 8 | (step16 & 0xFF00FF00FF00FF00ULL) >> 8;
#endif
        }

        template<typename T>
        static T BigToNative(T x) noexcept {
#ifdef EASY_EVENT_BIG_ENDIAN
            return x;
#else
            return EndianReverse(x);
#endif
        }

        template<typename T>
        static T NativeToBig(T x) noexcept {
#ifdef EASY_EVENT_BIG_ENDIAN
            return x;
#else
            return EndianReverse(x);
#endif
        }

        template<typename T>
        static T LittleToNative(T x) noexcept {
#ifdef EASY_EVENT_LITTLE_ENDIAN
            return x;
#else
            return EndianReverse(x);
#endif
        }

        template<typename T>
        static T NativeToLittle(T x) noexcept {
#ifdef EASY_EVENT_LITTLE_ENDIAN
            return x;
#else
            return EndianReverse(x);
#endif
        }

        template<typename T>
        static void EndianReverseInplace(T &x) noexcept {
            x = EndianReverse(x);
        }

        template<typename T>
        static void BigToNativeInplace(T &x) noexcept {
#ifdef EASY_EVENT_BIG_ENDIAN
            UnusedParameter(x);
#else
            EndianReverseInplace(x);
#endif
        }

        template<typename T>
        static void NativeToBigInplace(T &x) noexcept {
#ifdef EASY_EVENT_BIG_ENDIAN
            UnusedParameter(x);
#else
            EndianReverseInplace(x);
#endif
        }

        template<typename T>
        static void LittleToNativeInplace(T &x) noexcept {
#ifdef EASY_EVENT_LITTLE_ENDIAN
            UnusedParameter(x);
#else
            EndianReverseInplace(x);
#endif
        }

        template<typename T>
        static void NativeToLittleInplace(T &x) noexcept {
#ifdef EASY_EVENT_LITTLE_ENDIAN
            UnusedParameter(x);
#else
            EndianReverseInplace(x);
#endif
        }
    };

}

#endif //EASYEVENT_COMMON_ENDIAN_H
