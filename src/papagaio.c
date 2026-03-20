/*
 * papagaio.c — shared library, Lua 5.1 / 5.2 / 5.3 / 5.4+ / LuaJIT 2.x
 * Original logic: https://github.com/jardimdanificado/urb
 *
 * Build:
 *   cc -shared -fPIC -o papagaio.so papagaio.c $(pkg-config --cflags --libs lua5.4)
 *   cc -shared -fPIC -o papagaio.so papagaio.c $(pkg-config --cflags --libs luajit)
 */
#ifndef LUA_IMPL
#define LUA_IMPL
#include "../lib/minilua.h"
#endif

#include "papagaio.h"
#include "../lib/libregexp/libregexp.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* libregexp callbacks */
int lre_check_stack_overflow(void *opaque, size_t alloca_size) { (void)opaque; (void)alloca_size; return 0; }
int lre_check_timeout(void *opaque) { (void)opaque; return 0; }
void *lre_realloc(void *opaque, void *ptr, size_t size) {
    (void)opaque;
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, size);
}

/* =========================================================================
 * Lua version compatibility shims
 * Targets: Lua 5.1, LuaJIT 2.x (reports 5.1), Lua 5.2, 5.3, 5.4
 * ====================================================================== */

#if LUA_VERSION_NUM < 502
/*
 * luaL_newlib — Lua 5.1 only has luaL_register.
 * In 5.1 luaL_register(L, NULL, l) pushes nothing when name is NULL and the
 * table is already on the stack, so we create the table first.
 */
#undef  luaL_newlib
#define luaL_newlib(L, funcs) \
    (lua_newtable(L), luaL_register(L, NULL, (funcs)))

/*
 * luaL_requiref — not available in 5.1.
 * We only use it in papagaio_open() which is the C-embedding path; the Lua
 * module path (luaopen_papagaio) does not call it.
 */
static
#ifdef __GNUC__
__attribute__((unused))
#endif
void luaL_requiref(lua_State *L, const char *modname,
                           lua_CFunction openf, int glb)
{
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);
    lua_call(L, 1, 1);
    if (glb) {
        lua_pushvalue(L, -1);
        lua_setglobal(L, modname);
    }
}

/*
 * luaL_tolstring — not available in 5.1.
 * Mimics the 5.2+ behavior: tries __tostring metamethod, then falls back to
 * a type-based default.  Always leaves one new string on the stack.
 */
static const char *luaL_tolstring(lua_State *L, int idx, size_t *len)
{
    if (luaL_callmeta(L, idx, "__tostring")) {
        /* metamethod left result on stack — accept only strings */
        if (lua_type(L, -1) != LUA_TSTRING) {
            lua_pop(L, 1);
            lua_pushstring(L, "(invalid __tostring)");
        }
    } else {
        switch (lua_type(L, idx)) {
            case LUA_TNUMBER:
                lua_pushfstring(L, "%g", (double)lua_tonumber(L, idx));
                break;
            case LUA_TSTRING:
                lua_pushvalue(L, idx);
                break;
            case LUA_TBOOLEAN:
                lua_pushstring(L, lua_toboolean(L, idx) ? "true" : "false");
                break;
            case LUA_TNIL:
                lua_pushliteral(L, "nil");
                break;
            default:
                lua_pushfstring(L, "%s: %p",
                    luaL_typename(L, idx),
                    lua_topointer(L, idx));
                break;
        }
    }
    return lua_tolstring(L, -1, len);
}
#endif /* LUA_VERSION_NUM < 502 */

/* =========================================================================
 * Internal types (previously in papagaio_internal.h)
 * ====================================================================== */

typedef struct { const char *ptr; size_t len; } StrView;
typedef struct { char *data; size_t len; size_t cap; } StrBuf;

typedef enum {
    TOK_LITERAL, TOK_VAR, TOK_REGEX, TOK_BLOCK, TOK_WS,
    TOK_BLOCKSEQ, TOK_OPTIONS, TOK_OPTIONAL_LIT
} PapTokenType;

typedef enum {
    MOD_NONE, MOD_INT, MOD_FLOAT, MOD_NUMBER,
    MOD_UPPER, MOD_LOWER, MOD_CAPITALIZED,
    MOD_WORD, MOD_IDENTIFIER, MOD_HEX, MOD_PATH,
    MOD_BINARY, MOD_PERCENT, MOD_ALIASES,
    MOD_OPTIONAL, MOD_STARTS, MOD_ENDS
} VarModifier;

typedef struct {
    PapTokenType type;
    VarModifier modifier;
    StrView     value;
    StrView     var;
    StrView     open;
    StrView     close;
    uint8_t    *re;       /* compiled regex bytecode */
    char       *open_str;
    char       *close_str;
    unsigned    optional : 1;
    int         next_sig;
    unsigned    all_opt  : 1;
    char      **alts;
    int         alt_count;
    char       *literal_str;
} PapToken;

typedef struct {
    const char *sigil, *open, *close;
    const char *pattern, *regex, *eval, *block, *blockseq, *optional;
} Symbols;

typedef struct { PapToken *t; int count; int cap; Symbols sym; } Pattern;
typedef struct { StrView name; StrView value; char *owned; } Capture;

typedef struct {
    Capture    *cap;
    int         count;
    int         cap_size;
    int         start;
    int         end;
    const char *src;
    struct {
        const uint8_t **capture;
        int             capture_count;
        size_t          match_start;
        size_t          match_end;
        const char     *src;
    } regex;
} Match;

typedef struct { Pattern pattern; const char *replacement; } Rule;
typedef struct { char *m; char *r; } PatternPair;
typedef struct { char *code; size_t len; } EvalBlock;

/* =========================================================================
 * Embedded Papagaio Lua Script
 * ====================================================================== */

static const char *PAP_CORE_LUA = 
"local pap = {}\n"
"local function trim(s) return s:match('^%s*(.-)%s*$') end\n"
"_G.papagaio_internal = pap\n"
"_G.papagaio = pap\n"
"local ok, mod = pcall(require, 'papagaio')\n"
"if ok and type(mod) == 'table' then\n"
"  pap.process_text = mod.process_text\n"
"  pap.process = mod.process_text\n"
"  pap.process_ex = mod.process_ex\n"
"  pap.process_pairs = mod.process_pairs\n"
"end\n";

struct Papagaio { lua_State *L; int owned; };

/* Internal global for lazy-loaded Lua VM (for C library users NOT passing a context) */
static lua_State *g_lazy_L = NULL;
void papagaio_cleanup(void);

static void register_papagaio_preload(lua_State *L);
LUALIB_API int luaopen_memory(lua_State *L);

static void papagaio_stdout_clear(lua_State *L)
{
    lua_pushstring(L, "");
    lua_setglobal(L, "papagaio_stdout");
}

static void papagaio_stdout_append(lua_State *L, const char *msg, size_t len)
{
    lua_getglobal(L, "papagaio_stdout");
    size_t old_len = 0;
    const char *old = lua_tolstring(L, -1, &old_len);
    if (!old) old = "";

    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addlstring(&b, old, old_len);
    luaL_addlstring(&b, msg, len);
    luaL_pushresult(&b);
    lua_setglobal(L, "papagaio_stdout");

    lua_pop(L, 1); /* pop old value */
}

static int papagaio_print(lua_State *L)
{
    int n = lua_gettop(L);
    luaL_Buffer b;
    luaL_buffinit(L, &b);

    for (int i = 1; i <= n; i++) {
        if (i > 1) luaL_addchar(&b, '\t');
        size_t sl;
        luaL_tolstring(L, i, &sl);
        const char *s = lua_tostring(L, -1);
        if (s && sl) luaL_addlstring(&b, s, sl);
        lua_pop(L, 1);
    }
    luaL_addchar(&b, '\n');
    luaL_pushresult(&b);

    size_t out_len;
    const char *out = lua_tolstring(L, -1, &out_len);
    if (out && out_len > 0) {
        fwrite(out, 1, out_len, stdout);
        fflush(stdout);
        papagaio_stdout_append(L, out, out_len);
    }
    lua_pop(L, 1); /* pop the output string */

    return 0;
}

static lua_State *ensure_L(lua_State *L)
{
    if (L) return L;
    if (!g_lazy_L) {
        g_lazy_L = luaL_newstate();
        if (g_lazy_L) {
            luaL_openlibs(g_lazy_L);
            if (luaL_dostring(g_lazy_L, PAP_CORE_LUA) != LUA_OK) {
                fprintf(stderr, "papagaio: error loading core logic: %s\n", lua_tostring(g_lazy_L, -1));
            }
            register_papagaio_preload(g_lazy_L);
            papagaio_stdout_clear(g_lazy_L);
            lua_pushcfunction(g_lazy_L, papagaio_print);
            lua_setglobal(g_lazy_L, "print");
            lua_settop(g_lazy_L, 0);
            atexit(papagaio_cleanup);
        }
    }
    return g_lazy_L;
}

static void register_papagaio_preload(lua_State *L)
{
    if (!L) return;
    lua_getglobal(L, "package");
    if (!lua_istable(L, -1)) { lua_pop(L, 1); return; }
    lua_getfield(L, -1, "preload");
    if (!lua_istable(L, -1)) { lua_pop(L, 2); return; }
    lua_pushcfunction(L, luaopen_papagaio);
    lua_setfield(L, -2, "papagaio");
    lua_pushcfunction(L, luaopen_memory);
    lua_setfield(L, -2, "memory");
    lua_pop(L, 2);
}

