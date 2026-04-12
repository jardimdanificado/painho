#define _DEFAULT_SOURCE
#include "papagaio.h"
#include "../lib/wasm3/wasm3.h"
#include "../lib/wasm3/m3_env.h"
#include "../lib/wasm3/m3_function.h"
#include <stdbool.h>
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
 * Internal types (previously in papagaio_internal.h)
 * ====================================================================== */

typedef struct { const char *ptr; size_t len; } StrView;
typedef struct { char *data; size_t len; size_t cap; } StrBuf;

typedef enum {
    TOK_LITERAL, TOK_VAR, TOK_REGEX, TOK_BLOCK, TOK_WS,
    TOK_OPTIONS_OBSOLETE, TOK_OPTIONAL_LIT
} PapTokenType;

typedef enum {
    MOD_NONE, MOD_INT, MOD_FLOAT, MOD_NUMBER,
    MOD_UPPER, MOD_LOWER, MOD_CAPITALIZED,
    MOD_WORD, MOD_IDENTIFIER, MOD_HEX, MOD_PATH,
    MOD_BINARY, MOD_PERCENT, MOD_ALIASES,
    MOD_GROUP, MOD_STARTS, MOD_ENDS,
    MOD_PREFIX, MOD_SUFFIX, MOD_INFIX,
    MOD_INCLUDES
} VarModifier;

/* Forward declaration for recursive sub-patterns */
typedef struct Pattern_s Pattern;

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
    Pattern    *sub_pattern;      /* for optional/starts/ends/etc: recursive sub-pattern */
    Pattern   **alt_patterns;     /* for aliases: one sub-pattern per alternative */
    int         alt_pattern_count;
} PapToken;

typedef struct {
    char sigil[16];
    char open[16];
    char close[16];
    char optional[16]; 
    const char *pattern, *regex, *block, *changequote;
} Symbols;

struct Pattern_s { PapToken *t; int count; int cap; Symbols sym; };
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

/* =========================================================================
 * Embedded Papagaio Lua Script
 * ====================================================================== */

#define PAP_MAX_PLUGINS 64

typedef struct {
    PapFinalizer fn;
    void        *userdata;
} RegisteredFinalizer;

typedef struct {
    char        *name;
    PapCommandHandler handler;
    void        *userdata;
    int          is_wasm;
    IM3Function  wasm_func;
} RegisteredCommand;

typedef struct {
    char *name;
    PapModifierHandler handler;
    void *userdata;
} RegisteredModifier;

/* Forward declarations */
static char *wasm_command_bridge(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud);
static char *wasm_file_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud);
static char *wasm_bytes_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud);
static char *wasm_plugin_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud);

struct Papagaio {
    IM3Environment      env;
    IM3Runtime          runtime;

    RegisteredCommand  *commands;
    int                 cmd_count, cmd_cap;

    RegisteredModifier *modifiers;
    int                 mod_count, mod_cap;

    RegisteredFinalizer *finalizers;
    int                  fin_count, fin_cap;

    /* host arguments */
    int    argc;
    char **argv;
};

/* =========================================================================
 * Constants
 * ====================================================================== */

#define PAP_SIGIL    "$"
#define PAP_OPEN     "{"
#define PAP_CLOSE    "}"
#define PAP_PATTERN  "pattern"
#define PAP_REGEX    "regex"
#define PAP_BLOCK    "block"
#define PAP_OPTIONAL "optional"
#define PAP_CHANGEQUOTE "changequote"
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
    memset(&s, 0, sizeof(s));
    if (sigil) { strncpy(s.sigil, sigil, 15); s.sigil[15] = '\0'; }
    if (open)  { strncpy(s.open,  open,  15); s.open[15]  = '\0'; }
    if (close) { strncpy(s.close, close, 15); s.close[15] = '\0'; }
    strcpy(s.optional, "?");
    s.pattern  = PAP_PATTERN; s.regex   = PAP_REGEX;
    s.block    = PAP_BLOCK;
    s.changequote = PAP_CHANGEQUOTE;
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
{
    if (v.len == 0) return 0;
    for (size_t i = 0; i < v.len; i++) {
        if (s[i] == '\0' || s[i] != v.ptr[i]) return 0;
    }
    return 1;
}

