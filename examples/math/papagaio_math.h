#ifndef PAPAGAIO_MATH_H
#define PAPAGAIO_MATH_H

#include "../../src/papagaio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Math Extensibility via Plugin */
void papagaio_clear_math(Papagaio *ctx);
int papagaio_register_math_generic(Papagaio *ctx, const char *name, void *func, int arity, int is_closure, int is_pure, ...);
int papagaio_register_math_infix(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...);
int papagaio_register_math_infix_right(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...);
int papagaio_register_math_prefix(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...);
int papagaio_register_math_postfix(Papagaio *ctx, const char *name, void *func, int precedence, int is_closure, ...);
int papagaio_register_math_ternary(Papagaio *ctx, const char *name, const char *sep2, void *func, int precedence, int is_closure, ...);
int papagaio_register_math_quaternary(Papagaio *ctx, const char *name, const char *sep2, const char *sep3, void *func, int precedence, int is_closure, ...);

#define _PAP_ARITY(func) _Generic((func), \
    double (*)(void): 0, \
    double (*)(double): 1, \
    double (*)(double, double): 2, \
    double (*)(double, double, double): 3, \
    double (*)(double, double, double, double): 4, \
    double (*)(double, double, double, double, double): 5, \
    double (*)(double, double, double, double, double, double): 6, \
    double (*)(double, double, double, double, double, double, double): 7, \
    double (*)(double, double, double, double, double, double, double, double): 8, \
    double (*)(double, double, double, double, double, double, double, double, double): 9, \
    double (*)(double, double, double, double, double, double, double, double, double, double): 10, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double): 11, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double): 12, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double): 13, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double, double): 14, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 15, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 16, \
    double (*)(void*): 0, \
    double (*)(void*, double): 1, \
    double (*)(void*, double, double): 2, \
    double (*)(void*, double, double, double): 3, \
    double (*)(void*, double, double, double, double): 4, \
    double (*)(void*, double, double, double, double, double): 5, \
    double (*)(void*, double, double, double, double, double, double): 6, \
    double (*)(void*, double, double, double, double, double, double, double): 7, \
    double (*)(void*, double, double, double, double, double, double, double, double): 8, \
    double (*)(void*, double, double, double, double, double, double, double, double, double): 9, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double): 10, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double): 11, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double): 12, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double): 13, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 14, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 15, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 16 \
)

#define _PAP_IS_CLOSURE(func) _Generic((func), \
    double (*)(void): 0, \
    double (*)(double): 0, \
    double (*)(double, double): 0, \
    double (*)(double, double, double): 0, \
    double (*)(double, double, double, double): 0, \
    double (*)(double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 0, \
    double (*)(double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 0, \
    double (*)(void*): 1, \
    double (*)(void*, double): 1, \
    double (*)(void*, double, double): 1, \
    double (*)(void*, double, double, double): 1, \
    double (*)(void*, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 1, \
    double (*)(void*, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double): 1 \
)


#define louro_register(ctx, name, func, ...) \
    papagaio_register_math_generic(ctx, name, (void*)(func), _PAP_ARITY(func), _PAP_IS_CLOSURE(func), 1, ##__VA_ARGS__)

#define louro_register_impure(ctx, name, func, ...) \
    papagaio_register_math_generic(ctx, name, (void*)(func), _PAP_ARITY(func), _PAP_IS_CLOSURE(func), 0, ##__VA_ARGS__)

#define louro_register_infix(ctx, name, func, prec, ...) \
    papagaio_register_math_infix(ctx, name, (void*)(func), prec, _PAP_IS_CLOSURE(func), ##__VA_ARGS__)

#define louro_register_infix_right(ctx, name, func, prec, ...) \
    papagaio_register_math_infix_right(ctx, name, (void*)(func), prec, _PAP_IS_CLOSURE(func), ##__VA_ARGS__)

#define louro_register_prefix(ctx, name, func, prec, ...) \
    papagaio_register_math_prefix(ctx, name, (void*)(func), prec, _PAP_IS_CLOSURE(func), ##__VA_ARGS__)

#define louro_register_postfix(ctx, name, func, prec, ...) \
    papagaio_register_math_postfix(ctx, name, (void*)(func), prec, _PAP_IS_CLOSURE(func), ##__VA_ARGS__)

#define louro_register_ternary(ctx, name, sep2, func, prec, ...) \
    papagaio_register_math_ternary(ctx, name, sep2, (void*)(func), prec, _PAP_IS_CLOSURE(func), ##__VA_ARGS__)

#define louro_register_quaternary(ctx, name, sep2, sep3, func, prec, ...) \
    papagaio_register_math_quaternary(ctx, name, sep2, sep3, (void*)(func), prec, _PAP_IS_CLOSURE(func), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // PAPAGAIO_MATH_H