/* =========================================================================
 * Constants
 * ====================================================================== */

#define PAP_SIGIL    "$"
#define PAP_OPEN     "{"
#define PAP_CLOSE    "}"
#define PAP_PATTERN  "pattern"
#define PAP_REGEX    "regex"
#define PAP_EVAL     "eval"
#define PAP_BLOCK    "block"
#define PAP_BLOCKSEQ "blockseq"
#define PAP_OPTIONAL "optional"
#define PAP_ESC      '\x01'

/* =========================================================================
 * StrBuf
 * ====================================================================== */

static void sb_init(StrBuf *b)
{
    b->cap = 256; b->len = 0;
    b->data = (char *)malloc(b->cap);
    b->data[0] = '\0';
}
static void sb_grow(StrBuf *b, size_t n)
{
    size_t need = b->len + n + 1;
    if (need <= b->cap) return;
    size_t cap = b->cap;
    while (cap < need) cap <<= 1;
    b->data = (char *)realloc(b->data, cap);
    b->cap  = cap;
}
static void sb_append_n(StrBuf *b, const char *s, size_t n)
{
    if (!n) return;
    sb_grow(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}
static void sb_append_char(StrBuf *b, char c)
{
    sb_grow(b, 1);
    b->data[b->len++] = c;
    b->data[b->len]   = '\0';
}
static void sb_free(StrBuf *b)
{
    free(b->data); b->data = NULL; b->len = 0; b->cap = 0;
}

/* =========================================================================
 * Utility
 * ====================================================================== */

static Symbols make_symbols(const char *sigil, const char *open, const char *close)
{
    Symbols s;
    s.sigil    = sigil;  s.open  = open;  s.close = close;
    s.pattern  = PAP_PATTERN; s.regex   = PAP_REGEX; s.eval    = PAP_EVAL;
    s.block    = PAP_BLOCK;   s.blockseq = PAP_BLOCKSEQ;
    s.optional = PAP_OPTIONAL;
    return s;
}

static StrView trim_sv(StrView v)
{
    size_t s = 0, e = v.len;
    while (s < v.len && isspace((unsigned char)v.ptr[s])) s++;
    while (e > s    && isspace((unsigned char)v.ptr[e-1])) e--;
    return (StrView){ v.ptr + s, e - s };
}

static int sv_eq(StrView a, StrView b)
{ return a.len == b.len && memcmp(a.ptr, b.ptr, a.len) == 0; }

static int sv_pfx(const char *s, StrView v)
{ return memcmp(s, v.ptr, v.len) == 0; }

static int str_pfx(const char *s, const char *p)
{ return memcmp(s, p, strlen(p)) == 0; }

static void skip_ws(const char *s, int *p)
{ while (isspace((unsigned char)s[*p])) (*p)++; }

/* =========================================================================
 * Pattern / Match cleanup
 * ====================================================================== */

static void free_pattern(Pattern *p)
{
    if (!p || !p->t) return;
    for (int i = 0; i < p->count; i++) {
        free(p->t[i].open_str);
        free(p->t[i].close_str);
        free(p->t[i].literal_str);
        free(p->t[i].re);
        if (p->t[i].alts) {
            for (int j = 0; j < p->t[i].alt_count; j++)
                free(p->t[i].alts[j]);
            free(p->t[i].alts);
        }
    }
    free(p->t); p->t = NULL; p->count = 0; p->cap = 0;
}

static void free_match(Match *m)
{
    if (!m) return;
    if (m->cap) {
        for (int i = 0; i < m->count; i++)
            if (m->cap[i].owned) { free(m->cap[i].owned); m->cap[i].owned = NULL; }
        free(m->cap); m->cap = NULL;
    }
    if (m->regex.capture) { free((void *)m->regex.capture); m->regex.capture = NULL; }
    m->count = 0; m->cap_size = 0;
}

static void ensure_cap(Match *m)
{
    if (m->count >= m->cap_size) {
        m->cap_size <<= 1;
        m->cap = (Capture *)realloc(m->cap, sizeof(Capture) * m->cap_size);
    }
}

static void free_pairs(PatternPair *p, int n)
{
    if (!p) return;
    for (int i = 0; i < n; i++) { free(p[i].m); free(p[i].r); }
    free(p);
}

static void free_evals(EvalBlock *e, int n)
{
    if (!e) return;
    for (int i = 0; i < n; i++) free(e[i].code);
    free(e);
}

/* =========================================================================
 * Unescape delimiter
 * ====================================================================== */

static char *unescape_delim(StrView v, size_t *out_len)
{
    StrBuf out; sb_init(&out);
    for (size_t i = 0; i < v.len; i++) {
        char c = v.ptr[i];
        if (c == '\\' && i + 1 < v.len) {
            char n = v.ptr[i+1];
            if (n == '"' || n == '\'' || n == '\\') { sb_append_char(&out, n); i++; continue; }
        }
        sb_append_char(&out, c);
    }
    if (out_len) *out_len = out.len;
    return out.data;
}

/* =========================================================================
 * Sigil escape / restore
 * ====================================================================== */

static char *pap_prepare(const char *input, const Symbols *sym)
{
    if (!input) return NULL;
    StrBuf out; sb_init(&out);
    size_t sl = strlen(sym->sigil), len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\' && sl > 0 && i + sl < len &&
            memcmp(input + i + 1, sym->sigil, sl) == 0) {
            sb_append_char(&out, PAP_ESC); i += sl; continue;
        }
        sb_append_char(&out, input[i]);
    }
    return out.data;
}

static char *pap_restore(const char *input, const Symbols *sym)
{
    if (!input) return NULL;
    StrBuf out; sb_init(&out);
    size_t sl = strlen(sym->sigil), len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        if (input[i] == PAP_ESC) { sb_append_n(&out, sym->sigil, sl); continue; }
        sb_append_char(&out, input[i]);
    }
    return out.data;
}

/* =========================================================================
 * Block extraction
 * ====================================================================== */

static int extract_block(const char *src, int pos,
                          StrView o, StrView c, StrView *out)
{
    if (o.len == c.len && o.len > 0 && memcmp(o.ptr, c.ptr, o.len) == 0) {
        if (!sv_pfx(src + pos, o)) return pos;
        pos += o.len;
        int start = pos;
        while (src[pos]) {
            if (sv_pfx(src + pos, c)) {
                out->ptr = src + start; out->len = (size_t)(pos - start);
                return pos + (int)c.len;
            }
            pos++;
        }
        out->ptr = src + start; out->len = strlen(src + start);
        return (int)strlen(src);
    }
    if (!sv_pfx(src + pos, o)) return pos;
    pos += o.len;
    int start = pos, depth = 1;
    while (src[pos] && depth) {
        if      (sv_pfx(src + pos, o)) { depth++; pos += o.len; }
        else if (sv_pfx(src + pos, c)) {
            if (!--depth) {
                out->ptr = src + start; out->len = (size_t)(pos - start);
                return pos + (int)c.len;
            }
            pos += c.len;
        } else pos++;
    }
    out->ptr = src + start; out->len = strlen(src + start);
    return (int)strlen(src);
}

/* =========================================================================
 * Lua eval
 *
 * Stack contract: enters and exits with the same depth.
 * Returns malloc'd string or NULL on error.
 * ====================================================================== */

static char *pap_eval(lua_State *L,
                       const char *code, size_t code_len,
                       const char *match, size_t match_len)
{
    L = ensure_L(L);
    if (!L) return NULL;

    /* save old `match` → stack: [old_match] */
    lua_getglobal(L, "match");

    lua_pushlstring(L, match ? match : "", match_len);
    lua_setglobal(L, "match");                          /* stack: [old_match] */

    if (luaL_loadbuffer(L, code, code_len, "papagaio") != LUA_OK) {
        /* stack: [old_match, errmsg] */
        const char *errmsg = lua_tostring(L, -1);
        fprintf(stderr, "papagaio eval load failed: %s\n", errmsg ? errmsg : "(nil)");
        if (errmsg && errmsg[0] != '\0') {
            papagaio_stdout_append(L, "[papagaio eval load failed] ", 25);
            papagaio_stdout_append(L, errmsg, strlen(errmsg));
            papagaio_stdout_append(L, "\n", 1);
        }
        lua_pop(L, 1);
        lua_setglobal(L, "match");                      /* restore → [] */
        return NULL;
    }

    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        /* stack: [old_match, errmsg] */
        const char *errmsg = lua_tostring(L, -1);
        fprintf(stderr, "papagaio eval runtime failed: %s\n", errmsg ? errmsg : "(nil)");
        if (errmsg && errmsg[0] != '\0') {
            papagaio_stdout_append(L, "[papagaio eval runtime failed] ", 30);
            papagaio_stdout_append(L, errmsg, strlen(errmsg));
            papagaio_stdout_append(L, "\n", 1);
        }
        lua_pop(L, 1);
        lua_setglobal(L, "match");
        return NULL;
    }

    /* stack: [old_match, result] */
    size_t rlen = 0;
    const char *raw = "";
    if (!lua_isnil(L, -1)) {
        luaL_tolstring(L, -1, &rlen);
        raw = lua_tostring(L, -1);
        lua_pop(L, 1); /* pop tolstring result */
    }

    char *out = (char *)malloc(rlen + 1);
    if (out) {
        if (rlen) memcpy(out, raw, rlen);
        out[rlen] = '\0';
    }

    lua_pop(L, 1);             /* pop result → [old_match] */
    lua_setglobal(L, "match"); /* restore → []                   */
    return out;
}