static int str_pfx(const char *s, const char *p)
{
    if (!p || *p == '\0') return 0;
    while (*p) {
        if (*s == '\0' || *s != *p) return 0;
        s++; p++;
    }
    return 1;
}

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
        if (p->t[i].sub_pattern) {
            free_pattern(p->t[i].sub_pattern);
            free(p->t[i].sub_pattern);
        }
        if (p->t[i].alt_patterns) {
            for (int j = 0; j < p->t[i].alt_pattern_count; j++) {
                if (p->t[i].alt_patterns[j]) {
                    free_pattern(p->t[i].alt_patterns[j]);
                    free(p->t[i].alt_patterns[j]);
                }
            }
            free(p->t[i].alt_patterns);
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



/* =========================================================================
 * Unescape delimiter
 * ====================================================================== */

static int extract_block(const char *src, int pos,
                          StrView o, StrView c, StrView *out);

static char *unescape_delim(StrView v, size_t *out_len)
{
    StrBuf out; sb_init(&out);
    for (size_t i = 0; i < v.len; i++) {
        char c = v.ptr[i];
        if (c == '\\' && i + 1 < v.len) {
            char n = v.ptr[i+1];
            if (n == '{' || n == '}' || n == '[' || n == ']' || n == '(' || n == ')' || n == '\\') {
                sb_append_char(&out, n); i++; continue;
            }
        }
        sb_append_char(&out, c);
    }
    if (out_len) *out_len = out.len;
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


/* =========================================================================
 * replace_all
 * ====================================================================== */


/* =========================================================================
 * Forward declarations
 * ====================================================================== */

static void parse_pattern_ex(const char *pat, Pattern *p, const Symbols *sym);
static int  match_pattern(const char *src, int src_len,
                           Pattern *p, int start, Match *m);
static char *apply_replacement_ex(const char *rep, const Match *m,
                                   const Symbols *sym);
static char *resolve_preprocessor(Papagaio *ctx, const char *src, Symbols *sym);
static char *dispatch_commands(Papagaio *ctx, const char *src, const Symbols *sym);

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
            i += sl;
            int v = i;
            while (i < n && (isalnum((unsigned char)pat[i]) || pat[i] == '_')) i++;
            size_t vlen = (size_t)(i - v);

            if (vlen == 0) {
                t->type  = TOK_LITERAL;
                t->value = (StrView){ sym->sigil, (size_t)sl };
                p->count++; continue;
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
                else if (sv_eq(mod, (StrView){ sym->regex, strlen(sym->regex) })) {
                    t->type = TOK_REGEX;
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
                }
                else if (sv_eq(mod, (StrView){ sym->block, strlen(sym->block) })) {
                    t->type = TOK_BLOCK;

                    while (i < n && isspace((unsigned char)pat[i])) i++;
                    if (i < n && str_pfx(pat + i, sym->open)) {
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
                    }
                }
                else if (sv_eq(mod, (StrView){"aliases",     7 })) {
                    t->modifier = MOD_ALIASES;
                    int acap = 4;
                    t->alts = (char **)malloc(sizeof(char *) * acap);
                    t->alt_count = 0;
                    t->alt_patterns = (Pattern **)malloc(sizeof(Pattern *) * acap);
                    t->alt_pattern_count = 0;

                    StrView so = { sym->open,  strlen(sym->open) };
                    StrView sc = { sym->close, strlen(sym->close) };

                    while (i < n) {
                        while (i < n && isspace((unsigned char)pat[i])) i++;
                        if (i < n && sv_pfx(pat + i, so)) {
                            StrView blk;
                            i = (size_t)extract_block(pat, (int)i, so, sc, &blk);
                            if (t->alt_count >= acap) {
                                acap *= 2;
                                t->alts = (char **)realloc(t->alts, sizeof(char *) * acap);
                                t->alt_patterns = (Pattern **)realloc(t->alt_patterns, sizeof(Pattern *) * acap);
                            }
                            char *valt = (char*)malloc(blk.len + 1);
                            memcpy(valt, blk.ptr, blk.len); valt[blk.len] = '\0';
                            t->alts[t->alt_count++] = valt;
                            
                            Pattern *sp = (Pattern *)malloc(sizeof(Pattern));
                            memset(sp, 0, sizeof(Pattern));
                            parse_pattern_ex(valt, sp, sym);
                            t->alt_patterns[t->alt_pattern_count++] = sp;
                        } else break;
                    }
                }
                else if (sv_eq(mod, (StrView){"group",        5 }) ||
                         sv_eq(mod, (StrView){"optional",     8 }) ||
                         sv_eq(mod, (StrView){"starts",       6 }) ||
                         sv_eq(mod, (StrView){"ends",         4 }) ||
                         sv_eq(mod, (StrView){"prefix",       6 }) ||
                         sv_eq(mod, (StrView){"suffix",       6 }) ||
                         sv_eq(mod, (StrView){"infix",        5 }) ||
                         sv_eq(mod, (StrView){"includes",     8 })) {
                    if (sv_eq(mod, (StrView){"group", 5}) || 
                        sv_eq(mod, (StrView){"optional", 8})) {
                        t->modifier = MOD_GROUP;
                    } else if (sv_eq(mod, (StrView){"starts", 6})) {
                        t->modifier = MOD_STARTS;
                    } else if (sv_eq(mod, (StrView){"ends", 4})) {
                        t->modifier = MOD_ENDS;
                    } else if (sv_eq(mod, (StrView){"prefix", 6})) {
                        t->modifier = MOD_PREFIX;
                    } else if (sv_eq(mod, (StrView){"suffix", 6})) {
                        t->modifier = MOD_SUFFIX;
                    } else if (sv_eq(mod, (StrView){"infix", 5})) {
                        t->modifier = MOD_INFIX;
                    } else {
                        t->modifier = MOD_INCLUDES;
                    }

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
                            t->value.ptr = t->literal_str;
                            t->value.len = phrase.len;
                        }
                        /* Parse as recursive sub-pattern if it contains sigils */
                        int has_sigil = 0;
                        for (size_t si = 0; si + (size_t)sl <= phrase.len; si++) {
                            if (memcmp(phrase.ptr + si, sym->sigil, sl) == 0) { has_sigil = 1; break; }
                        }
                        if (has_sigil) {
                            char *sub_str = (char *)malloc(phrase.len + 1);
                            memcpy(sub_str, phrase.ptr, phrase.len);
                            sub_str[phrase.len] = '\0';
                            t->sub_pattern = (Pattern *)malloc(sizeof(Pattern));
                            memset(t->sub_pattern, 0, sizeof(Pattern));
                            parse_pattern_ex(sub_str, t->sub_pattern, sym);
                            free(sub_str);
                        }
                        i = next;
                    }
                }
            }

            if (i < n && str_pfx(pat + i, sym->optional)) { t->optional = 1; i += strlen(sym->optional); }
            if (t->type != TOK_REGEX && t->type != TOK_BLOCK) {
                t->type = TOK_VAR;
            }
            p->count++; continue;
        }

        /* literal */
        int l = i;
        while (i < n && !isspace((unsigned char)pat[i]) && !str_pfx(pat + i, sym->sigil) && !str_pfx(pat + i, sym->optional)) i++;
        t->type  = TOK_LITERAL;
        t->value = (StrView){ pat + l, (size_t)(i - l) };
        p->count++;
        if (i < n && str_pfx(pat + i, sym->optional)) { p->t[p->count-1].optional = 1; i += strlen(sym->optional); }
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

/* Helper: check if a sub-pattern matches at position `at` within src[0..src_len).
   Returns the end position of the match, or -1 if no match. */
