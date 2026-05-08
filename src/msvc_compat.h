#ifndef PAPAGAIO_MSVC_COMPAT_H
#define PAPAGAIO_MSVC_COMPAT_H

#ifdef _MSC_VER

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <intrin.h>
#include <stdint.h>
#include <stdlib.h>

/* GCC attributes compatibility */
#ifndef __attribute__
#define __attribute__(x) 
#endif

#ifndef __maybe_unused
#define __maybe_unused
#endif

#ifndef likely
#define likely(x)       (x)
#endif

#ifndef unlikely
#define unlikely(x)     (x)
#endif

#ifndef force_inline
#define force_inline __forceinline
#endif

#ifndef no_inline
#define no_inline __declspec(noinline)
#endif

/* QuickJS specific macros that use attributes */
#ifndef js_likely
#define js_likely(x) (x)
#endif

#ifndef js_unlikely
#define js_unlikely(x) (x)
#endif

#ifndef js_force_inline
#define js_force_inline __forceinline
#endif

#define __js_printf_like(f, a)

/* clz/ctz shims using unique names to avoid conflicts with QuickJS cutils.h fallbacks */
static inline int papagaio_msvc_clz32(unsigned int a) {
    unsigned long index;
    if (_BitScanReverse(&index, a)) return 31 - index;
    return 32;
}

static inline int papagaio_msvc_clz64(uint64_t a) {
    unsigned long index;
#if defined(_M_X64) || defined(_M_ARM64)
    if (_BitScanReverse64(&index, a)) return 63 - index;
#else
    if (_BitScanReverse(&index, (unsigned long)(a >> 32))) return 31 - index;
    if (_BitScanReverse(&index, (unsigned long)a)) return 63 - index;
#endif
    return 64;
}

static inline int papagaio_msvc_ctz32(unsigned int a) {
    unsigned long index;
    if (_BitScanForward(&index, a)) return index;
    return 32;
}

static inline int papagaio_msvc_ctz64(uint64_t a) {
    unsigned long index;
#if defined(_M_X64) || defined(_M_ARM64)
    if (_BitScanForward64(&index, a)) return index;
#else
    if (_BitScanForward(&index, (unsigned long)a)) return index;
    if (_BitScanForward(&index, (unsigned long)(a >> 32))) return index + 32;
#endif
    return 64;
}

/* Provide __builtin_ shims for submodules that expect them (like wasm3).
   We do NOT define clz32/clz64/ctz32/ctz64 as macros here to allow 
   QuickJS's cutils.h to define its own portable fallbacks without conflict. */
#define __builtin_clz papagaio_msvc_clz32
#define __builtin_clzll papagaio_msvc_clz64
#define __builtin_ctz papagaio_msvc_ctz32
#define __builtin_ctzll papagaio_msvc_ctz64
#define __builtin_expect(x, y) (x)

/* Redefine strdup if needed (MSVC uses _strdup) */
#define strdup _strdup

/* Redefine snprintf if needed */
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif

#endif /* _MSC_VER */

#endif /* PAPAGAIO_MSVC_COMPAT_H */