/* =========================================================================
 * Inline $eval{} in replacement strings
 * ====================================================================== */

static int apply_eval_ph(StrBuf *out, const char *rep, size_t rlen,
                          size_t *idx, const Symbols *sym,
                          lua_State *L, const Match *m)
{
    if (!sym->eval) return 0;
    lua_State *actual_L = ensure_L(L);
    if (!actual_L) return 0;
    size_t sl  = strlen(sym->sigil);
    size_t el  = strlen(sym->eval);
    size_t pos = *idx + sl + el;
    while (pos < rlen && isspace((unsigned char)rep[pos])) pos++;

    StrView o = { sym->open,  strlen(sym->open)  };
    StrView c = { sym->close, strlen(sym->close) };
    StrView blk;
    int next = extract_block(rep, (int)pos, o, c, &blk);
    if ((size_t)next == pos) return 0;

    StrView t = trim_sv(blk);
    char *code = (char *)malloc(t.len + 1);
    if (!code) return 0;
    memcpy(code, t.ptr, t.len); code[t.len] = '\0';

    const char *ms = ""; size_t ml = 0;
    if (m && m->src && m->end >= m->start) {
        ms = m->src + m->start; ml = (size_t)(m->end - m->start);
    }

    char *res = pap_eval(actual_L, code, t.len, ms, ml);
    free(code);
    if (!res) return 0;
    sb_append_n(out, res, strlen(res));
    free(res);
    *idx = (size_t)next;
    return 1;
}

/* =========================================================================
 * Nested pattern / eval extraction
 * ====================================================================== */

static char *extract_nested(const char *src, const Symbols *sym,
                              PatternPair **out_pairs, int *out_count)
{
    if (out_pairs) *out_pairs = NULL;
    if (out_count) *out_count = 0;
    if (!src || !sym || !sym->pattern) return NULL;

    int collect = out_pairs && out_count;
    PatternPair *pairs = NULL;
    int pc = 0, pcap = 0;

    StrBuf out; sb_init(&out);
    size_t sl = strlen(sym->sigil), pl = strlen(sym->pattern);
    StrView o = { sym->open,  strlen(sym->open)  };
    StrView c = { sym->close, strlen(sym->close) };
    size_t len = strlen(src), i = 0;

    while (i < len) {
        if (sl > 0 && i + sl + pl <= len &&
            memcmp(src + i,      sym->sigil,   sl) == 0 &&
            memcmp(src + i + sl, sym->pattern, pl) == 0) {

            size_t j = i + sl + pl;
            while (j < len && isspace((unsigned char)src[j])) j++;

            if (j < len && sv_pfx(src + j, o)) {
                StrView mp;
                int next = extract_block(src, (int)j, o, c, &mp);
                size_t k = (size_t)next;
                while (k < len && isspace((unsigned char)src[k])) k++;

                if (k < len && sv_pfx(src + k, o)) {
                    StrView rp;
                    int end = extract_block(src, (int)k, o, c, &rp);
                    StrView mt = trim_sv(mp), rt = trim_sv(rp);

                    if (collect) {
                        if (pc >= pcap) {
                            pcap = pcap ? pcap * 2 : 8;
                            pairs = (PatternPair *)realloc(pairs, sizeof(PatternPair) * pcap);
                        }
                        pairs[pc].m = (char *)malloc(mt.len + 1);
                        pairs[pc].r = (char *)malloc(rt.len + 1);
                        if (pairs[pc].m && pairs[pc].r) {
                            memcpy(pairs[pc].m, mt.ptr, mt.len); pairs[pc].m[mt.len] = '\0';
                            memcpy(pairs[pc].r, rt.ptr, rt.len); pairs[pc].r[rt.len] = '\0';
                            pc++;
                        } else { free(pairs[pc].m); free(pairs[pc].r); }
                    }
                    i = (size_t)end; continue;
                }
            }
        }
        sb_append_char(&out, src[i++]);
    }
    if (out_pairs) *out_pairs = pairs;
    if (out_count) *out_count = pc;
    return out.data;
}

static char *extract_evals(const char *src, const Symbols *sym,
                             EvalBlock **out_evals, int *out_count)
{
    if (out_evals) *out_evals = NULL;
    if (out_count) *out_count = 0;
    if (!src || !sym || !sym->eval) return NULL;

    EvalBlock *evals = NULL;
    int ec = 0, ecap = 0;

    StrBuf out; sb_init(&out);
    size_t sl = strlen(sym->sigil), el = strlen(sym->eval);
    StrView o = { sym->open,  strlen(sym->open)  };
    StrView c = { sym->close, strlen(sym->close) };
    size_t len = strlen(src), i = 0;

    while (i < len) {
        if (sl > 0 && i + sl + el <= len &&
            memcmp(src + i,      sym->sigil, sl) == 0 &&
            memcmp(src + i + sl, sym->eval,  el) == 0) {

            size_t j = i + sl + el;
            while (j < len && isspace((unsigned char)src[j])) j++;

            if (j < len && sv_pfx(src + j, o)) {
                StrView cb;
                int next = extract_block(src, (int)j, o, c, &cb);
                StrView ct = trim_sv(cb);

                if (ec >= ecap) {
                    ecap = ecap ? ecap * 2 : 8;
                    evals = (EvalBlock *)realloc(evals, sizeof(EvalBlock) * ecap);
                }
                evals[ec].code = (char *)malloc(ct.len + 1);
                evals[ec].len  = ct.len;
                if (evals[ec].code) {
                    memcpy(evals[ec].code, ct.ptr, ct.len);
                    evals[ec].code[ct.len] = '\0';
                    char ph[32];
                    snprintf(ph, sizeof(ph), "__E%d__", ec);
                    sb_append_n(&out, ph, strlen(ph));
                    ec++; i = (size_t)next; continue;
                }
            }
        }
        sb_append_char(&out, src[i++]);
    }
    if (out_evals) *out_evals = evals;
    if (out_count) *out_count = ec;
    return out.data;
}

/* =========================================================================
 * replace_all
 * ====================================================================== */

static char *replace_all(const char *src, const char *needle, const char *repl)
{
    if (!src || !needle || !*needle) return NULL;
    StrBuf out; sb_init(&out);
    size_t nl = strlen(needle), rl = repl ? strlen(repl) : 0, len = strlen(src);
    size_t i = 0;
    while (i < len) {
        if (i + nl <= len && memcmp(src + i, needle, nl) == 0) {
            if (rl) sb_append_n(&out, repl, rl);
            i += nl;
        } else sb_append_char(&out, src[i++]);
    }
    return out.data;
}

/* =========================================================================
 * Forward declarations
 * ====================================================================== */

static void parse_pattern_ex(const char *pat, Pattern *p, const Symbols *sym);
static int  match_pattern(const char *src, int src_len,
                           Pattern *p, int start, Match *m);
static char *apply_replacement_ex(const char *rep, const Match *m,
                                   const Symbols *sym, lua_State *L);

/* =========================================================================
 * apply_evals / apply_patterns
 * ====================================================================== */

static char *apply_evals(lua_State *L, const char *src,
                          EvalBlock *evals, int n, const Symbols *sym,
                          const char *match, size_t mlen)
{
    (void)sym;
    if (!src) return NULL;
    char *cur = (char *)malloc(strlen(src) + 1);
    if (!cur) return NULL;
    strcpy(cur, src);

    for (int i = n - 1; i >= 0; i--) {
        char ph[32]; snprintf(ph, sizeof(ph), "__E%d__", i);
        char *res = pap_eval(L, evals[i].code, evals[i].len, match, mlen);
        if (!res) { res = (char *)malloc(20); if (res) strcpy(res, "error: eval failed"); }
        if (res) {
            char *next = replace_all(cur, ph, res);
            free(res);
            if (next) { free(cur); cur = next; }
        }
    }
    return cur;
}