static int sub_pattern_matches_at(const char *src, int src_len, Pattern *sp, int at)
{
    Match sub_m; memset(&sub_m, 0, sizeof(sub_m));
    memset(&sub_m, 0, sizeof(sub_m));
    if (match_pattern(src, src_len, sp, at, &sub_m)) {
        int end = sub_m.end;
        free_match(&sub_m);
        return end;
    }
    return -1;
}


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
            if (!sv_pfx(src + pos, t->value)) {
                if (t->optional) continue;
                goto fail;
            }
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
                /* Fast path: try flat string match first */
                for (int ai = 0; ai < t->alt_count; ai++) {
                    size_t al = strlen(t->alts[ai]);
                    if ((size_t)(src_len - pos) >= al &&
                        memcmp(src + pos, t->alts[ai], al) == 0) {
                        ensure_cap(m);
                        m->cap[m->count++] = (Capture){ t->var, { src + pos, al }, NULL };
                        pos += (int)al; hit = 1; break;
                    }
                }
                /* Recursive path: try alternatives with sub-patterns */
                if (!hit) {
                    for (int ai = 0; ai < t->alt_pattern_count; ai++) {
                        if (!t->alt_patterns[ai]) continue;
                        Match sub_m; memset(&sub_m, 0, sizeof(sub_m));
                        memset(&sub_m, 0, sizeof(sub_m));
                        if (match_pattern(src, src_len, t->alt_patterns[ai], pos, &sub_m)) {
                            /* Merge sub-captures into parent match */
                            for (int ci = 0; ci < sub_m.count; ci++) {
                                ensure_cap(m);
                                m->cap[m->count++] = sub_m.cap[ci];
                            }
                            /* The variable itself */
                            ensure_cap(m);
                            m->cap[m->count++] = (Capture){ t->var, { src + pos, (size_t)(sub_m.end - pos) }, NULL };
                            pos = sub_m.end;
                            hit = 1;
                            /* Free sub_m but NOT its captures (we moved them) */
                            free(sub_m.cap);
                            if (sub_m.regex.capture) free((void *)sub_m.regex.capture);
                            break;
                        }
                    }
                }
                if (!hit) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                    continue;
                }
                continue;
            }
            if (t->modifier == MOD_GROUP) {
                if (t->sub_pattern) {
                    Match sub_m; memset(&sub_m, 0, sizeof(sub_m));
                    if (!match_pattern(src, src_len, t->sub_pattern, pos, &sub_m)) {
                        if (!t->optional) goto fail;
                        ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                        continue;
                    }
                    for (int ci = 0; ci < sub_m.count; ci++) {
                        ensure_cap(m);
                        m->cap[m->count++] = sub_m.cap[ci];
                    }
                    pos = sub_m.end;
                    free(sub_m.cap);
                    if (sub_m.regex.capture) free((void *)sub_m.regex.capture);
                } else {
                    if ((size_t)(src_len-pos) >= t->value.len && t->value.len > 0 &&
                        memcmp(src+pos, t->value.ptr, t->value.len) == 0)
                        pos += (int)t->value.len;
                    else if (!t->optional) goto fail;
                }
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(pos-s) }, NULL };
                continue;
            }
            if (t->modifier == MOD_STARTS || t->modifier == MOD_PREFIX) {
                if (t->sub_pattern) {
                    Match sub_m; memset(&sub_m, 0, sizeof(sub_m));
                    if (!match_pattern(src, src_len, t->sub_pattern, pos, &sub_m)) {
                        if (!t->optional) goto fail;
                        ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                        continue;
                    }
                    for (int ci = 0; ci < sub_m.count; ci++) {
                        ensure_cap(m);
                        m->cap[m->count++] = sub_m.cap[ci];
                    }
                    free(sub_m.cap);
                    if (sub_m.regex.capture) free((void *)sub_m.regex.capture);
                } else {
                    if ((size_t)(src_len-pos) < t->value.len ||
                        memcmp(src+pos, t->value.ptr, t->value.len) != 0) {
                        if (!t->optional) goto fail;
                        ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                        continue;
                    }
                }
            }

            if (nx && (nx->type == TOK_LITERAL || nx->type == TOK_BLOCK)) {
                while (src[pos]) {
                    if (src[pos] == '\n') break;
                    if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                    if (nx->type == TOK_BLOCK &&
                        sv_pfx(src+pos, nx->open)) break;
                    if (!CHAR_VALID(src[pos], pos, s)) break;
                    pos++;
                    if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && !t->sub_pattern &&
                        t->value.len > 0 && (size_t)(pos-s) >= t->value.len &&
                        memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) == 0) break;
                    if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && t->sub_pattern) {
                        /* Try matching sub-pattern ending at current pos */
                        for (int bp = s; bp < pos; bp++) {
                            int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                            if (me == pos) { goto ends_break_1; }
                        }
                    }
                }
                ends_break_1:

                {
                int end = pos;
                while (end > s && isspace((unsigned char)src[end-1])) end--;
                size_t clen = (size_t)(end - s);

                int failed = 0;
                if (t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) {
                    if (t->sub_pattern) {
                        /* Check if sub-pattern matches at any position ending at end */
                        int found_sp = 0;
                        for (int bp = s; bp < end; bp++) {
                            int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                            if (me == end) { found_sp = 1; break; }
                        }
                        if (!found_sp) failed = 1;
                        else if (t->modifier == MOD_SUFFIX) {
                            /* suffix needs more content before the sub-pattern match */
                            int earliest = end;
                            for (int bp = s; bp < end; bp++) {
                                int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                                if (me == end && bp > s) { earliest = bp; break; }
                            }
                            if (earliest <= s) failed = 1;
                        }
                    } else {
                        if (t->value.len > 0 && (clen < t->value.len || memcmp(src+end-t->value.len, t->value.ptr, t->value.len) != 0))
                            failed = 1;
                        else if (t->modifier == MOD_SUFFIX && clen <= t->value.len)
                            failed = 1;
                    }
                } else if (t->modifier == MOD_PREFIX) {
                    if (t->sub_pattern) {
                        int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, s);
                        if (me < 0 || me >= end) failed = 1;
                    } else {
                        if (clen <= t->value.len) failed = 1;
                    }
                } else if (t->modifier == MOD_INFIX) {
                    int found = 0;
                    if (t->sub_pattern) {
                        for (int bp = s + 1; bp < end; bp++) {
                            int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                            if (me > 0 && me < end && bp > s) { found = 1; break; }
                        }
                    } else {
                        if (clen >= t->value.len + 2 &&
                            memcmp(src + s, t->value.ptr, t->value.len) != 0 &&
                            memcmp(src + end - t->value.len, t->value.ptr, t->value.len) != 0) 
                        {
                            for (size_t j = 1; j <= clen - t->value.len - 1; j++) {
                                if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                    found = 1; break;
                                }
                            }
                        }
                    }
                    if (!found) failed = 1;
                } else if (t->modifier == MOD_INCLUDES) {
                    int found = 0;
                    if (t->sub_pattern) {
                        for (int bp = s; bp < end; bp++) {
                            int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                            if (me > 0 && me <= end) { found = 1; break; }
                        }
                    } else {
                        if (clen >= t->value.len) {
                            for (size_t j = 0; j <= clen - t->value.len; j++) {
                                if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                    found = 1; break;
                                }
                            }
                        }
                    }
                    if (!found) failed = 1;
                }

                if (failed) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; 
                    pos = s; continue;
                }
                if (end == s) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; 
                    pos = s; continue;
                }
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(end-s) }, NULL };
                pos = end; continue;
                } /* end scope for nxt+lit/blk/opt branch */
            }

            while (src[pos]) {
                if (nx && isspace((unsigned char)src[pos])) break;
                if (nx) {
                    if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                    if (nx->type == TOK_BLOCK &&
                        sv_pfx(src+pos, nx->open)) break;
                } else if (src[pos] == '\n') break; /* no next: stop only at newline */
                if (!CHAR_VALID(src[pos], pos, s)) break;
                pos++;
                if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && !t->sub_pattern &&
                    t->value.len > 0 && (size_t)(pos-s) >= t->value.len &&
                    memcmp(src+pos-t->value.len, t->value.ptr, t->value.len) == 0) break;
                if ((t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) && t->sub_pattern) {
                    for (int bp = s; bp < pos; bp++) {
                        int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                        if (me == pos) { goto ends_break_2; }
                    }
                }
            }
            ends_break_2:

            {
            int end = pos;
            while (end > s && isspace((unsigned char)src[end-1])) end--;
            size_t clen = (size_t)(end - s);

            int failed = 0;
            if (t->modifier == MOD_ENDS || t->modifier == MOD_SUFFIX) {
                if (t->sub_pattern) {
                    int found_sp = 0;
                    for (int bp = s; bp < end; bp++) {
                        int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                        if (me == end) { found_sp = 1; break; }
                    }
                    if (!found_sp) failed = 1;
                    else if (t->modifier == MOD_SUFFIX) {
                        int earliest = end;
                        for (int bp = s; bp < end; bp++) {
                            int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                            if (me == end && bp > s) { earliest = bp; break; }
                        }
                        if (earliest <= s) failed = 1;
                    }
                } else {
                    if (t->value.len > 0 && (clen < t->value.len || memcmp(src+end-t->value.len, t->value.ptr, t->value.len) != 0))
                        failed = 1;
                    else if (t->modifier == MOD_SUFFIX && clen <= t->value.len)
                        failed = 1;
                }
            } else if (t->modifier == MOD_PREFIX) {
                if (t->sub_pattern) {
                    int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, s);
                    if (me < 0 || me >= end) failed = 1;
                } else {
                    if (clen <= t->value.len) failed = 1;
                }
            } else if (t->modifier == MOD_INFIX) {
                int found = 0;
                if (t->sub_pattern) {
                    for (int bp = s + 1; bp < end; bp++) {
                        int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                        if (me > 0 && me < end && bp > s) { found = 1; break; }
                    }
                } else {
                    if (clen >= t->value.len + 2 &&
                        memcmp(src + s, t->value.ptr, t->value.len) != 0 &&
                        memcmp(src + end - t->value.len, t->value.ptr, t->value.len) != 0) 
                    {
                        for (size_t j = 1; j <= clen - t->value.len - 1; j++) {
                            if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                found = 1; break;
                            }
                        }
                    }
                }
                if (!found) failed = 1;
            } else if (t->modifier == MOD_INCLUDES) {
                int found = 0;
                if (t->sub_pattern) {
                    for (int bp = s; bp < end; bp++) {
                        int me = sub_pattern_matches_at(src, src_len, t->sub_pattern, bp);
                        if (me > 0 && me <= end) { found = 1; break; }
                    }
                } else {
                    if (clen >= t->value.len) {
                        for (size_t j = 0; j <= clen - t->value.len; j++) {
                            if (memcmp(src + s + j, t->value.ptr, t->value.len) == 0) {
                                found = 1; break;
                            }
                        }
                    }
                }
                if (!found) failed = 1;
            }

            if (failed) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; 
                pos = s; continue;
            }
            if (pos == s) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL }; 
                pos = s; continue;
            }
            ensure_cap(m);
            m->cap[m->count++] = (Capture){ t->var, { src+s, (size_t)(pos-s) }, NULL };
            continue;
            } /* end scope for no-nxt branch */
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

            size_t match_start = capture[0] ? (size_t)(capture[0] - (uint8_t *)src) : (size_t)pos;
            size_t match_end   = capture[1] ? (size_t)(capture[1] - (uint8_t *)src) : (size_t)pos;
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
            ensure_cap(m);
            m->cap[m->count++] = (Capture){ t->var, v, NULL };
            continue;
        }

    }

    m->end = pos; return 1;

