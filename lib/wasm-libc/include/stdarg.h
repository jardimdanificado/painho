#ifndef _STDARG_H
#define _STDARG_H

/*
 * stdarg.h para clang/wasm32.
 *
 * Clang fornece __builtin_va_* que funcionam corretamente para wasm32.
 * Não usamos a implementação manual baseada em ponteiro de stack porque
 * o WASM não garante layout de stack convencional.
 */

typedef __builtin_va_list va_list;

#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dst, src)   __builtin_va_copy(dst, src)

#endif /* _STDARG_H */