static char *apply_patterns(lua_State *L, const char *src,
                              PatternPair *pairs, int pc, const Symbols *sym)
{
    if (!src) return NULL;
    char *cur = (char *)malloc(strlen(src) + 1);
    if (!cur) return NULL;
    strcpy(cur, src);

    for (int i = 0; i < pc; i++) {
        Pattern pat; parse_pattern_ex(pairs[i].m, &pat, sym);
        StrBuf out; sb_init(&out);
        int len = (int)strlen(cur), pos = 0, matched = 0;

        while (pos < len) {
            Match m;
            if (match_pattern(cur, len, &pat, pos, &m)) {
                PatternPair *nested = NULL; int nc = 0;
                char *clean = extract_nested(pairs[i].r, sym, &nested, &nc);
                char *rep   = apply_replacement_ex(
                    clean ? clean : pairs[i].r, &m, sym, NULL);
                free(clean);

                char *nout = rep;
                if (nc > 0) {
                    char *next = apply_patterns(L, nout, nested, nc, sym);
                    if (next) { free(nout); nout = next; }
                }
                free_pairs(nested, nc);

                if (L && m.src && m.end >= m.start) {
                    lua_pushlstring(L, m.src + m.start,
                                   (size_t)(m.end - m.start));
                    lua_setglobal(L, "match");
                }

                EvalBlock *evls = NULL; int en = 0;
                char *ph = extract_evals(nout, sym, &evls, &en);
                char *applied = NULL;
                if (ph) {
                    const char *ms = m.src ? m.src + m.start : "";
                    size_t      ml = (m.src && m.end >= m.start)
                                     ? (size_t)(m.end - m.start) : 0;
                    applied = apply_evals(L, ph, evls, en, sym, ms, ml);
                    free(ph);
                }
                free_evals(evls, en);
                if (applied) { sb_append_n(&out, applied, strlen(applied)); free(applied); }
                free(nout);
                pos = m.end; free_match(&m); matched = 1; continue;
            }
            sb_append_char(&out, cur[pos++]);
        }

        free_pattern(&pat);
        if (matched) { free(cur); cur = out.data; }
        else            sb_free(&out);
    }
    return cur;
}

/* =========================================================================
 * parse_pattern_ex
 * ====================================================================== */

static void parse_pattern_ex(const char *pat, Pattern *p, const Symbols *sym)
{
    int n = (int)strlen(pat);
    p->cap = 16; p->count = 0;
    p->t   = (PapToken *)malloc(sizeof(PapToken) * p->cap);
    p->sym = *sym;

    int sl  = (int)strlen(sym->sigil);
    int ol  = (int)strlen(sym->open);
    int cl  = (int)strlen(sym->close);
    int i   = 0;

    while (i < n) {
        if (p->count == p->cap) {
            p->cap <<= 1;
            p->t = (PapToken *)realloc(p->t, sizeof(PapToken) * p->cap);
        }
        PapToken *t = &p->t[p->count];
        memset(t, 0, sizeof(*t));

        /* whitespace */
        if (isspace((unsigned char)pat[i])) {
            while (i < n && isspace((unsigned char)pat[i])) i++;
            t->type = TOK_WS; p->count++; continue;
        }

        /* sigil-led */
        if (str_pfx(pat + i, sym->sigil)) {

            /* $block{OPEN}{CLOSE}varname[?]
             *
             * Matches one or more consecutive OPEN...CLOSE blocks and
             * concatenates their contents separated by a single space.
             *
             *   $block{[}{]}items       [a][b][c]  ->  "a b c"
             *   $block{[}{]}items?      same, but optional
             *   $block{<<}{>>}body      <<foo>><<bar>>  ->  "foo bar"
             *   $block{\"}{\"} word   "hello"  ->  "hello"
             *
             * If {CLOSE} is omitted the parser reuses sym->close.
             * The keyword ("block") is taken from sym->block so it follows
             * any custom symbol set passed to process_ex / make_symbols.
             */
            {
                size_t bkw_len = strlen(sym->block);
                size_t bsq_len = strlen(sym->blockseq);
                int is_bsq = ((i + (int)sl + (int)bsq_len) <= n)
                    && memcmp(pat + i + sl, sym->blockseq, bsq_len) == 0
                    && str_pfx(pat + i + sl + bsq_len, sym->open);
                int is_blk = ((i + (int)sl + (int)bkw_len) <= n)
                    && memcmp(pat + i + sl, sym->block, bkw_len) == 0
                    && str_pfx(pat + i + sl + bkw_len, sym->open);

                if (is_blk || is_bsq) {
                    i += sl + (int)(is_blk ? bkw_len : bsq_len);

                    i += ol; /* skip open delimiter */
                    int o = i;
                    while (i < n && !str_pfx(pat + i, sym->close)) i++;
                    StrView raw_open = { pat + o, (size_t)(i - o) };
                    if (str_pfx(pat + i, sym->close)) i += cl;

                    StrView raw_close = { sym->close, strlen(sym->close) };
                    if (str_pfx(pat + i, sym->open)) {
                        i += ol; int c = i;
                        while (i < n && !str_pfx(pat + i, sym->close)) i++;
                        raw_close = (StrView){ pat + c, (size_t)(i - c) };
                        if (str_pfx(pat + i, sym->close)) i += cl;
                    }

                    StrView ot = trim_sv(raw_open); size_t olen = 0;
                    char *ou = unescape_delim(ot, &olen);
                    if (olen == 0) { free(ou); t->open = (StrView){ sym->open, strlen(sym->open) }; }
                    else { t->open_str = ou; t->open = (StrView){ t->open_str, olen }; }

                    StrView ct2 = trim_sv(raw_close); size_t clen = 0;
                    char *cu = unescape_delim(ct2, &clen);
                    if (clen == 0) { free(cu); t->close = (StrView){ sym->close, strlen(sym->close) }; }
                    else { t->close_str = cu; t->close = (StrView){ t->close_str, clen }; }

                    int v = i;
                    while (i < n && (isalnum((unsigned char)pat[i]) || pat[i] == '_')) i++;
                    t->var = (StrView){ pat + v, (size_t)(i - v) };

                    if (i < n && pat[i] == '?') { t->optional = 1; i++; }
                    t->type = is_blk ? TOK_BLOCK : TOK_BLOCKSEQ;
                    p->count++; continue;
                }
            }

            i += sl;
            int v = i;
            while (i < n && (isalnum((unsigned char)pat[i]) || pat[i] == '_')) i++;
            size_t vlen = (size_t)(i - v);

            if (vlen == 0) {
                t->type  = TOK_LITERAL;
                t->value = (StrView){ sym->sigil, (size_t)sl };
                p->count++; continue;
            }

            /* $regex VAR {pattern}
             * The keyword is part of the syntax and is not the capture name.
             */
            StrView key = { pat + v, vlen };
            if (sv_eq(key, (StrView){ sym->regex, (size_t)strlen(sym->regex) })) {
                /* Read capture variable name */
                while (i < n && isspace((unsigned char)pat[i])) i++;
                int v2 = i;
                while (i < n && (isalnum((unsigned char)pat[i]) || pat[i] == '_')) i++;
                t->var = (StrView){ pat + v2, (size_t)(i - v2) };

                /* Read regex pattern block */
                while (i < n && isspace((unsigned char)pat[i])) i++;
                if (i < n && str_pfx(pat + i, sym->open)) {
                    StrView blk;
                    StrView so = { sym->open,  (size_t)ol };
                    StrView sc = { sym->close, (size_t)cl };
                    int next = extract_block(pat, i, so, sc, &blk);
                    t->value = trim_sv(blk);
                    i = next;
                } else {
                    t->value = (StrView){ pat + i, 0 };
                }

                if (i < n && pat[i] == '?') { t->optional = 1; i++; }
                t->type = TOK_REGEX; p->count++; continue;
            }

            t->var = (StrView){ pat + v, vlen };

            /* modifier */
            if (i + sl <= n && memcmp(pat + i, sym->sigil, sl) == 0) {
                i += sl;
                int ms = i;
                while (i < n && (isalnum((unsigned char)pat[i]) || pat[i] == '_')) i++;
                StrView mod = { pat + ms, (size_t)(i - ms) };

                if      (sv_eq(mod, (StrView){"int",         3 })) t->modifier = MOD_INT;
                else if (sv_eq(mod, (StrView){"float",       5 })) t->modifier = MOD_FLOAT;
                else if (sv_eq(mod, (StrView){"number",      6 })) t->modifier = MOD_NUMBER;
                else if (sv_eq(mod, (StrView){"upper",       5 })) t->modifier = MOD_UPPER;
                else if (sv_eq(mod, (StrView){"lower",       5 })) t->modifier = MOD_LOWER;
                else if (sv_eq(mod, (StrView){"capitalized", 11})) t->modifier = MOD_CAPITALIZED;
                else if (sv_eq(mod, (StrView){"word",        4 })) t->modifier = MOD_WORD;
                else if (sv_eq(mod, (StrView){"identifier",  10})) t->modifier = MOD_IDENTIFIER;
                else if (sv_eq(mod, (StrView){"hex",         3 })) t->modifier = MOD_HEX;
                else if (sv_eq(mod, (StrView){"path",        4 })) t->modifier = MOD_PATH;
                else if (sv_eq(mod, (StrView){"binary",      6 })) t->modifier = MOD_BINARY;
                else if (sv_eq(mod, (StrView){"percent",     7 })) t->modifier = MOD_PERCENT;
                else if (sv_eq(mod, (StrView){"aliases",     7 })) {
                    t->modifier = MOD_ALIASES;
                    while (i < n && isspace((unsigned char)pat[i])) i++;
                    if (i < n && str_pfx(pat + i, sym->open)) {
                        StrView blk;
                        StrView so = { sym->open,  (size_t)ol };
                        StrView sc = { sym->close, (size_t)cl };
                        int next = extract_block(pat, i, so, sc, &blk);
                        int acap = 4;
                        t->alts = (char **)malloc(sizeof(char *) * acap);
                        t->alt_count = 0;
                        const char *cp = blk.ptr, *bend = blk.ptr + blk.len;
                        while (cp <= bend) {
                            const char *comma = cp;
                            while (comma < bend && *comma != ',') comma++;
                            StrView part = trim_sv((StrView){ cp, (size_t)(comma - cp) });
                            if (part.len > 0) {
                                if (t->alt_count >= acap) {
                                    acap <<= 1;
                                    t->alts = (char **)realloc(t->alts, sizeof(char *) * acap);
                                }
                                t->alts[t->alt_count] = (char *)malloc(part.len + 1);
                                if (t->alts[t->alt_count]) {
                                    memcpy(t->alts[t->alt_count], part.ptr, part.len);
                                    t->alts[t->alt_count][part.len] = '\0';
                                    t->alt_count++;
                                }
                            }
                            if (comma >= bend) break;
                            cp = comma + 1;
                        }
                        i = next;
                    }
                }
                else if (sv_eq(mod, (StrView){"optional", 8}) ||
                         sv_eq(mod, (StrView){"starts",   6}) ||
                         sv_eq(mod, (StrView){"ends",     4})) {
                    if      (sv_eq(mod, (StrView){"optional", 8})) t->modifier = MOD_OPTIONAL;
                    else if (sv_eq(mod, (StrView){"starts",   6})) t->modifier = MOD_STARTS;
                    else                                            t->modifier = MOD_ENDS;
                    while (i < n && isspace((unsigned char)pat[i])) i++;
                    if (i < n && str_pfx(pat + i, sym->open)) {
                        StrView blk;
                        StrView so = { sym->open,  (size_t)ol };
                        StrView sc = { sym->close, (size_t)cl };
                        int next = extract_block(pat, i, so, sc, &blk);
                        StrView phrase = trim_sv(blk);
                        t->literal_str = (char *)malloc(phrase.len + 1);
                        if (t->literal_str) {
                            memcpy(t->literal_str, phrase.ptr, phrase.len);
                            t->literal_str[phrase.len] = '\0';
                        }
                        i = next;
                    }
                }
            }

            if (i < n && pat[i] == '?') { t->optional = 1; i++; }
            t->type = TOK_VAR; p->count++; continue;
        }

        /* literal */
        int l = i;
        while (i < n && !isspace((unsigned char)pat[i]) && !str_pfx(pat + i, sym->sigil)) i++;
        t->type  = TOK_LITERAL;
        t->value = (StrView){ pat + l, (size_t)(i - l) };
        p->count++;
    }

    /* next_sig + all_opt */
    for (int a = 0; a < p->count; a++) {
        p->t[a].next_sig = -1;
        for (int b = a + 1; b < p->count; b++)
            if (p->t[b].type != TOK_WS) { p->t[a].next_sig = b; break; }
        int all = 1;
        for (int b = a + 1; b < p->count; b++) {
            if (p->t[b].type == TOK_WS) continue;
            if (!p->t[b].optional) { all = 0; break; }
        }
        p->t[a].all_opt = (unsigned)all;
    }

    /* WS adjacent to optional tokens */
    for (int a = 0; a < p->count; a++) {
        if (p->t[a].type != TOK_WS) continue;
        int ns = p->t[a].next_sig;
        if (ns >= 0 && p->t[ns].optional) { p->t[a].optional = 1; continue; }
        for (int b = a - 1; b >= 0; b--) {
            if (p->t[b].type == TOK_WS) continue;
            if (p->t[b].optional) p->t[a].optional = 1;
            break;
        }
    }
}

