#pragma once
#if !defined(__WINDOWS__) && !defined(__POSIX__)
#if defined(_MSC_VER)||defined(WIN32)||defined(_WIN32)
#define  __WINDOWS__ 1
#elif defined(__linux__)
#define  __POSIX__ 1
#elif defined(__APPLE__)
#define  __POSIX__ 1
#else
#error "Unrecognized OS platform"
#endif
#endif// !defined(__WINDOWS__) && !defined(__POSIX__)


#if (defined(_DEBUG)||defined(DEBUG)) && !defined(__DEBUG__) 
#define  __DEBUG__ 
#endif // debug version

typedef unsigned int uint;
typedef long long int64;

#ifdef __WINDOWS__
typedef signed __int8  int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
//typedef unsigned __int64 uint64;
#else
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
//typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
//typedef uint64_t uint64;
#endif

#ifdef __WINDOWS__
#include <Windows.h>
#endif