fail:
    free_match(m);
    return 0;
}

#undef CHAR_VALID

/* =========================================================================
 * apply_replacement_ex
 * ====================================================================== */

static char *apply_replacement_ex(const char *rep, const Match *m,
                                   const Symbols *sym)
{
    StrBuf out; sb_init(&out);
    size_t n = strlen(rep), i = 0, sl = strlen(sym->sigil);

    while (i < n) {
        if (str_pfx(rep + i, sym->sigil)) {
            /* Case 1: Escaped sigil (e.g., $$ -> $) */
            if (i + sl < n && str_pfx(rep + i + sl, sym->sigil)) {
                sb_append_n(&out, sym->sigil, sl);
                i += sl * 2;
                continue;
            }
            /* Case 2: Braced variable (e.g., ${n}) */
            size_t ol = strlen(sym->open), cl = strlen(sym->close);
            if (i + sl < n && str_pfx(rep + i + sl, sym->open)) {
                size_t ns = i + sl + ol, ne = ns;
                while (ne + cl <= n && !str_pfx(rep + ne, sym->close)) ne++;
                if (ne + cl <= n && str_pfx(rep + ne, sym->close)) {
                    StrView name = { rep + ns, ne - ns };
                    int found = 0;
                    for (int k = 0; k < m->count; k++) {
                        if (sv_eq(m->cap[k].name, name)) {
                            sb_append_n(&out, m->cap[k].value.ptr, m->cap[k].value.len);
                            found = 1; break;
                        }
                    }
                    if (!found) { 
                        sb_append_n(&out, sym->sigil, sl); 
                        sb_append_n(&out, sym->open, ol);
                        sb_append_n(&out, name.ptr, name.len); 
                        sb_append_n(&out, sym->close, cl); 
                    }
                    i = ne + cl;
                    continue;
                }
            }

            /* Case 3: Simple variable (e.g., $n) */
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

static Papagaio *g_lazy_ctx = NULL;
static void pap_close_lazy_ctx(void) { if (g_lazy_ctx) { papagaio_close(g_lazy_ctx); g_lazy_ctx = NULL; } }

static Papagaio *pap_get_lazy_ctx(void)
{
    if (!g_lazy_ctx) {
        g_lazy_ctx = papagaio_open();
        if (g_lazy_ctx) atexit(pap_close_lazy_ctx);
    }
    return g_lazy_ctx;
}

static char *pap_process_impl(const char *input,
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
    Papagaio *ctx = pap_get_lazy_ctx();
    char *prepared = strdup(input);
    char *preprocessed = resolve_preprocessor(ctx, prepared, &sym); free(prepared);
    char *dispatched = dispatch_commands(ctx, preprocessed, &sym); free(preprocessed);

    const char *work = dispatched;
    int len = (int)strlen(work), pos = 0;

    while (pos < len) {
        int matched = 0;
        for (int i = 0; i < rc; i++) {
            Match m;
            if (match_pattern(work, len, &rules[i].pattern, pos, &m)) {
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym);
                sb_append_n(&out, r, (int)strlen(r)); free(r);
                pos = m.end; free_match(&m); matched = 1; break;
            }
        }
        if (!matched) sb_append_char(&out, work[pos++]);
    }

    free(dispatched);
    for (int i = 0; i < rc; i++) free_pattern(&rules[i].pattern);
    free(rules);

    char *result = (char *)malloc(out.len + 1);
    memcpy(result, out.data, out.len + 1);
    sb_free(&out);
    return result;
}

/* =========================================================================
 * Plugin Implementation Helpers
 * ====================================================================== */

int papagaio_register_command(Papagaio *ctx, const char *name, PapCommandHandler handler, void *ud)
{
    if (!ctx) return -1;
    if (!name || !handler) return -1;
    if (ctx->cmd_count >= ctx->cmd_cap) {
        ctx->cmd_cap = ctx->cmd_cap ? ctx->cmd_cap << 1 : 8;
        ctx->commands = (RegisteredCommand *)realloc(ctx->commands, sizeof(RegisteredCommand) * ctx->cmd_cap);
    }
    ctx->commands[ctx->cmd_count].name = strdup(name);
    ctx->commands[ctx->cmd_count].handler = handler;
    ctx->commands[ctx->cmd_count].userdata = ud;
    ctx->commands[ctx->cmd_count].is_wasm = 0;
    ctx->commands[ctx->cmd_count].wasm_func = NULL;
    ctx->cmd_count++;
    return 0;
}



int papagaio_register_modifier(Papagaio *ctx, const char *name, PapModifierHandler handler, void *ud)
{
    if (!ctx || !name || !handler) return -1;
    if (ctx->mod_count >= ctx->mod_cap) {
        ctx->mod_cap = ctx->mod_cap ? ctx->mod_cap << 1 : 8;
        ctx->modifiers = (RegisteredModifier *)realloc(ctx->modifiers, sizeof(RegisteredModifier) * ctx->mod_cap);
    }
    ctx->modifiers[ctx->mod_count].name = strdup(name);
    ctx->modifiers[ctx->mod_count].handler = handler;
    ctx->modifiers[ctx->mod_count].userdata = ud;
    ctx->mod_count++;
    return 0;
}



static char *wasm_plugin_handler(Papagaio *ctx, const char *cmd_name, int argc, const char **argv, const size_t *argl, void *ud) {
    (void)ctx; (void)cmd_name; (void)ud;
    if (argc < 2) return strdup("");
    
    /* Copy name and body to null-terminated strings */
    char *name = (char*)malloc(argl[0] + 1);
    char *body = (char*)malloc(argl[1] + 1);
    if (!name || !body) { free(name); free(body); return NULL; }
    memcpy(name, argv[0], argl[0]); name[argl[0]] = '\0';
    memcpy(body, argv[1], argl[1]); body[argl[1]] = '\0';

    /* Wasm C boilerplate generator (New Convention: no imports) */
    char *tpl =
        "/* Generated by Papagaio Wasm SDK */\n"
        "typedef unsigned int  uint32_t;\n"
        "typedef unsigned long size_t;\n"
        "extern void *malloc(size_t sz);\n"
        "extern void free(void *ptr);\n\n"
        "/* argv_ptr is a Wasm offset to an i32 table of string offsets */\n"
        "static const char *_pap_get_arg(uint32_t argv_ptr, int i) {\n"
        "    uint32_t *table = (uint32_t *)argv_ptr;\n"
        "    return (const char *)table[i];\n"
        "}\n\n"
        "/* Implementation */\n"
        "char* intern_cmd_%s(int argc, const char **argv) {\n"
        "%s\n"
        "}\n\n"
        "/* Bridge - called by host with (argc, argv_table_ptr) */\n"
        "__attribute__((export_name(\"papagaio_%s\")))\n"
        "char* pap_bridge_%s(int argc, uint32_t argv_ptr) {\n"
        "    const char **argv = (const char **)malloc(argc * sizeof(const char *));\n"
        "    for (int i = 0; i < argc; i++) argv[i] = _pap_get_arg(argv_ptr, i);\n"
        "    char *res = intern_cmd_%s(argc, argv);\n"
        "    free((void*)argv);\n"
        "    return res;\n"
        "}\n";

    int exists = -1;
    for (int i = 0; i < ctx->cmd_count; i++) {
        if (strcmp(ctx->commands[i].name, name) == 0) { exists = i; break; }
    }
    
    if (exists < 0) {
        papagaio_register_command(ctx, name, wasm_command_bridge, NULL);
        ctx->commands[ctx->cmd_count - 1].is_wasm = 1;
        ctx->commands[ctx->cmd_count - 1].wasm_func = NULL;
    }

    size_t out_sz = strlen(tpl) + strlen(name) * 4 + strlen(body) + 256;
    char *out = (char*)malloc(out_sz);
    if (out) {
        snprintf(out, out_sz, tpl, name, body, name, name, name);
    }
    free(name);
    free(body);
    return out;
}

/* =========================================================================
 * Wasm Host Functions - minimal, only needed for legacy compat
 * The new convention passes args via Wasm memory directly from the bridge.
 * ====================================================================== */
static Papagaio *g_active_ctx = NULL;

m3ApiRawFunction(host_write) {
    m3ApiGetArgMem(const char *, buf);
    m3ApiGetArg(int32_t, len);
    if (buf) fwrite(buf, 1, (size_t)len, stdout);
    m3ApiSuccess();
}

m3ApiRawFunction(host_write_err) {
    m3ApiGetArgMem(const char *, buf);
    m3ApiGetArg(int32_t, len);
    if (buf) fwrite(buf, 1, (size_t)len, stderr);
    m3ApiSuccess();
}

m3ApiRawFunction(host_abort) {
    m3ApiGetArgMem(const char *, msg);
    fprintf(stderr, "[WASM ABORT] %s\n", msg ? msg : "unknown error");
    abort();
}

static void link_host_functions(Papagaio *ctx, IM3Module module) {
    (void)ctx;
    m3_LinkRawFunction(module, "env", "__host_write",     "v(*i)", host_write);
    m3_LinkRawFunction(module, "env", "__host_write_err", "v(*i)", host_write_err);
    m3_LinkRawFunction(module, "env", "__host_abort",     "v(*)",  host_abort);
}

/* =========================================================================
 * Public C API
 * ====================================================================== */

Papagaio *papagaio_open(void)
{
    Papagaio *ctx = (Papagaio *)malloc(sizeof(Papagaio));
    if (!ctx) return NULL;
    ctx->env        = m3_NewEnvironment();
    ctx->runtime    = m3_NewRuntime(ctx->env, 1024 * 1024, NULL);
    ctx->commands   = NULL; ctx->cmd_count = 0; ctx->cmd_cap = 0;
    ctx->modifiers  = NULL; ctx->mod_count = 0; ctx->mod_cap = 0;
    ctx->finalizers = NULL; ctx->fin_count = 0; ctx->fin_cap = 0;
    ctx->argc       = 0;    ctx->argv      = NULL;
    papagaio_register_command(ctx, "wasm_plugin", wasm_plugin_handler, NULL);
    papagaio_register_command(ctx, "wasmfile", wasm_file_handler, NULL);
    papagaio_register_command(ctx, "wasm", wasm_bytes_handler, NULL);
    return ctx;
}

void papagaio_close(Papagaio *ctx)
{
    if (!ctx) return;
    for (int i = 0; i < ctx->fin_count; i++) {
        if (ctx->finalizers[i].fn)
            ctx->finalizers[i].fn(ctx->finalizers[i].userdata);
    }
    if (ctx->runtime) m3_FreeRuntime(ctx->runtime);
    if (ctx->env)     m3_FreeEnvironment(ctx->env);
    free(ctx->commands);
    free(ctx->modifiers);
    free(ctx->finalizers);
    free(ctx);
}

void papagaio_set_args(Papagaio *ctx, int argc, char **argv)
{
    if (!ctx) return;
    ctx->argc = argc;
    ctx->argv = argv;
}

void papagaio_get_args(Papagaio *ctx, int *argc, char ***argv)
{
    if (!ctx) { if (argc) *argc = 0; if (argv) *argv = NULL; return; }
    if (argc) *argc = ctx->argc;
    if (argv) *argv = ctx->argv;
}

int papagaio_has_command(Papagaio *ctx, const char *name)
{
    if (!ctx) return 0;
    for (int i = 0; i < ctx->cmd_count; i++) {
        if (strcmp(ctx->commands[i].name, name) == 0) return 1;
    }
    return 0;
}

char *papagaio_process(const char *input, ...)
{
    va_list args; va_start(args, input);
    char *r = pap_process_impl(input, PAP_SIGIL, PAP_OPEN, PAP_CLOSE, args);
    va_end(args); return r;
}

char *papagaio_process_ex(const char *input, const char *sigil,
                          const char *open, const char *close, ...)
{
    va_list args; va_start(args, close);
    char *r = pap_process_impl(input, sigil, open, close, args);
    va_end(args); return r;
}

char *papagaio_process_pairs(Papagaio *ctx, const char *input,
                             const char **patterns, const char **repls,
                             int pair_count)
{
    (void)ctx;
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
                char *r = apply_replacement_ex(rules[i].replacement, &m, &sym);
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

/* =========================================================================
 * resolve_directives
 * ====================================================================== */
static unsigned char base64_table[256] = {0};
static void init_b64(void) {
    if (base64_table[(int)'A']) return;
    const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for(int i=0; i<64; i++) base64_table[(int)chars[i]] = i;
}
static uint8_t *decode_b64(const char *src, size_t len, size_t *out_len) {
    init_b64();
    uint8_t *out = (uint8_t *)malloc(len);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i < len) {
        uint32_t v = 0;
        int cnt = 0;
        int pad = 0;
        while (cnt < 4 && i < len) {
            char c = src[i++];
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') continue;
            if (c == '=') { pad++; v <<= 6; cnt++; continue; }
            if (pad > 0) continue; /* Ignore everything after first padding char in quartet */
            v = (v << 6) | (base64_table[(int)c] & 0x3F);
            cnt++;
        }
        if (cnt < 4) {
            if (cnt > 0) { /* Partial block at end of string */
                 v <<= (6 * (4 - cnt));
                 if (cnt >= 2) out[j++] = (uint8_t)((v >> 16) & 0xFF);
                 if (cnt >= 3) out[j++] = (uint8_t)((v >> 8) & 0xFF);
            }
            break;
        }
        out[j++] = (uint8_t)((v >> 16) & 0xFF);
        if (pad < 2) out[j++] = (uint8_t)((v >> 8) & 0xFF);
        if (pad < 1) out[j++] = (uint8_t)(v & 0xFF);
        if (pad > 0) break; /* End of stream on padding */
    }
    *out_len = j;
    return out;
}


static void papagaio_load_wasm_bytes(Papagaio *ctx, uint8_t *bytes, size_t size) {
    IM3Module module;
    M3Result result = m3_ParseModule(ctx->env, &module, (bytes_t)bytes, (uint32_t)size);
    if (result) { fprintf(stderr, "[WASM] Parse Error: %s\n", result); return; }
    
    result = m3_LoadModule(ctx->runtime, module);
    if (result) { fprintf(stderr, "[WASM] Load Error: %s\n", result); return; }
    
    /* Link MUST happen after LoadModule */
    link_host_functions(ctx, module);
    
    m3_CompileModule(module);

    /* Intelligent Auto-Registration: Scan module functions for exports */
    for (uint32_t i = 0; i < module->numFunctions; i++) {
        M3Function *f_info = &module->functions[i];
        if (f_info->export_name) {
            fprintf(stderr, "[DEBUG] Export found: '%s'\n", f_info->export_name);
        }
        if (f_info->export_name && strncmp(f_info->export_name, "papagaio_", 9) == 0) {
            const char *cmd_name = f_info->export_name + 9;
            fprintf(stderr, "[DEBUG] Registering command: '%s'\n", cmd_name);
            
            /* Must use m3_FindFunction AFTER compile to get JIT-ready handle */
            IM3Function f_ready = NULL;
            M3Result fres = m3_FindFunction(&f_ready, ctx->runtime, f_info->export_name);
            if (fres || !f_ready) {
                fprintf(stderr, "[WASM] Could not find compiled '%s': %s\n", f_info->export_name, fres ? fres : "null");
                continue;
            }
            
            int found = -1;
            for (int ci = 0; ci < ctx->cmd_count; ci++) {
                if (strcmp(ctx->commands[ci].name, cmd_name) == 0) { found = ci; break; }
            }
            if (found >= 0) {
                ctx->commands[found].wasm_func = f_ready;
                ctx->commands[found].userdata = (void*)f_ready;
            } else {
                papagaio_register_command(ctx, cmd_name, wasm_command_bridge, (void*)f_ready);
                ctx->commands[ctx->cmd_count-1].is_wasm = 1;
                ctx->commands[ctx->cmd_count-1].wasm_func = f_ready;
            }
        }
    }
}

static void papagaio_load_wasm_file(Papagaio *ctx, const char *path) {
    char trim_path[256]; size_t pl = strlen(path);
    size_t start = 0; while(start < pl && isspace((unsigned char)path[start])) start++;
    size_t end = pl; while(end > start && isspace((unsigned char)path[end-1])) end--;
    size_t len = end - start; if (len >= 255) len = 255;
    memcpy(trim_path, path + start, len); trim_path[len] = '\0';
    FILE *f = fopen(trim_path, "rb"); if (!f) { fprintf(stderr, "[ERROR] Could not open Wasm file: '%s'\n", trim_path); return; }
    fseek(f, 0, SEEK_END); size_t size = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *bytes = malloc(size);
    if (bytes) { 
        if (fread(bytes, 1, size, f) == size) papagaio_load_wasm_bytes(ctx, bytes, size); 
        else fprintf(stderr, "[ERROR] Could not read Wasm file: '%s'\n", trim_path);
        free(bytes); 
    }
    fclose(f);
}

static char *wasm_command_bridge(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud) {
    (void)name; (void)argl;
    IM3Function f = (IM3Function)ud;
    if (!f) return NULL;

    /* Serialize args into Wasm memory:
       Layout: [argc * i32 offsets] [str0\0] [str1\0] ...
       Starting at offset ARGS_BASE (leave first 4KB for stack/rodata) */
    const uint32_t ARGS_BASE = 4096;
    uint32_t mem_size = 0;
    uint8_t *mem = m3_GetMemory(ctx->runtime, &mem_size, 0);
    if (!mem) return NULL;

    /* Write string data after the pointer table */
    uint32_t table_size = (uint32_t)argc * 4; /* array of i32 offsets */
    uint32_t str_base = ARGS_BASE + table_size;
    uint32_t cur = str_base;

    for (int i = 0; i < argc; i++) {
        size_t slen = argl[i];
        if (cur + slen + 1 > mem_size) return NULL; /* out of bounds */
        memcpy(mem + cur, argv[i], slen);
        mem[cur + slen] = '\0';
        /* Write the offset into the pointer table */
        uint32_t off = cur;
        if (ARGS_BASE + i * 4 + 4 > mem_size) return NULL;
        memcpy(mem + ARGS_BASE + i * 4, &off, 4);
        cur += (uint32_t)(slen + 1 + 7) & ~7u; /* align 8 */
    }

    /* Call: papagaio_X(argc: i32, argv_table: i32) -> i32 */
    g_active_ctx = ctx;
    M3Result res = m3_CallV(f, (uint32_t)argc, (uint32_t)ARGS_BASE);
    g_active_ctx = NULL;

    if (res) {
        fprintf(stderr, "[WASM] CallV Error: %s\n", res);
        return NULL;
    }

    uint32_t wasm_ptr = 0;
    m3_GetResultsV(f, &wasm_ptr);
    if (wasm_ptr == 0) return NULL;

    mem = m3_GetMemory(ctx->runtime, &mem_size, 0);
    if (!mem || wasm_ptr >= mem_size) return NULL;

    char *res_str = strdup((const char *)(mem + wasm_ptr));
    return res_str;
}

static char *wasm_file_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud) {
    (void)name; (void)ud; (void)argl; if (argc > 0) papagaio_load_wasm_file(ctx, argv[0]); return strdup("");
}

static char *wasm_bytes_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud) {
    (void)name; (void)ud; (void)argl; if (argc > 0) {
        size_t out_len = 0; uint8_t *bytes = decode_b64(argv[0], strlen(argv[0]), &out_len);
        if (bytes) { papagaio_load_wasm_bytes(ctx, bytes, out_len); free(bytes); }
    }
    return strdup("");
}