/* =========================================================================
 * match_pattern
 * ====================================================================== */

/* Character-validity check for typed captures */
#define CHAR_VALID(c, pos, s) ( \
    !(t->modifier == MOD_INT         && !(isdigit((unsigned char)(c)) || ((pos)==(s) && (c)=='-'))) && \
    !(t->modifier == MOD_FLOAT       && !(isdigit((unsigned char)(c)) || (c)=='.' || ((pos)==(s) && (c)=='-'))) && \
    !(t->modifier == MOD_NUMBER      && !(isdigit((unsigned char)(c)) || (c)=='.' || ((pos)==(s) && (c)=='-'))) && \
    !(t->modifier == MOD_UPPER       && !isupper((unsigned char)(c))) && \
    !(t->modifier == MOD_LOWER       && !islower((unsigned char)(c))) && \
    !(t->modifier == MOD_CAPITALIZED && (((pos)==(s)) ? !isupper((unsigned char)(c)) : !islower((unsigned char)(c)))) && \
    !(t->modifier == MOD_WORD        && !isalpha((unsigned char)(c))) && \
    !(t->modifier == MOD_IDENTIFIER  && (!(isalnum((unsigned char)(c)) || (c)=='_') || ((pos)==(s) && isdigit((unsigned char)(c))))) && \
    !(t->modifier == MOD_HEX         && (!isxdigit((unsigned char)(c)) && !((c)=='x' && (pos)>(s) && src[(pos)-1]=='0') && !((c)=='X' && (pos)>(s) && src[(pos)-1]=='0'))) && \
    !(t->modifier == MOD_PATH        && (isspace((unsigned char)(c)) || (c)=='\n')) && \
    !(t->modifier == MOD_BINARY      && ((c)!='0' && (c)!='1' && (c)!='b' && (c)!='B')) && \
    !(t->modifier == MOD_PERCENT     && !(isdigit((unsigned char)(c)) || (c)=='.' || (c)=='%' || ((pos)==(s) && (c)=='-'))))

