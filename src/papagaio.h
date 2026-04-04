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
