#ifndef PAPAGAIO_H
#define PAPAGAIO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration for Lua state to avoid mandatory lua.h dependency in user code.
   Users who need to access the inner VM can still include lua.h in their own .c files. */
struct lua_State;

typedef struct Papagaio Papagaio;

/**
 * Papagaio — Standalone Text Processing & Pattern Matching Engine (C-First)
 *
 * This is a C library which EMBEDS a Lua 5.1-5.4/LuaJIT VM to handle script-based
 * substitutions ($eval{} blocks). It can be used anywhere C is available.
 *
 * It ALSO functions as a native Lua module (papagaio.so) for Lua developers.
 *
 * ---------------------------------------------------------------------
 * C API
 *
 *   Papagaio *papagaio_open()           -- create context (fresh Lua state)
 *   void      papagaio_close(ctx)       -- destroy context
 *   void      papagaio_cleanup()         -- cleanup lazy VM (auto-called via atexit)
 *   struct lua_State *papagaio_L(ctx)   -- borrow inner lua_State
 *
 *   char *papagaio_process(input, ...)              -- NULL-terminated pairs
 *   char *papagaio_process_ex(input, sig, o, c, ...)
 *   char *papagaio_process_pairs(ctx, input, pats, repls, n)
 *   char *papagaio_process_text(ctx, input, len)
 *
 *   All return malloc'd strings; caller must free().
 *   Pass NULL as ctx to use the lazy internal VM for $eval{} support.
 */


/* Lifecycle */
Papagaio  *papagaio_open(void);
void       papagaio_close(Papagaio *ctx);
void       papagaio_cleanup(void);
struct lua_State *papagaio_L(Papagaio *ctx);
void       papagaio_set_args(Papagaio *ctx, int argc, char **argv);

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

/* Lua module entry point — called automatically by require "papagaio" */
#ifndef LUALIB_API
#if defined(_WIN32)
#define LUALIB_API __declspec(dllexport)
#else
#define LUALIB_API extern
#endif
#endif

/* Note: we use struct lua_State* here to keep the header C-only/Lua-free */
LUALIB_API int luaopen_papagaio(struct lua_State *L);

#ifdef __cplusplus
}
#endif
#endif /* PAPAGAIO_H */
