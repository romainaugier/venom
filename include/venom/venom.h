/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#pragma once

#if !defined(__VENOM)
#define __VENOM

#if defined(_MSC_VER)
#define VENOM_MSVC
#pragma warning(disable:4711) /* function selected for automatic inline expansion */
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#elif defined(__GNUC__)
#define VENOM_GCC
#elif defined(__clang__)
#define VENOM_CLANG
#endif /* defined(_MSC_VER) */

#define VENOM_STRIFY(x) #x
#define VENOM_STRIFY_MACRO(m) VENOM_STRIFY(m)

#if !defined(VENOM_VERSION_MAJOR)
#define VENOM_VERSION_MAJOR 0
#endif /* !defined(VENOM_VERSION_MAJOR) */

#if !defined(VENOM_VERSION_MINOR)
#define VENOM_VERSION_MINOR 0
#endif /* !defined(VENOM_VERSION_MINOR) */

#if !defined(VENOM_VERSION_PATCH)
#define VENOM_VERSION_PATCH 0
#endif /* !defined(VENOM_VERSION_PATCH) */

#if !defined(VENOM_VERSION_REVISION)
#define VENOM_VERSION_REVISION 0
#endif /* !defined(VENOM_VERSION_REVISION) */

#define VENOM_VERSION_STR VENOM_STRIFY_MACRO(VENOM_VERSION_MAJOR)"." \
                              VENOM_STRIFY_MACRO(VENOM_VERSION_MINOR)"." \
                              VENOM_STRIFY_MACRO(VENOM_VERSION_PATCH)"." \
                              VENOM_STRIFY_MACRO(VENOM_VERSION_REVISION)

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#if INTPTR_MAX == INT64_MAX || defined(__x86_64__)
#define VENOM_X64
#define VENOM_SIZEOF_PTR 8
#elif INTPTR_MAX == INT32_MAX
#define VENOM_X86
#define VENOM_SIZEOF_PTR 4
#endif /* INTPTR_MAX == INT64_MAX || defined(__x86_64__) */

#if defined(_WIN32)
#define VENOM_WIN
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif /* !defined(WIN32_LEAN_AND_MEAN) */
#if defined(VENOM_X64)
#define VENOM_PLATFORM_STR "WIN64"
#else
#define VENOM_PLATFORM_STR "WIN32"
#endif /* defined(VENOM_x64) */
#elif defined(__linux__)
#define VENOM_LINUX
#if defined(VENOM_X64)
#define VENOM_PLATFORM_STR "LINUX64"
#else
#define VENOM_PLATFORM_STR "LINUX32"
#endif /* defined(VENOM_X64) */
#endif /* defined(_WIN32) */

#if defined(VENOM_WIN)
#if defined(VENOM_MSVC)
#define VENOM_EXPORT __declspec(dllexport)
#define VENOM_IMPORT __declspec(dllimport)
#elif defined(VENOM_GCC) || defined(VENOM_CLANG)
#define VENOM_EXPORT __attribute__((dllexport))
#define VENOM_IMPORT __attribute__((dllimport))
#endif /* defined(VENOM_MSVC) */
#elif defined(VENOM_LINUX)
#define VENOM_EXPORT __attribute__((visibility("default")))
#define VENOM_IMPORT
#endif /* defined(VENOM_WIN) */

#if defined(VENOM_MSVC)
#define VENOM_FORCE_INLINE __forceinline
#define VENOM_LIB_ENTRY
#define VENOM_LIB_EXIT
#elif defined(VENOM_GCC)
#define VENOM_FORCE_INLINE inline __attribute__((always_inline)) 
#define VENOM_LIB_ENTRY __attribute__((constructor))
#define VENOM_LIB_EXIT __attribute__((destructor))
#elif defined(VENOM_CLANG)
#define VENOM_FORCE_INLINE __attribute__((always_inline))
#define VENOM_LIB_ENTRY __attribute__((constructor))
#define VENOM_LIB_EXIT __attribute__((destructor))
#endif /* defined(VENOM_MSVC) */

#if defined(VENOM_BUILD_SHARED)
#define VENOM_API VENOM_EXPORT
#else
#define VENOM_API VENOM_IMPORT
#endif /* defined(VENOM_BUILD_SHARED) */

#if defined(VENOM_WIN)
#define VENOM_FUNCTION __FUNCTION__
#elif defined(VENOM_GCC) || defined(VENOM_CLANG)
#define VENOM_FUNCTION __PRETTY_FUNCTION__
#endif /* VENOM_WIN */

#define CONCAT_(prefix, suffix)     prefix##suffix
#define CONCAT(prefix, suffix)      CONCAT_(prefix, suffix)

#define VENOM_ASSERT(expr, message) if(!(expr)) { fprintf(stderr, "Assertion failed in file %s at line %d: %s", __FILE__, __LINE__, message); abort(); }

#define VENOM_STATIC_ASSERT(expr)        \
    struct CONCAT(__outscope_assert_, __COUNTER__)      \
    {                                                   \
        char                                            \
        outscope_assert                                 \
        [2*(expr)-1];                                   \
                                                        \
    } CONCAT(__outscope_assert_, __COUNTER__)

#define VENOM_NOT_IMPLEMENTED fprintf(stderr, "Function " VENOM_FUNCTION " not implemented"); exit(1);

#if defined(VENOM_MSVC)
#define VENOM_PACKED_STRUCT(__struct__) __pragma(pack(push, 1)) __struct__ __pragma(pack(pop))
#elif defined(VENOM_GCC) || defined(VENOM_CLANG)
#define VENOM_PACKED_STRUCT(__struct__) __struct__ __attribute__((__packed__))
#else
#define VENOM_PACKED_STRUCT(__struct__) __struct__
#endif /* defined(VENOM_MSVC) */

#if defined(VENOM_MSVC)
#define dump_struct(s) 
#elif defined(VENOM_CLANG)
#define dump_struct(s) __builtin_dump_struct(s, printf)
#elif defined(VENOM_GCC)
#define dump_struct(s) 
#endif /* defined(VENOM_MSVC) */

#if defined(DEBUG_BUILD)
#define VENOM_DEBUG 1
#else
#define VENOM_DEBUG 0
#endif /* defined(DEBUG_BUILD) */

#if defined(__cplusplus)
#define VENOM_CPP_ENTER extern "C" {
#define VENOM_CPP_END }
#else
#define VENOM_CPP_ENTER
#define VENOM_CPP_END
#endif /* defined(__cplusplus) */

#define VENOM_ATEXIT_REGISTER(__func__, __exit__)                                  \
        int res_##__func__ = atexit(__func__);                                     \
        if(res_##__func__ != 0)                                                    \
        {                                                                          \
            fprintf(stderr, "Cannot register function \""#__func__"\" in atexit"); \
            if(__exit__) exit(1);                                                  \
        }                                                                           

#endif /* !defined(__VENOM) */