static int match_pattern(const char *src, int src_len,
                          Pattern *p, int start, Match *m)
{
    m->cap_size = 16; m->count = 0;
    m->cap      = (Capture *)malloc(sizeof(Capture) * m->cap_size);
    m->start    = start; m->src = src;
    m->regex.capture       = NULL; m->regex.capture_count = 0;
    m->regex.match_start   = start; m->regex.match_end = start;
    m->regex.src           = src;

    int pos = start;

    for (int i = 0; i < p->count; i++) {
        PapToken *t  = &p->t[i];
        PapToken *nx = (t->next_sig >= 0) ? &p->t[t->next_sig] : NULL;

        if (t->type == TOK_WS) {
            if (!isspace((unsigned char)src[pos])) {
                if (!t->all_opt && !t->optional) goto fail;
                continue;
            }
            skip_ws(src, &pos); continue;
        }

        if (t->type == TOK_LITERAL) {
            if (!sv_pfx(src + pos, t->value)) goto fail;
            pos += (int)t->value.len; continue;
        }

        if (t->type == TOK_VAR) {
            /* skip horizontal whitespace only — never consume \n, which is a
             * line boundary and must remain in the output stream */
            if (i == 0 || p->t[i-1].type != TOK_WS) {
                while (src[pos] == ' ' || src[pos] == '\t') pos++;
            }
            int s = pos;

            if (t->modifier == MOD_ALIASES) {
                int hit = 0;
                for (int ai = 0; ai < t->alt_count; ai++) {
                    size_t al = strlen(t->alts[ai]);
                    if ((size_t)(src_len - pos) >= al &&
                        memcmp(src + pos, t->alts[ai], al) == 0) {
                        pos += (int)al; hit = 1; break;
                    }
                }
                if (!hit) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                    continue;
                }
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(pos-s) }, NULL };
                continue;
            }
            if (t->modifier == MOD_OPTIONAL) {
                if ((size_t)(src_len-pos) >= t->value.len && t->value.len > 0 &&
                    memcmp(src+pos, t->value.ptr, t->value.len) == 0)
                    pos += (int)t->value.len;
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(pos-s) }, NULL };
                continue;
            }
            if (t->modifier == MOD_STARTS) {
                if ((size_t)(src_len-pos) < t->value.len ||
                    memcmp(src+pos, t->value.ptr, t->value.len) != 0) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                    continue;
                }
            }

            if (nx && (nx->type == TOK_LITERAL || nx->type == TOK_BLOCK ||
                       nx->type == TOK_BLOCKSEQ || nx->type == TOK_OPTIONS)) {
                while (src[pos]) {
                    if (src[pos] == '\n') break;
                    if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                    if ((nx->type == TOK_BLOCK || nx->type == TOK_BLOCKSEQ) &&
                        sv_pfx(src+pos, nx->open)) break;
                    if (!CHAR_VALID(src[pos], pos, s)) break;
                    if (nx->type == TOK_OPTIONS) {
                        int fa = 0;
                        for (int ai = 0; ai < nx->alt_count; ai++) {
                            size_t al = strlen(nx->alts[ai]);
                            if ((size_t)(src_len-pos) >= al &&
                                memcmp(src+pos, nx->alts[ai], al) == 0) { fa = 1; break; }
                        }
                        if (fa) break;
                    }
                    pos++;
                    if (t->modifier == MOD_ENDS && t->value.len > 0 &&
                        (size_t)(pos-s) >= t->value.len &&
                        memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) == 0) break;
                }
                if (t->modifier == MOD_ENDS && t->value.len > 0 &&
                    ((size_t)(pos-s) < t->value.len ||
                     memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) != 0)) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; continue;
                }
                int end = pos;
                while (end > s && isspace((unsigned char)src[end-1])) end--;
                if (end == s) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; continue;
                }
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(end-s) }, NULL };
                pos = end; continue;
            }

            while (src[pos]) {
                if (nx && isspace((unsigned char)src[pos])) break;
                if (nx) {
                    if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                    if ((nx->type == TOK_BLOCK || nx->type == TOK_BLOCKSEQ) &&
                        sv_pfx(src+pos, nx->open)) break;
                } else if (src[pos] == '\n') break; /* sem next: para só em newline */
                if (!CHAR_VALID(src[pos], pos, s)) break;
                pos++;
                if (t->modifier == MOD_ENDS && t->value.len > 0 &&
                    (size_t)(pos-s) >= t->value.len &&
                    memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) == 0) break;
            }
            if (t->modifier == MOD_ENDS && t->value.len > 0 &&
                ((size_t)(pos-s) < t->value.len ||
                 memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) != 0)) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; continue;
            }
            if (pos == s) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; continue;
            }
            ensure_cap(m);
            m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(pos-s) }, NULL };
            continue;
        }

        if (t->type == TOK_REGEX) {
            if (!t->re && t->value.len > 0) {
                int l = 0;
                char err[128];
                char *pat = (char *)malloc(t->value.len + 1);
                if (pat) {
                    memcpy(pat, t->value.ptr, t->value.len);
                    pat[t->value.len] = '\0';
                    t->re = lre_compile(&l, err, sizeof(err), pat, t->value.len, 0, NULL);
                    (void)err;
                    free(pat);
                }
            }
            if (!t->re) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                continue;
            }

            int cap_count = lre_get_capture_count(t->re);
            int cap_slots = cap_count * 2;
            uint8_t **capture = (uint8_t **)malloc(sizeof(uint8_t *) * cap_slots);
            if (!capture) goto fail;
            for (int ci = 0; ci < cap_slots; ci++) capture[ci] = NULL;

            int rc = lre_exec(capture, t->re, (const uint8_t *)src, pos, src_len, 0, NULL);
            if (rc != 1) {
                free(capture);
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                continue;
            }

            size_t match_start = capture[0] ? (size_t)(capture[0] - (const uint8_t *)src) : pos;
            size_t match_end   = capture[1] ? (size_t)(capture[1] - (const uint8_t *)src) : pos;
            if (match_end < match_start) match_end = match_start;

            /* keep last regex capture info for potential introspection */
            if (m->regex.capture) { free((void *)m->regex.capture); m->regex.capture = NULL; }
            m->regex.capture = (const uint8_t **)capture;
            m->regex.capture_count = cap_count;
            m->regex.match_start = match_start;
            m->regex.match_end = match_end;
            m->regex.src = src;

            ensure_cap(m);
            m->cap[m->count++] = (Capture){ t->var, { src + match_start, (size_t)(match_end - match_start) }, NULL };
            pos = (int)match_end;

            continue;
        }

        if (t->type == TOK_BLOCK) {
            if (!sv_pfx(src+pos, t->open)) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; continue;
            }
            StrView v;
            pos = extract_block(src, pos, t->open, t->close, &v);
            ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, v, NULL }; continue;
        }

        if (t->type == TOK_OPTIONS) {
            int hit = 0;
            for (int ai = 0; ai < t->alt_count; ai++) {
                size_t al = strlen(t->alts[ai]);
                if ((size_t)(src_len-pos) >= al &&
                    memcmp(src+pos, t->alts[ai], al) == 0) {
                    pos += (int)al; hit = 1; break;
                }
            }
            if (!hit && !t->optional) goto fail;
            continue;
        }

        if (t->type == TOK_OPTIONAL_LIT) {
            if ((size_t)(src_len-pos) >= t->value.len && t->value.len > 0 &&
                memcmp(src+pos, t->value.ptr, t->value.len) == 0)
                pos += (int)t->value.len;
            continue;
        }

        if (t->type == TOK_BLOCKSEQ) {
            if (!sv_pfx(src+pos, t->open)) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; continue;
            }
            StrBuf buf; sb_init(&buf);
            int blocks = 0;
            while (sv_pfx(src+pos, t->open)) {
                StrView v;
                pos = extract_block(src, pos, t->open, t->close, &v);
                if (blocks > 0) sb_append_char(&buf, ' ');
                sb_append_n(&buf, v.ptr, v.len);
                blocks++; skip_ws(src, &pos);
            }
            if (blocks == 0) {
                if (!t->optional) goto fail;
                sb_free(&buf); ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; continue;
            }
            ensure_cap(m);
            m->cap[m->count++] = (Capture){ t->var, { buf.data, buf.len }, buf.data };
        }
    }

    m->end = pos; return 1;

fail:
    free(m->cap); m->cap = NULL;
    if (m->regex.capture) { free((void *)m->regex.capture); m->regex.capture = NULL; }
    return 0;
}

#undef CHAR_VALID

/* =========================================================================
 * apply_replacement_ex
 * ====================================================================== */

static char *apply_replacement_ex(const char *rep, const Match *m,
                                   const Symbols *sym, lua_State *L)
{
    StrBuf out; sb_init(&out);
    size_t n = strlen(rep), i = 0, sl = strlen(sym->sigil);

    while (i < n) {
        if (str_pfx(rep + i, sym->sigil)) {
            if (sym->eval && apply_eval_ph(&out, rep, n, &i, sym, L, m)) continue;

            size_t ns = i + sl, ne = ns;
            while (ne < n && (isalnum((unsigned char)rep[ne]) || rep[ne] == '_')) ne++;

            StrView name = { rep + ns, ne - ns };
            int found = 0;
            if (name.len > 0) {
                for (int k = 0; k < m->count; k++) {
                    if (sv_eq(m->cap[k].name, name)) {
                        sb_append_n(&out, m->cap[k].value.ptr, m->cap[k].value.len);
                        found = 1; break;
                    }
                }
            }
            if (!found) { sb_append_n(&out, sym->sigil, sl); sb_append_n(&out, name.ptr, name.len); }
            i = (ne == ns) ? i + sl : ne;
            continue;
        }
        sb_append_char(&out, rep[i++]);
    }
    return out.data;
}

/* =========================================================================
 * Internal process loop
 * ====================================================================== */

static char *pap_process_impl(lua_State *L, const char *input,
                               const char *sigil, const char *open,
                               const char *close, va_list ap)
{
    Symbols sym = make_symbols(sigil, open, close);
    int rc = 0, rcap = 8;
    Rule *rules = (Rule *)malloc(sizeof(Rule) * rcap);

    while (1) {
        const char *pat = va_arg(ap, const char *); if (!pat) break;
        const char *rep = va_arg(ap, const char *);
        if (rc >= rcap) { rcap <<= 1; rules = (Rule *)realloc(rules, sizeof(Rule) * rcap); }
        parse_pattern_ex(pat, &rules[rc].pattern, &sym);
        rules[rc].replacement = rep; rc++;
    }

    StrBuf out; sb_init(&out);
    int len = (int)strlen(input), pos = 0;

    while (pos < len) {
        int matched = 0;
        for (int i = 0; i < rc; i++) {
            Match m;
            if (match_pattern(input, len, &rules[i].pattern, pos, &m)) {
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym, L);
                sb_append_n(&out, r, strlen(r)); free(r);
                pos = m.end; free_match(&m); matched = 1; break;
            }
        }
        if (!matched) sb_append_char(&out, input[pos++]);
    }

    for (int i = 0; i < rc; i++) free_pattern(&rules[i].pattern);
    free(rules);

    char *result = (char *)malloc(out.len + 1);
    memcpy(result, out.data, out.len + 1);
    sb_free(&out);
    return result;
}

/* =========================================================================
 * Public C API
 * ====================================================================== */

Papagaio *papagaio_open(void)
{
    Papagaio *ctx = (Papagaio *)malloc(sizeof(Papagaio));
    if (!ctx) return NULL;
    ctx->L     = luaL_newstate();
    ctx->owned = 1;
    if (!ctx->L) { free(ctx); return NULL; }
    luaL_openlibs(ctx->L);
    if (luaL_dostring(ctx->L, PAP_CORE_LUA) != LUA_OK) {
        fprintf(stderr, "papagaio: error loading core logic: %s\n", lua_tostring(ctx->L, -1));
    }

    register_papagaio_preload(ctx->L);
    papagaio_stdout_clear(ctx->L);
    lua_pushcfunction(ctx->L, papagaio_print);
    lua_setglobal(ctx->L, "print");
    lua_settop(ctx->L, 0);
    return ctx;
}


void papagaio_close(Papagaio *ctx)
{
    if (!ctx) return;
    if (ctx->owned && ctx->L) lua_close(ctx->L);
    free(ctx);
}

void papagaio_cleanup(void)
{
    if (g_lazy_L) {
        lua_close(g_lazy_L);
        g_lazy_L = NULL;
    }
}

void papagaio_set_args(Papagaio *ctx, int argc, char **argv)
{
    lua_State *L = ctx ? ctx->L : ensure_L(NULL);
    if (!L) return;

    lua_settop(L, 0);
    lua_newtable(L);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "arg");
    lua_settop(L, 0);
}

lua_State *papagaio_L(Papagaio *ctx) { return ctx ? ctx->L : NULL; }

