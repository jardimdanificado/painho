#ifndef PAPAGAIO_MSVC_COMPAT_H
#define PAPAGAIO_MSVC_COMPAT_H

#ifdef _MSC_VER

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <intrin.h>
#include <stdint.h>
#include <stdlib.h>

/* GCC attributes compatibility */
#define __attribute__(x) 
#define __maybe_unused
#define likely(x)       (x)
#define unlikely(x)     (x)
#define force_inline __forceinline
#define no_inline __declspec(noinline)

/* QuickJS specific macros that use attributes */
#define js_likely(x) (x)
#define js_unlikely(x) (x)
#define js_force_inline __forceinline
#define __js_printf_like(f, a)

/* clz/ctz shims */
static inline int clz32(unsigned int a) {
    unsigned long index;
    if (_BitScanReverse(&index, a)) return 31 - index;
    return 32;
}

static inline int clz64(uint64_t a) {
    unsigned long index;
#if defined(_M_X64) || defined(_M_ARM64)
    if (_BitScanReverse64(&index, a)) return 63 - index;
#else
    if (_BitScanReverse(&index, (unsigned long)(a >> 32))) return 31 - index;
    if (_BitScanReverse(&index, (unsigned long)a)) return 63 - index;
#endif
    return 64;
}

static inline int ctz32(unsigned int a) {
    unsigned long index;
    if (_BitScanForward(&index, a)) return index;
    return 32;
}

static inline int ctz64(uint64_t a) {
    unsigned long index;
#if defined(_M_X64) || defined(_M_ARM64)
    if (_BitScanForward64(&index, a)) return index;
#else
    if (_BitScanForward(&index, (unsigned long)a)) return index;
    if (_BitScanForward(&index, (unsigned long)(a >> 32))) return index + 32;
#endif
    return 64;
}

/* Redefine strdup if needed (MSVC uses _strdup) */
#define strdup _strdup

/* Redefine snprintf if needed */
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif

#endif /* _MSC_VER */

#endif /* PAPAGAIO_MSVC_COMPAT_H */