static char *resolve_preprocessor(Papagaio *ctx, const char *src, Symbols *sym)
{
    StrBuf out; sb_init(&out);
    size_t i = 0, len = strlen(src);

    while (i < len) {
        if (src[i] == '$') { /* FIXED SIGIL for preprocessor */
            size_t j = i + 1;
            size_t ks = j;
            while (j < len && (isalnum((unsigned char)src[j]) || src[j] == '_')) j++;
            size_t klen = j - ks;

            if (klen > 0) {
                int is_sym = (klen == 13 && memcmp(src + ks, "changesymbols", 13) == 0);
                
                if (is_sym) {
                    size_t saved_j = j;
                    while (j < len && isspace((unsigned char)src[j])) j++;
                    if (j < len && src[j] == '{') {
                        StrView b1, b2, b3, b4;
                        if (is_sym) {
                            int next1 = extract_block(src, (int)j, (StrView){"{",1}, (StrView){"}",1}, &b1);
                            size_t j2 = (size_t)next1; while (j2 < len && isspace((unsigned char)src[j2])) j2++;
                            if (j2 < len && src[j2] == '{') {
                                int next2 = extract_block(src, (int)j2, (StrView){"{",1}, (StrView){"}",1}, &b2);
                                size_t j3 = (size_t)next2; while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                                if (j3 < len && src[j3] == '{') {
                                    int next3 = extract_block(src, (int)j3, (StrView){"{",1}, (StrView){"}",1}, &b3);
                                    size_t j4 = (size_t)next3; while (j4 < len && isspace((unsigned char)src[j4])) j4++;
                                    if (j4 < len && src[j4] == '{') {
                                        int next4 = extract_block(src, (int)j4, (StrView){"{",1}, (StrView){"}",1}, &b4);
                                        StrView t1 = trim_sv(b1), t2 = trim_sv(b2), t3 = trim_sv(b3), t4 = trim_sv(b4);
                                        
                                        if (t1.len > 0 && t1.len < 16) {
                                            memcpy(sym->sigil, t1.ptr, t1.len); sym->sigil[t1.len] = '\0';
                                        }
                                        if (t2.len > 0 && t2.len < 16 && t3.len > 0 && t3.len < 16) {
                                            memcpy(sym->open,  t2.ptr, t2.len); sym->open[t2.len]  = '\0';
                                            memcpy(sym->close, t3.ptr, t3.len); sym->close[t3.len] = '\0';
                                        }
                                        if (t4.len > 0 && t4.len < 16) {
                                            memcpy(sym->optional, t4.ptr, t4.len);
                                            sym->optional[t4.len] = '\0';
                                        }
                                        i = (size_t)next4; continue;
                                    }
                                }
                            }
                        }
                    }
                    j = saved_j; /* restore if blocks didn't match perfectly */
                }
            }
        }
        sb_append_char(&out, src[i++]);
    }
    return out.data;
}