char *papagaio_process(const char *input, ...)
{
    va_list args; va_start(args, input);
    char *r = pap_process_impl(NULL, input, PAP_SIGIL, PAP_OPEN, PAP_CLOSE, args);
    va_end(args); return r;
}

char *papagaio_process_ex(const char *input, const char *sigil,
                          const char *open, const char *close, ...)
{
    va_list args; va_start(args, close);
    char *r = pap_process_impl(NULL, input, sigil, open, close, args);
    va_end(args); return r;
}

char *papagaio_process_pairs(Papagaio *ctx, const char *input,
                             const char **patterns, const char **repls,
                             int pair_count)
{
    lua_State *L = ctx ? ctx->L : NULL;
    Symbols sym  = make_symbols(PAP_SIGIL, PAP_OPEN, PAP_CLOSE);
    Rule *rules  = (Rule *)malloc(sizeof(Rule) * (pair_count ? pair_count : 1));
    if (!rules) return NULL;

    for (int i = 0; i < pair_count; i++) {
        parse_pattern_ex(patterns[i], &rules[i].pattern, &sym);
        rules[i].replacement = repls[i];
    }

    StrBuf out; sb_init(&out);
    int len = (int)strlen(input), pos = 0;

    while (pos < len) {
        int matched = 0;
        for (int i = 0; i < pair_count; i++) {
            Match m;
            if (match_pattern(input, len, &rules[i].pattern, pos, &m)) {
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym, L);
                sb_append_n(&out, r, strlen(r)); free(r);
                pos = m.end; free_match(&m); matched = 1; break;
            }
        }
        if (!matched) sb_append_char(&out, input[pos++]);
    }

    for (int i = 0; i < pair_count; i++) free_pattern(&rules[i].pattern);
    free(rules);

    char *result = (char *)malloc(out.len + 1);
    if (!result) { sb_free(&out); return NULL; }
    memcpy(result, out.data, out.len + 1);
    sb_free(&out); return result;
}

char *papagaio_process_text(Papagaio *ctx, const char *input, size_t len)
{
    if (!input) return NULL;
    lua_State *L = ctx ? ctx->L : NULL;
    L = ensure_L(L);
    if (L) papagaio_stdout_clear(L);
    Symbols sym  = make_symbols(PAP_SIGIL, PAP_OPEN, PAP_CLOSE);
    char *buf = (char *)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, input, len); buf[len] = '\0';

    char *prepared = pap_prepare(buf, &sym); free(buf);
    if (!prepared) return NULL;

    /* A. Resolve Patterns First */
    PatternPair *pairs = NULL; int pc = 0;
    char *text_no_patterns = extract_nested(prepared, &sym, &pairs, &pc);
    free(prepared);
    if (!text_no_patterns) { free_pairs(pairs, pc); return NULL; }

    char *cur = text_no_patterns;
    if (pc > 0) {
        char *last = NULL;
        while (1) {
            if (last && strcmp(cur, last) == 0) break;
            free(last);
            last = (char *)malloc(strlen(cur) + 1);
            if (!last) break;
            strcpy(last, cur);

            char *next = apply_patterns(L, cur, pairs, pc, &sym);
            if (next && next != cur) { free(cur); cur = next; }

            PatternPair *nested = NULL; int nc = 0;
            char *check = extract_nested(cur, &sym, &nested, &nc);
            free(check); free_pairs(nested, nc);
            if (nc == 0) break;
        }
        free(last);
    }
    free_pairs(pairs, pc);

    /* B. If we have evals, enter AST mode */
    EvalBlock *evals = NULL; int ec = 0;
    char *ph = extract_evals(cur, &sym, &evals, &ec);

    if (ec > 0) {
        L = ensure_L(L);

        /* Restore placeholders as text */
        char *tmp_restored = pap_restore(ph, &sym);

        /* 2. Run evals once */
        char **results = (char **)malloc(sizeof(char *) * ec);
        for (int i = 0; i < ec; i++) {
            results[i] = pap_eval(L, evals[i].code, evals[i].len, "", 0);
        }

        /* 3. Substitute placeholders */
        char *final = tmp_restored;
        for (int i = ec - 1; i >= 0; i--) {
            char eph[32]; snprintf(eph, sizeof(eph), "__E%d__", i);
            char *res = results[i];
            if (!res) {
                const char *emsg = "error";
                res = (char *)malloc(strlen(emsg) + 1);
                if (res) strcpy(res, emsg);
            }
            char *next = replace_all(final, eph, res);
            free(res);
            if (next) { free(final); final = next; }
        }
        free(results);
        free(cur); free(ph); free_evals(evals, ec); cur = final;
    } else {
        free(ph); free_evals(evals, ec);
    }

    char *restored = pap_restore(cur, &sym);
    free(cur);

    if (L && restored) {
        lua_pushlstring(L, restored, strlen(restored));
        lua_setglobal(L, "papagaio_content");

        lua_getglobal(L, "papagaio_stdout");
        size_t stdout_len = 0;
        const char *stdout_data = lua_tolstring(L, -1, &stdout_len);
        if (stdout_data && stdout_len > 0) {
            size_t restored_len = strlen(restored);
            char *combined = (char *)malloc(stdout_len + restored_len + 1);
            if (combined) {
                memcpy(combined, stdout_data, stdout_len);
                memcpy(combined + stdout_len, restored, restored_len + 1);
                free(restored);
                restored = combined;
            }
        }
        lua_pop(L, 1);
        papagaio_stdout_clear(L);
    }

    return restored;
}

/* =========================================================================
 * Lua module
 * ====================================================================== */

/* pap.process(input, pat, repl [, pat, repl, ...]) */
static int l_process(lua_State *L)
{
    const char *input = luaL_checkstring(L, 1);
    int nargs = lua_gettop(L) - 1;
    if (nargs % 2 != 0)
        return luaL_error(L, "process: expected pairs of pattern/replacement");

    Symbols sym    = make_symbols(PAP_SIGIL, PAP_OPEN, PAP_CLOSE);
    int     pc     = nargs / 2;
    Rule   *rules  = (Rule *)malloc(sizeof(Rule) * (pc ? pc : 1));

    for (int i = 0; i < pc; i++) {
        parse_pattern_ex(luaL_checkstring(L, 2 + i*2),
                         &rules[i].pattern, &sym);
        rules[i].replacement = luaL_checkstring(L, 3 + i*2);
    }

    StrBuf out; sb_init(&out);
    int len = (int)strlen(input), pos = 0;
    while (pos < len) {
        int matched = 0;
        for (int i = 0; i < pc; i++) {
            Match m;
            if (match_pattern(input, len, &rules[i].pattern, pos, &m)) {
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym, L);
                sb_append_n(&out, r, strlen(r)); free(r);
                pos = m.end; free_match(&m); matched = 1; break;
            }
        }
        if (!matched) sb_append_char(&out, input[pos++]);
    }
    for (int i = 0; i < pc; i++) free_pattern(&rules[i].pattern);
    free(rules);
    lua_pushlstring(L, out.data, out.len);
    sb_free(&out);
    return 1;
}

/* pap.process_ex(input, sigil, open, close, pat, repl [...]) */
static int l_process_ex(lua_State *L)
{
    const char *input = luaL_checkstring(L, 1);
    const char *sigil = luaL_checkstring(L, 2);
    const char *open  = luaL_checkstring(L, 3);
    const char *close = luaL_checkstring(L, 4);
    int nargs = lua_gettop(L) - 4;
    if (nargs % 2 != 0)
        return luaL_error(L, "process_ex: expected pairs of pattern/replacement");

    Symbols sym   = make_symbols(sigil, open, close);
    int     pc    = nargs / 2;
    Rule   *rules = (Rule *)malloc(sizeof(Rule) * (pc ? pc : 1));

    for (int i = 0; i < pc; i++) {
        parse_pattern_ex(luaL_checkstring(L, 5 + i*2), &rules[i].pattern, &sym);
        rules[i].replacement = luaL_checkstring(L, 6 + i*2);
    }

    StrBuf out; sb_init(&out);
    int len = (int)strlen(input), pos = 0;
    while (pos < len) {
        int matched = 0;
        for (int i = 0; i < pc; i++) {
            Match m;
            if (match_pattern(input, len, &rules[i].pattern, pos, &m)) {
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym, L);
                sb_append_n(&out, r, strlen(r)); free(r);
                pos = m.end; free_match(&m); matched = 1; break;
            }
        }
        if (!matched) sb_append_char(&out, input[pos++]);
    }
    for (int i = 0; i < pc; i++) free_pattern(&rules[i].pattern);
    free(rules);
    lua_pushlstring(L, out.data, out.len);
    sb_free(&out);
    return 1;
}

