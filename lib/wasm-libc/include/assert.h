#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdio.h>
#include <stdlib.h>

/*
 * assert(expr): se expr for falso, imprime diagnóstico e chama abort().
 *
 * Define NDEBUG antes de incluir este header para desativar todas as
 * asserções em builds de produção — zero overhead.
 */

#ifdef NDEBUG
#  define assert(expr) ((void)0)
#else
#  define assert(expr) \
     ((expr) \
      ? (void)0 \
      : (__assert_fail(#expr, __FILE__, __LINE__, __func__), (void)0))
#endif

/* função interna — não use diretamente */
static inline void __assert_fail(const char *expr, const char *file,
                                  int line, const char *func) {
    fprintf_stderr("%s:%d: %s: Assertion '%s' failed.\n",
                   file, line, func, expr);
    abort();
}

/* static_assert em tempo de compilação (C11) */
#ifndef static_assert
#  define static_assert(expr, msg) _Static_assert(expr, msg)
#endif

#endif /* _ASSERT_H */