static char *dispatch_commands(Papagaio *ctx, const char *src, const Symbols *sym)
{
    if (!ctx || !src) return src ? strdup(src) : NULL;
    StrBuf out; sb_init(&out);
    size_t i = 0, len = strlen(src);
    char sigil = sym->sigil[0];
    StrView so = { sym->open,  strlen(sym->open)  };
    StrView sc = { sym->close, strlen(sym->close) };

    while (i < len) {
        if (src[i] == sigil) {
            size_t j = i + 1;
            size_t ks = j;
            while (j < len && (isalnum((unsigned char)src[j]) || src[j] == '_')) j++;
            size_t klen = j - ks;

            if (klen > 0) {
                int found = -1;
                for (int ci = 0; ci < ctx->cmd_count; ci++) {
                    if (strlen(ctx->commands[ci].name) == klen &&
                        memcmp(ctx->commands[ci].name, src + ks, klen) == 0) {
                        found = ci; break;
                    }
                }

                if (found >= 0) {
                    char *vargv[32]; size_t vargl[32]; int vargc = 0;
                    while (vargc < 32) {
                        size_t sj = j;
                        while(j < len && isspace((unsigned char)src[j])) j++;
                        if (j < len && sv_pfx(src + j, so)) {
                            StrView blk; j = (size_t)extract_block(src, (int)j, so, sc, &blk);
                            char *arg = (char*)malloc(blk.len + 1);
                            if (arg) { memcpy(arg, blk.ptr, blk.len); arg[blk.len] = '\0'; }
                            vargv[vargc] = arg; vargl[vargc] = blk.len; vargc++;
                        } else { j = sj; break; }
                    }
                    RegisteredCommand *cmd = &ctx->commands[found];
                    char *res = cmd->handler(ctx, cmd->name, vargc, (const char **)vargv, vargl, cmd->userdata);
                    if (res) { sb_append_n(&out, res, strlen(res)); free(res); }
                    for (int ci = 0; ci < vargc; ci++) if (vargv[ci]) free(vargv[ci]);
                    i = j; continue;
                } else if (klen == 8 && memcmp(src + ks, "document", 8) == 0) {
                    /* Special built-in $document operator */
                    sb_append_n(&out, src, len);
                    i = j; continue;
                }
            }
        }
        sb_append_char(&out, src[i++]);
    }
    char *ret = strdup(out.data); sb_free(&out); return ret;
}

