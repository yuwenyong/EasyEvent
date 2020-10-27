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

#include <cassert>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>

#ifndef _WIN32
#   include <pthread.h>
#   include <errno.h>
#   include <unistd.h>
#endif

#define EASY_EVENT_PLATFORM_WINDOWS     0
#define EASY_EVENT_PLATFORM_LINUX       1
#define EASY_EVENT_PLATFORM_OTHER       2


// must be first (win 64 also define _WIN32)
#if defined( _WIN64 )
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_WINDOWS
#elif defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_WINDOWS
#elif defined( __linux__ )
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_LINUX
#else
#  define EASY_EVENT_PLATFORM EASY_EVENT_PLATFORM_OTHER
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

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <numeric>
#include <chrono>
#include <random>
#include <tuple>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <csignal>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#endif //EASYEVENT_COMMON_CONFIG_H
