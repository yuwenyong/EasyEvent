//
// Created by yuwenyong on 2020/10/26.
//

#ifndef EASYEVENT_COMMON_CONFIG_H
#define EASYEVENT_COMMON_CONFIG_H

#include <stdlib.h>

#if defined(__GLIBC__)
#   include <endian.h>
#elif defined(__APPLE__)
#   include <machine/endian.h>
#elif defined(__FreeBSD__)
#   include <sys/endian.h>
#endif

#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)) || \
    (defined(_BYTE_ORDER) && defined(_LITTLE_ENDIAN) && (_BYTE_ORDER == _LITTLE_ENDIAN)) || \
    (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))

#   define EASY_EVENT_LITTLE_ENDIAN

#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)) || \
      (defined(_BYTE_ORDER) && defined(_BIG_ENDIAN) && (_BYTE_ORDER == _BIG_ENDIAN)) || \
      (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))

#   define EASY_EVENT_BIG_ENDIAN

#elif defined(__i386)      || \
      defined(_M_IX86)     || \
      defined(__x86_64)    || \
      defined(_M_X64)      || \
      defined(_M_IA64)     || \
      defined(__alpha__)   || \
      defined(__ARMEL__)   || \
      defined(_M_ARM_FP)   || \
      defined(__arm64)     || \
      defined(__MIPSEL__)

#   define EASY_EVENT_LITTLE_ENDIAN

#elif defined(__sparc)   || \
      defined(__sparc__) || \
      defined(__hppa)    || \
      defined(__ppc__)   || \
      defined(__powerpc) || \
      defined(_ARCH_COM) || \
      defined(__MIPSEB__)

#   define EASY_EVENT_BIG_ENDIAN

#else

#   error "Unknown architecture"

#endif

#ifdef _MSC_VER

#   ifdef _WIN64
#      define EASY_EVENT_64
#   else
#      define EASY_EVENT_32
#   endif

#else
#   include <stdint.h>

#   if defined(__WORDSIZE) && (__WORDSIZE == 64)
#      define EASY_EVENT_64
#   elif defined(__WORDSIZE) && (__WORDSIZE == 32)
#      define EASY_EVENT_32
#   elif defined(__sun) && (defined(__sparcv9) || defined(__x86_64))  || \
         defined(__linux__) && defined(__x86_64)                      || \
         defined(__APPLE__) && defined(__x86_64)                      || \
         defined(__hppa) && defined(__LP64__)                         || \
         defined(_ARCH_COM) && defined(__64BIT__)                     || \
         defined(__alpha__)                                           || \
         defined(_WIN64)
#      define EASY_EVENT_64
#   else
#      define EASY_EVENT_32
#   endif
#endif

#if defined(_WIN32) && defined(_MSC_VER)
typedef __int64 Int64;
#   define INT64(n) n##i64
#   define INT64_FORMAT "%lld"

typedef unsigned __int64 UInt64;
#   define UINT64(n) n##ui64
#   define UINT64_FORMAT "%llu"
#elif defined(EASY_EVENT_64) && !defined(_WIN32)
typedef long Int64;
#   define INT64(n) n##L
#   define INT64_FORMAT "%ld"

typedef unsigned long UInt64;
#   define UINT64(n) n##UL
#   define UINT64_FORMAT "%lu"
#else
typedef long long Int64;
#   define INT64(n) n##LL
#   define INT64_FORMAT "%lld"

typedef unsigned long long UInt64;
#   define UINT64(n) n##ULL
#   define UINT64_FORMAT "%llu"
#endif


#ifndef NDEBUG
#   ifndef _DEBUG
#       define _DEBUG
#   endif
#   ifndef DEBUG
#       define DEBUG
#   endif
#   define EASY_EVENT_DEBUG
#endif

#define Assert(expr)        assert(expr)

#ifdef EASY_EVENT_DEBUG
#   define Verify(expr)     assert(expr)
#else
#   define Verify(expr)     (expr)
#endif

#define UnusedParameter(p)  (void)(p)


#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <csignal>
#include <cstddef>


#ifdef _WIN32
#   define _WIN32_WINNT 0x0601
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#   include <sys/timeb.h>
#   include <sys/types.h>
#else
#   include <pthread.h>
#   include <unistd.h>
#   include <sys/time.h>
#endif

#define EASY_EVENT_PLATFORM_WINDOWS     0
#define EASY_EVENT_PLATFORM_LINUX       1
#define EASY_EVENT_PLATFORM_APPLE       2
#define EASY_EVENT_PLATFORM_UNIX        3


#if defined(_WIN64) || defined(__WIN32__) || defined(WIN32) || defined(_WIN32)
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_WINDOWS
#elif defined(__linux__)
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_LINUX
#elif defined(__MACH__) && defined(__APPLE__)
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_APPLE
#elif defined(unix) || defined(__unix__) || defined(__unix)
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_UNIX
#else
#  error "Unknown platform"
#endif

#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_LINUX || EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_APPLE
#   define EASY_EVENT_HAS_SSIZE_T 1
#endif

#ifndef EASY_EVENT_HAS_SSIZE_T
#   if defined(EASY_EVENT_64)
typedef Int64 ssize_t;
#   else
typedef int ssize_t;
#   endif
#endif

#include <cstddef>
#include <cinttypes>
#include <climits>

#ifdef EASY_EVENT_USE_DYNAMIC_LINKING
#  if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
#    define EASY_EVENT_EXPORT __declspec(dllexport)
#    define EASY_EVENT_IMPORT __declspec(dllimport)
#  else
#    define EASY_EVENT_EXPORT __attribute__((visibility("default")))
#    define EASY_EVENT_IMPORT
#  endif
#else
#  define EASY_EVENT_EXPORT
#  define EASY_EVENT_IMPORT
#endif

#ifdef EASY_EVENT_API_EXPORT
#  define EASY_EVENT_API EASY_EVENT_EXPORT
#else
#  define EASY_EVENT_API EASY_EVENT_IMPORT
#endif

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#include <exception>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <utility>
#include <type_traits>
#include <numeric>
#include <algorithm>
#include <random>
#include <memory>
#include <vector>
#include <array>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <stack>
#include <tuple>
#include <optional>
#include <variant>
#include <string_view>
#include <string>
#include <regex>
#include <system_error>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>
#include <filesystem>

#ifdef _MSC_VER
#   include <float.h>
#   define snprintf _snprintf
#   define atoll _atoi64
#   define vsnprintf _vsnprintf
#   define llabs _abs64
#else
#   define stricmp strcasecmp
#   define strnicmp strncasecmp
#endif

inline unsigned long atoul(char const* str) {
    return strtoul(str, nullptr, 10);
}

inline unsigned long long atoull(char const* str) {
    return strtoull(str, nullptr, 10);
}

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

using ByteArray = std::vector<std::byte>;

using StringPair = std::pair<std::string, std::string>;
using StringVec = std::vector<std::string>;
using StringList = std::list<std::string>;
using StringMap = std::map<std::string, std::string>;
using StringSet = std::set<std::string>;
using StringVecMap = std::map<std::string, StringVec>;

using StringViewVec = std::vector<std::string_view>;
using StringViewList = std::list<std::string_view>;
using StringViewSet = std::set<std::string_view>;


#endif //EASYEVENT_COMMON_CONFIG_H