/* pap.process_pairs(input, { ["$pat"]="repl", ... } | { {"$pat","repl"}, ... }) */
static int l_process_pairs(lua_State *L)
{
    const char *input = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    int    cap  = 8, count = 0;
    char **pats  = (char **)malloc(sizeof(char *) * cap);
    char **repls = (char **)malloc(sizeof(char *) * cap);

    lua_pushnil(L);
    while (lua_next(L, 2)) {
        const char *pat = NULL, *rep = NULL;
        if (lua_type(L, -1) == LUA_TTABLE) {
            lua_rawgeti(L, -1, 1); lua_rawgeti(L, -2, 2);
            pat = lua_tostring(L, -2); rep = lua_tostring(L, -1);
            lua_pop(L, 2);
        } else {
            pat = lua_tostring(L, -2); rep = lua_tostring(L, -1);
        }
        if (pat && rep) {
            if (count >= cap) {
                cap <<= 1;
                pats  = (char **)realloc(pats,  sizeof(char *) * cap);
                repls = (char **)realloc(repls, sizeof(char *) * cap);
            }
            pats[count] = (char *)pat; repls[count] = (char *)rep; count++;
        }
        lua_pop(L, 1);
    }

    /* borrow the calling state so $eval{} sees the caller's globals */
    Papagaio borrow; borrow.L = L; borrow.owned = 0;
    char *result = papagaio_process_pairs(&borrow, input,
                                          (const char **)pats,
                                          (const char **)repls, count);
    free(pats); free(repls);
    if (!result) return luaL_error(L, "process_pairs: internal error");
    lua_pushstring(L, result); free(result);
    return 1;
}

/* pap.process_text(input) */
static int l_process_text(lua_State *L)
{
    size_t len; const char *input = luaL_checklstring(L, 1, &len);
    Papagaio borrow; borrow.L = L; borrow.owned = 0;
    char *result = papagaio_process_text(&borrow, input, len);
    if (!result) return luaL_error(L, "process_text: internal error");
    lua_pushstring(L, result); free(result);
    return 1;
}

static const luaL_Reg papagaio_funcs[] = {
    { "process",       l_process       },
    { "process_ex",    l_process_ex    },
    { "process_pairs", l_process_pairs },
    { "process_text",  l_process_text  },
    { NULL, NULL }
};

LUALIB_API int luaopen_papagaio(lua_State *L)
{
    lua_getglobal(L, "papagaio_internal");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_dostring(L, PAP_CORE_LUA);
    } else {
        lua_pop(L, 1);
    }

    /* Make memory module available via require("memory") */
    lua_getglobal(L, "package");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "preload");
        if (lua_istable(L, -1)) {
            lua_pushcfunction(L, luaopen_memory);
            lua_setfield(L, -2, "memory");
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    luaL_newlib(L, papagaio_funcs);
    return 1;
}

/* --------------------------- memory module --------------------------- */

static uintptr_t mem_check_ptr(lua_State *L, int idx)
{
    if (lua_islightuserdata(L, idx)) return (uintptr_t)lua_touserdata(L, idx);
#if LUA_VERSION_NUM >= 502
    if (lua_isinteger(L, idx)) return (uintptr_t)lua_tointeger(L, idx);
    if (lua_isnumber(L, idx)) return (uintptr_t)lua_tonumber(L, idx);
#else
    if (lua_isnumber(L, idx)) return (uintptr_t)lua_tonumber(L, idx);
#endif
    luaL_argerror(L, idx, "expected pointer (lightuserdata or number)");
    return 0;
}

static int l_memory_alloc(lua_State *L)
{
    lua_Integer size = luaL_checkinteger(L, 1);
    if (size <= 0) return luaL_error(L, "alloc: invalid size");
    void *p = malloc((size_t)size);
    if (!p) return luaL_error(L, "alloc: out of memory");
    lua_pushinteger(L, (lua_Integer)(uintptr_t)p);
    return 1;
}

static int l_memory_free(lua_State *L)
{
    if (!lua_isnil(L, 1)) {
        void *p = (void *)(uintptr_t)mem_check_ptr(L, 1);
        free(p);
    }
    return 0;
}

static int l_memory_realloc(lua_State *L)
{
    void *p = (void *)(uintptr_t)mem_check_ptr(L, 1);
    lua_Integer size = luaL_checkinteger(L, 2);
    if (size <= 0) {
        free(p);
        lua_pushnil(L);
        return 1;
    }
    void *n = realloc(p, (size_t)size);
    if (!n) return luaL_error(L, "realloc: out of memory");
    lua_pushinteger(L, (lua_Integer)(uintptr_t)n);
    return 1;
}

static int l_memory_zero(lua_State *L)
{
    void *p = (void *)(uintptr_t)mem_check_ptr(L, 1);
    lua_Integer size = luaL_checkinteger(L, 2);
    if (size < 0) return luaL_error(L, "zero: invalid size");
    memset(p, 0, (size_t)size);
    return 0;
}

static int l_memory_copy(lua_State *L)
{
    void *dst = (void *)(uintptr_t)mem_check_ptr(L, 1);
    void *src = (void *)(uintptr_t)mem_check_ptr(L, 2);
    lua_Integer size = luaL_checkinteger(L, 3);
    if (size < 0) return luaL_error(L, "copy: invalid size");
    memcpy(dst, src, (size_t)size);
    lua_pushinteger(L, (lua_Integer)(uintptr_t)dst);
    return 1;
}

static int l_memory_set(lua_State *L)
{
    void *p = (void *)(uintptr_t)mem_check_ptr(L, 1);
    int byte = (int)luaL_checkinteger(L, 2);
    lua_Integer size = luaL_checkinteger(L, 3);
    if (byte < 0 || byte > 0xFF) return luaL_error(L, "set: byte value out of range");
    if (size < 0) return luaL_error(L, "set: invalid size");
    memset(p, (unsigned char)byte, (size_t)size);
    return 0;
}

static int l_memory_compare(lua_State *L)
{
    void *a = (void *)(uintptr_t)mem_check_ptr(L, 1);
    void *b = (void *)(uintptr_t)mem_check_ptr(L, 2);
    lua_Integer size = luaL_checkinteger(L, 3);
    if (size < 0) return luaL_error(L, "compare: invalid size");
    int r = memcmp(a, b, (size_t)size);
    lua_pushinteger(L, r);
    return 1;
}

static int l_memory_string(lua_State *L)
{
    void *p = (void *)(uintptr_t)mem_check_ptr(L, 1);
    if (!p) { lua_pushnil(L); return 1; }
    lua_pushstring(L, (const char *)p);
    return 1;
}

static int l_memory_readi8(lua_State *L)
{
    int8_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writei8(lua_State *L)
{
    int8_t v = (int8_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readu8(lua_State *L)
{
    uint8_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writeu8(lua_State *L)
{
    uint8_t v = (uint8_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readi16(lua_State *L)
{
    int16_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writei16(lua_State *L)
{
    int16_t v = (int16_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readu16(lua_State *L)
{
    uint16_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writeu16(lua_State *L)
{
    uint16_t v = (uint16_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readi32(lua_State *L)
{
    int32_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writei32(lua_State *L)
{
    int32_t v = (int32_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readu32(lua_State *L)
{
    uint32_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writeu32(lua_State *L)
{
    uint32_t v = (uint32_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readi64(lua_State *L)
{
    int64_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writei64(lua_State *L)
{
    int64_t v = (int64_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readu64(lua_State *L)
{
    uint64_t v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushinteger(L, (lua_Integer)v);
    return 1;
}

static int l_memory_writeu64(lua_State *L)
{
    uint64_t v = (uint64_t)luaL_checkinteger(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readf32(lua_State *L)
{
    float v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushnumber(L, (lua_Number)v);
    return 1;
}

static int l_memory_writef32(lua_State *L)
{
    float v = (float)luaL_checknumber(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static int l_memory_readf64(lua_State *L)
{
    double v;
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy(&v, (void *)p, sizeof(v));
    lua_pushnumber(L, (lua_Number)v);
    return 1;
}

static int l_memory_writef64(lua_State *L)
{
    double v = (double)luaL_checknumber(L, 2);
    uintptr_t p = mem_check_ptr(L, 1);
    memcpy((void *)p, &v, sizeof(v));
    return 0;
}

static const luaL_Reg memory_funcs[] = {
    { "alloc",      l_memory_alloc },
    { "free",       l_memory_free },
    { "realloc",    l_memory_realloc },
    { "zero",       l_memory_zero },
    { "copy",       l_memory_copy },
    { "set",        l_memory_set },
    { "compare",    l_memory_compare },
    { "string",     l_memory_string },
    { "readi8",     l_memory_readi8 },
    { "writei8",    l_memory_writei8 },
    { "readu8",     l_memory_readu8 },
    { "writeu8",    l_memory_writeu8 },
    { "readi16",    l_memory_readi16 },
    { "writei16",   l_memory_writei16 },
    { "readu16",    l_memory_readu16 },
    { "writeu16",   l_memory_writeu16 },
    { "readi32",    l_memory_readi32 },
    { "writei32",   l_memory_writei32 },
    { "readu32",    l_memory_readu32 },
    { "writeu32",   l_memory_writeu32 },
    { "readi64",    l_memory_readi64 },
    { "writei64",   l_memory_writei64 },
    { "readu64",    l_memory_readu64 },
    { "writeu64",   l_memory_writeu64 },
    { "readf32",    l_memory_readf32 },
    { "writef32",   l_memory_writef32 },
    { "readf64",    l_memory_readf64 },
    { "writef64",   l_memory_writef64 },
    { NULL, NULL }
};

LUALIB_API int luaopen_memory(lua_State *L)
{
    luaL_newlib(L, memory_funcs);
    return 1;
}