char *papagaio_process_text(Papagaio *ctx, const char *input, size_t len)
{
    if (!ctx || !input) return NULL;
    Symbols sym = make_symbols(PAP_SIGIL, PAP_OPEN, PAP_CLOSE);

    char *buf = (char *)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, input, len); buf[len] = '\0';

    char *preprocessed = resolve_preprocessor(ctx, buf, &sym); free(buf);
    if (!preprocessed) return NULL;
    
    /* A. Resolve Patterns */
    PatternPair *pairs = NULL; int pc = 0;
    char *text_no_patterns = extract_nested(preprocessed, &sym, &pairs, &pc);
    free(preprocessed);
    if (!text_no_patterns) { free_pairs(pairs, pc); return NULL; }
    
    char *cur = text_no_patterns;
    for (int i = 0; i < pc; i++) {
        StrBuf out; sb_init(&out);
        size_t clen = strlen(cur), pos = 0;
        Pattern pat;
        parse_pattern_ex(pairs[i].m, &pat, &sym);
        
        while (pos < clen) {
            Match m;
            if (match_pattern(cur, (int)clen, &pat, (int)pos, &m)) {
                char *r = apply_replacement_ex(pairs[i].r, &m, &sym);
                sb_append_n(&out, r, strlen(r));
                free(r);
                pos = (size_t)m.end;
                free_match(&m);
            } else {
                sb_append_char(&out, cur[pos++]);
            }
        }
        free_pattern(&pat);
        free(cur);
        cur = out.data;
    }
    free_pairs(pairs, pc);

    char *final = dispatch_commands(ctx, cur, &sym);
    free(cur);
    
    return final;
}

