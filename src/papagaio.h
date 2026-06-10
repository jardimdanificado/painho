#ifndef PAPAGAIO_H
#define PAPAGAIO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Papagaio Papagaio;

/**
 * Papagaio — Modular Plugin-Based Text Processing & Pattern Matching Engine (C-First)
 *
 * This is a C library which uses a plugin architecture to support various
 * scripting languages (Lua, QuickJS, etc) and custom commands via $import{}.
 *
 * ---------------------------------------------------------------------
 * C API
 *
 *   Papagaio *papagaio_open()           -- create context
 *   void      papagaio_close(ctx)       -- destroy context and unload plugins
 *
 *   char *papagaio_process(input, ...)              -- NULL-terminated pairs
 *   char *papagaio_process_ex(input, sig, o, c, ...)
 *   char *papagaio_process_pairs(ctx, input, pats, repls, n)
 *   char *papagaio_process_text(ctx, input, len)
 *
 *   All return malloc'd strings; caller must free().
 */


/* Lifecycle */
Papagaio  *papagaio_open(void);
void       papagaio_close(Papagaio *ctx);
void       papagaio_set_args(Papagaio *ctx, int argc, char **argv);
void       papagaio_get_args(Papagaio *ctx, int *argc, char ***argv);

/* Extensibility Definitions */
typedef char *(*PapCommandHandler)(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *userdata);
typedef char *(*PapModifierHandler)(const char *match, const char *modifier, size_t match_len, size_t mod_len, void *userdata);
typedef void (*PapFinalizer)(void *userdata);

int   papagaio_has_command(Papagaio *ctx, const char *name);
int   papagaio_register_command(Papagaio *ctx, const char *name, PapCommandHandler handler, void *ud);
int   papagaio_register_modifier(Papagaio *ctx, const char *name, PapModifierHandler handler, void *ud);

/* Math Extensibility */
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

/* C API Execution Functions */
char *papagaio_process(const char *input, ...);
char *papagaio_process_ex(const char *input,
                          const char *sigil,
                          const char *open,
                          const char *close, ...);
char *papagaio_process_pairs(Papagaio   *ctx,
                             const char *input,
                             const char **patterns,
                             const char **repls,
                             int         pair_count);
char *papagaio_process_text(Papagaio   *ctx,
                            const char *input,
                            size_t      len);

#ifdef __cplusplus
}
#endif
#endif /* PAPAGAIO_H */
