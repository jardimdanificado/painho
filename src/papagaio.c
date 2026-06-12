#define _DEFAULT_SOURCE
#include "papagaio.h"
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#ifndef PAPAGAIO_NO_DL
#if defined(_WIN32)
#define PAPAGAIO_USE_WINDOWS_DL
#include <windows.h>
#elif (defined(__unix__) || defined(__APPLE__) || defined(__HAIKU__)) && !defined(__EMSCRIPTEN__)
#define PAPAGAIO_USE_POSIX_DL
#include <dlfcn.h>
#endif
#endif

typedef struct { const char *ptr; size_t len; } StrView;
typedef struct { char *data; size_t len; size_t cap; } StrBuf;

typedef enum {
    TOK_LITERAL, TOK_VAR, TOK_BLOCK, TOK_WS,
    TOK_OPTIONS_OBSOLETE, TOK_OPTIONAL_LIT
} PapTokenType;

typedef enum {
    MOD_NONE, MOD_INT, MOD_FLOAT, MOD_NUMBER,
    MOD_UPPER, MOD_LOWER, MOD_CAPITALIZED,
    MOD_WORD, MOD_IDENTIFIER, MOD_HEX, MOD_PATH,
    MOD_BINARY, MOD_PERCENT, MOD_ALIASES,
    MOD_GROUP, MOD_STARTS, MOD_ENDS,
    MOD_PREFIX, MOD_SUFFIX, MOD_INFIX,
    MOD_INCLUDES, MOD_REPEAT, MOD_WHILE, MOD_UNTIL, MOD_BYTE, MOD_CUSTOM
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

    char       *open_str;
    char       *close_str;
    unsigned    optional    : 1;
    unsigned    ws_consume  : 1; /* trailing sigil: eat whitespace after match */
    int         next_sig;
    unsigned    all_opt     : 1;
    char      **alts;
    int         alt_count;
    char       *literal_str;
    Pattern    *sub_pattern;      /* for optional/starts/ends/etc: recursive sub-pattern */
    Pattern   **alt_patterns;     /* for aliases: one sub-pattern per alternative */
    int         alt_pattern_count;
    char       *custom_mod_name;  /* for MOD_CUSTOM: name of registered modifier */
} PapToken;

typedef struct {
    char sigil[16];
    char open[16];
    char close[16];
    char optional[16]; 
    const char *pattern, *block, *changequote;
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

    Papagaio *ctx;
} Match;

typedef struct { Pattern pattern; const char *replacement; } Rule;
typedef struct { char *m; char *r; } PatternPair;

/* =========================================================================
 * Command Registration
 * ====================================================================== */

typedef struct {
    PapFinalizer fn;
    void        *userdata;
} RegisteredFinalizer;

typedef struct {
    char        *name;
    PapCommandHandler handler;
    void        *userdata;
} RegisteredCommand;

typedef struct {
    char *name;
    PapModifierHandler handler;
    void *userdata;
} RegisteredModifier;

/* Forward declarations */
static char *include_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *ud);

typedef struct Scope {
    PatternPair *rules;
    int rule_count, rule_cap;
    struct Scope *parent;
} Scope;

struct Papagaio {
    RegisteredCommand  *commands;
    int                 cmd_count, cmd_cap;

    RegisteredModifier *modifiers;
    int                 mod_count, mod_cap;

    RegisteredFinalizer *finalizers;
    int                  fin_count, fin_cap;

    /* host arguments */
    int    argc;
    char **argv;

    /* preprocessor options */
    int    auto_export;

    /* patterns - persistent within one top-level call */
    Scope *global_scope;
    Scope *current_scope;
    int          depth;
    int          disable_sandbox;
    int          disable_patterns;
    int          once_mode;

    /* original document for $document$original */
    char        *original_doc;
    size_t       original_len;

    /* dynamic libraries */
    void       **dl_handles;
    int          dl_count, dl_cap;

    /* CLI-only features */
    int          cli_mode;
};

/* =========================================================================
 * Constants
 * ====================================================================== */

#define PAP_SIGIL    "$"
#define PAP_OPEN     "{"
#define PAP_CLOSE    "}"
#define PAP_PATTERN  "pattern"
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
    s.pattern  = PAP_PATTERN;
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
        free(p->t[i].custom_mod_name);

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
        pos += (int)o.len;
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
    pos += (int)o.len;
    int start = pos, depth = 1;
    while (src[pos] && depth) {
        if      (sv_pfx(src + pos, o)) { depth++; pos += (int)o.len; }
        else if (sv_pfx(src + pos, c)) {
            if (!--depth) {
                out->ptr = src + start; out->len = (size_t)(pos - start);
                return pos + (int)c.len;
            }
            pos += (int)c.len;
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
static int  match_pattern(Papagaio *ctx, const char *src, int src_len,
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

                /* Empty modifier name: treat as trailing sigil (ws_consume) */
                if (mod.len == 0) {
                    i -= sl;  /* back up so trailing sigil handler can process it */
                }
                else if (sv_eq(mod, (StrView){"int",         3 })) t->modifier = MOD_INT;
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
                else {
                    /* unrecognized modifier: treat as custom */
                    t->modifier = MOD_CUSTOM;
                    t->custom_mod_name = (char *)malloc(mod.len + 1);
                    memcpy(t->custom_mod_name, mod.ptr, mod.len);
                    t->custom_mod_name[mod.len] = '\0';
                }
            }

            if (i < n && str_pfx(pat + i, sym->optional)) { t->optional = 1; i += (int)strlen(sym->optional); }
            /* Trailing sigil after var/modifier: next whitespace is consumed (optional) */
            if (i < n && str_pfx(pat + i, sym->sigil)) {
                size_t sl2 = strlen(sym->sigil);
                size_t j2 = i + sl2;
                /* Only treat as ws_consume if NOT followed by alphanum (would start new var) */
                if (j2 >= (size_t)n || (!isalnum((unsigned char)pat[j2]) && pat[j2] != '_')) {
                    t->ws_consume = 1;
                    i += (int)sl2;
                }
            }
            if (t->type != TOK_BLOCK) {
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
        if (i < n && str_pfx(pat + i, sym->optional)) { p->t[p->count-1].optional = 1; i += (int)strlen(sym->optional); }
        /* Trailing sigil on literal: consume whitespace after match */
        if (i < n && str_pfx(pat + i, sym->sigil)) {
            size_t sl2 = strlen(sym->sigil);
            size_t j2 = i + sl2;
            if (j2 >= (size_t)n || (!isalnum((unsigned char)pat[j2]) && pat[j2] != '_')) {
                p->t[p->count-1].ws_consume = 1;
                i += (int)sl2;
            }
        }
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
            /* WS after a ws_consume token is inherently optional */
            if (p->t[b].ws_consume)  p->t[a].optional = 1;
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
static int sub_pattern_matches_at(Papagaio *ctx, const char *src, int src_len, Pattern *sp, int at)
{
    Match sub_m; sub_m.ctx = ctx;
    if (match_pattern(ctx, src, src_len, sp, at, &sub_m)) {
        int end = sub_m.end;
        free_match(&sub_m);
        return end;
    }
    return -1;
}


static int match_pattern(Papagaio *ctx, const char *src, int src_len,
                          Pattern *p, int start, Match *m)
{
    m->ctx      = ctx;
    m->cap_size = 16; m->count = 0;
    m->cap      = (Capture *)malloc(sizeof(Capture) * m->cap_size);
    m->start    = start; m->src = src;


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
            pos += (int)t->value.len;
            if (t->ws_consume) { while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r') pos++; }
            continue;
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
                        Match sub_m; sub_m.ctx = ctx;
                        if (match_pattern(ctx, src, src_len, t->alt_patterns[ai], pos, &sub_m)) {
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
                    if (!match_pattern(ctx, src, src_len, t->sub_pattern, pos, &sub_m)) {
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
            if (t->modifier == MOD_CUSTOM) {
                int s = pos;
                if (nx && (nx->type == TOK_LITERAL || nx->type == TOK_BLOCK)) {
                    while (src[pos]) {
                        if (src[pos] == '\n') break;
                        if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                        if (nx->type == TOK_BLOCK && sv_pfx(src+pos, nx->open)) break;
                        pos++;
                    }
                } else {
                    while (src[pos]) {
                        if (nx && isspace((unsigned char)src[pos])) break;
                        if (nx) {
                            if (nx->type == TOK_LITERAL && sv_pfx(src+pos, nx->value)) break;
                            if (nx->type == TOK_BLOCK && sv_pfx(src+pos, nx->open)) break;
                        } else if (src[pos] == '\n') break;
                        pos++;
                    }
                }
                int end = pos;
                while (end > s && isspace((unsigned char)src[end-1])) end--;
                size_t clen = (size_t)(end - s);
                if (clen == 0) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                    pos = s; continue;
                }
                RegisteredModifier *rm = NULL;
                if (ctx && t->custom_mod_name) {
                    for (int mi = 0; mi < ctx->mod_count; mi++) {
                        if (strcmp(ctx->modifiers[mi].name, t->custom_mod_name) == 0) {
                            rm = &ctx->modifiers[mi];
                            break;
                        }
                    }
                }
                if (!rm) {
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                    pos = s; continue;
                }
                char *res = rm->handler(src + s, rm->name, clen, strlen(rm->name), rm->userdata);
                if (!res || !res[0]) {
                    free(res);
                    if (!t->optional) goto fail;
                    ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                    pos = s; continue;
                }
                ensure_cap(m);
                m->cap[m->count++] = (Capture){ t->var, { res, strlen(res) }, res };
                pos = end;
                if (t->ws_consume) { while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r') pos++; }
                continue;
            }
            if (t->modifier == MOD_STARTS || t->modifier == MOD_PREFIX) {
                if (t->sub_pattern) {
                    Match sub_m; memset(&sub_m, 0, sizeof(sub_m));
                    if (!match_pattern(ctx, src, src_len, t->sub_pattern, pos, &sub_m)) {
                        if (!t->optional) goto fail;
                        ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                        continue;
                    }
                    for (int ci = 0; ci < sub_m.count; ci++) {
                        ensure_cap(m);
                        m->cap[m->count++] = sub_m.cap[ci];
                    }
                    free(sub_m.cap);
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
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                            if (me == end) { found_sp = 1; break; }
                        }
                        if (!found_sp) failed = 1;
                        else if (t->modifier == MOD_SUFFIX) {
                            /* suffix needs more content before the sub-pattern match */
                            int earliest = end;
                            for (int bp = s; bp < end; bp++) {
                                int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, s);
                        if (me < 0 || me >= end) failed = 1;
                    } else {
                        if (clen <= t->value.len) failed = 1;
                    }
                } else if (t->modifier == MOD_INFIX) {
                    int found = 0;
                    if (t->sub_pattern) {
                        for (int bp = s + 1; bp < end; bp++) {
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
                pos = end;
                if (t->ws_consume) { while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r') pos++; }
                continue;
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
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
                        if (me == end) { found_sp = 1; break; }
                    }
                    if (!found_sp) failed = 1;
                    else if (t->modifier == MOD_SUFFIX) {
                        int earliest = end;
                        for (int bp = s; bp < end; bp++) {
                            int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
                    int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, s);
                    if (me < 0 || me >= end) failed = 1;
                } else {
                    if (clen <= t->value.len) failed = 1;
                }
            } else if (t->modifier == MOD_INFIX) {
                int found = 0;
                if (t->sub_pattern) {
                    for (int bp = s + 1; bp < end; bp++) {
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
                        int me = sub_pattern_matches_at(ctx, src, src_len, t->sub_pattern, bp);
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
            if (t->ws_consume) { while (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r') pos++; }
            continue;
            } /* end scope for no-nxt branch */
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

            /* Case 2: Braced variable (e.g., ${n}suffix) */
            size_t ol = strlen(sym->open), cl = strlen(sym->close);
            if (ol > 0 && i + sl + ol <= n && str_pfx(rep + i + sl, sym->open)) {
                /* Search for matching close delimiter, respecting nesting */
                size_t ns = i + sl + ol;
                size_t ne = ns;
                int depth = 1;
                while (ne < n && depth > 0) {
                    if (cl > 0 && ne + cl <= n && str_pfx(rep + ne, sym->open) && strcmp(sym->open, sym->close) != 0) {
                        depth++; ne += ol;
                    } else if (cl > 0 && ne + cl <= n && str_pfx(rep + ne, sym->close)) {
                        depth--;
                        if (depth == 0) break;
                        ne += cl;
                    } else {
                        ne++;
                    }
                }
                if (depth == 0 && ne >= ns) {
                    StrView name = { rep + ns, ne - ns };
                    /* Only treat as braced var if name is valid identifier */
                    int valid_name = (name.len > 0);
                    for (size_t vi = 0; vi < name.len && valid_name; vi++) {
                        if (!isalnum((unsigned char)name.ptr[vi]) && name.ptr[vi] != '_') valid_name = 0;
                    }
                    if (valid_name) {
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
 * List Operation Subsystem
 * ====================================================================== */

/* Lookup the current value of a $NAME variable set via $from / $list$set.
   Returns a newly malloc'd copy, or NULL if not found. Caller must free(). */
static char *pap_var_lookup(Papagaio *ctx, const Symbols *sym,
                             const char *name, size_t nlen)
{
    if (!ctx || !name || nlen == 0) return NULL;
    size_t sl = strlen(sym->sigil);
    size_t ol = strlen(sym->open);
    size_t cl = strlen(sym->close);
    /* Pattern: $$NAME$aliases{NAME}  (same format as $from) */
    size_t pat_len = sl * 3 + nlen * 2 + 7 + ol + cl;
    char *pat = (char *)malloc(pat_len + 1);
    if (!pat) return NULL;
    char *p = pat;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, name, nlen);     p += nlen;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, "aliases", 7);   p += 7;
    memcpy(p, sym->open,  ol); p += ol;
    memcpy(p, name, nlen);     p += nlen;
    memcpy(p, sym->close, cl); p += cl;
    *p = '\0';
    char *result = NULL;
    Scope *s = ctx->current_scope;
    while (s) {
        for (int i = 0; i < s->rule_count; i++) {
            if (s->rules[i].m && strcmp(s->rules[i].m, pat) == 0) {
                result = strdup(s->rules[i].r);
                break;
            }
        }
        if (result) break;
        s = s->parent;
    }
    free(pat);
    return result;
}


static void push_scope(Papagaio *ctx) {
    Scope *s = (Scope *)malloc(sizeof(Scope));
    s->rules = NULL;
    s->rule_count = 0;
    s->rule_cap = 0;
    s->parent = ctx->current_scope;
    ctx->current_scope = s;
}

static void clear_scope(Scope *s) {
    if (s && s->rules) {
        for (int i = 0; i < s->rule_count; i++) {
            free(s->rules[i].m);
            free(s->rules[i].r);
        }
        free(s->rules);
        s->rules = NULL;
        s->rule_count = 0;
        s->rule_cap = 0;
    }
}

static void pop_scope(Papagaio *ctx) {
    if (!ctx->current_scope) return;
    Scope *s = ctx->current_scope;
    clear_scope(s);
    ctx->current_scope = s->parent;
    free(s);
}

/* Create or update a $NAME variable in ctx->current_scope->rules (same logic as $from). */
static void pap_var_update(Papagaio *ctx, const Symbols *sym,
                            const char *name, size_t nlen,
                            const char *new_value)
{
    if (!ctx || !name || nlen == 0 || !new_value) return;
    size_t sl = strlen(sym->sigil);
    size_t ol = strlen(sym->open);
    size_t cl = strlen(sym->close);
    size_t pat_len = sl * 3 + nlen * 2 + 7 + ol + cl;
    char *pat_str = (char *)malloc(pat_len + 1);
    if (!pat_str) return;
    char *p = pat_str;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, name, nlen);     p += nlen;
    memcpy(p, sym->sigil, sl); p += sl;
    memcpy(p, "aliases", 7);   p += 7;
    memcpy(p, sym->open,  ol); p += ol;
    memcpy(p, name, nlen);     p += nlen;
    memcpy(p, sym->close, cl); p += cl;
    *p = '\0';
Scope *found_scope = NULL;
    int found_idx = -1;
    for (Scope *s = ctx->current_scope; s; s = s->parent) {
        for (int ri = 0; ri < s->rule_count; ri++) {
            if (s->rules[ri].m && strcmp(s->rules[ri].m, pat_str) == 0) {
                found_scope = s;
                found_idx = ri;
                break;
            }
        }
        if (found_scope) break;
    }

    if (found_scope) {
        free(found_scope->rules[found_idx].r);
        found_scope->rules[found_idx].r = strdup(new_value);
        free(pat_str);
    } else {
        if (ctx->current_scope->rule_count + 1 > ctx->current_scope->rule_cap) {
            ctx->current_scope->rule_cap = ctx->current_scope->rule_cap ? ctx->current_scope->rule_cap * 2 : 8;
            ctx->current_scope->rules = (PatternPair *)realloc(ctx->current_scope->rules,
                          sizeof(PatternPair) * ctx->current_scope->rule_cap);
        }
        ctx->current_scope->rules[ctx->current_scope->rule_count].m = pat_str;
        ctx->current_scope->rules[ctx->current_scope->rule_count].r = strdup(new_value);
        ctx->current_scope->rule_count++;
    }
}

/* Split str by sep (multi-char). Returns malloc'd array of strdup'd strings.
   Empty string or NULL str → returns NULL with *out_count = 0. */
static char **pap_list_split(const char *str, const char *sep,
                              size_t seplen, int *out_count)
{
    *out_count = 0;
    if (!str || str[0] == '\0') return NULL;
    if (seplen == 0) {
        /* Empty separator: split char by char */
        int n = (int)strlen(str);
        char **arr = (char **)malloc(sizeof(char *) * (size_t)n);
        for (int ci = 0; ci < n; ci++) {
            arr[ci] = (char *)malloc(2);
            arr[ci][0] = str[ci];
            arr[ci][1] = '\0';
        }
        *out_count = n;
        return arr;
    }
    size_t len = strlen(str);
    int count = 1;
    for (size_t i = 0; i + seplen <= len; ) {
        if (memcmp(str + i, sep, seplen) == 0) { count++; i += seplen; }
        else i++;
    }
    char **arr = (char **)malloc(sizeof(char *) * count);
    int idx = 0;
    const char *start = str;
    for (size_t i = 0; i <= len; ) {
        int at_sep = (i + seplen <= len && memcmp(str + i, sep, seplen) == 0);
        if (i == len || at_sep) {
            size_t elen = (size_t)((str + i) - start);
            arr[idx] = (char *)malloc(elen + 1);
            memcpy(arr[idx], start, elen);
            arr[idx][elen] = '\0';
            idx++;
            if (i == len) break;
            i += seplen;
            start = str + i;
        } else i++;
    }
    *out_count = count;
    return arr;
}

/* Join parts array with sep into a newly malloc'd string. */
static char *pap_list_join(char **parts, int count,
                            const char *sep, size_t seplen)
{
    if (!parts || count == 0) return strdup("");
    size_t total = 0;
    for (int i = 0; i < count; i++) total += strlen(parts[i]);
    if (count > 1) total += seplen * (size_t)(count - 1);
    char *result = (char *)malloc(total + 1);
    char *p = result;
    for (int i = 0; i < count; i++) {
        size_t l = strlen(parts[i]);
        memcpy(p, parts[i], l); p += l;
        if (i + 1 < count && seplen > 0) { memcpy(p, sep, seplen); p += seplen; }
    }
    *p = '\0';
    return result;
}

/* Free a split array. */
static void pap_list_free(char **parts, int count)
{
    if (!parts) return;
    for (int i = 0; i < count; i++) free(parts[i]);
    free(parts);
}

/* Resolve an index string (possibly negative) against count.
   Returns -1 when out of range. */
static int pap_list_normalize_idx(const char *idx_str, int count)
{
    if (!idx_str || !idx_str[0] || count == 0) return -1;
    int idx = atoi(idx_str);
    if (idx < 0) idx = count + idx;
    if (idx < 0 || idx >= count) return -1;
    return idx;
}

/* Helper: process a StrView as text and return malloc'd result (never NULL). */
static char *pap_process_sv(Papagaio *ctx, StrView sv)
{
    char *tmp = (char *)malloc(sv.len + 1);
    memcpy(tmp, sv.ptr, sv.len); tmp[sv.len] = '\0';
    ctx->disable_patterns++;
    char *out = papagaio_process_text(ctx, tmp, sv.len);
    ctx->disable_patterns--;
    free(tmp);
    return out ? out : strdup("");
}

/* Central dispatcher for $VAR$list$OP{sep}{...} operations.
   raw_blocks[0] = sep block, raw_blocks[1..] = extra argument blocks.
   Emitting ops write to sb_out (may be NULL for pure-mutating ops).
   Mutating ops update ctx->current_scope->rules via pap_var_update. */
static void pap_list_op(Papagaio *ctx, const Symbols *sym,
                         StrBuf *sb_out,
                         const char *name, size_t nlen,
                         const char *sep_str, size_t seplen,
                         const char *op,   size_t oplen,
                         StrView *raw_blocks, int block_count)
{
    /* --- Look up current variable value --- */
    char *var_val = pap_var_lookup(ctx, sym, name, nlen);
    if (!var_val) var_val = strdup("");

    /* --- Split into parts --- */
    int count = 0;
    char **parts = pap_list_split(var_val, sep_str, seplen, &count);
    free(var_val);

    int mutated = 0;

#define OP_IS(s) (oplen == sizeof(s)-1 && memcmp(op, s, sizeof(s)-1) == 0)

    /* get: emit element at index */
    if (OP_IS("get")) {
        if (block_count >= 1 && sb_out) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int idx = pap_list_normalize_idx(idx_str, count);
            if (idx >= 0) sb_append_n(sb_out, parts[idx], strlen(parts[idx]));
            free(idx_str);
        }
    }
    /* count: emit number of elements */
    else if (OP_IS("count")) {
        if (sb_out) {
            char nbuf[32];
            snprintf(nbuf, sizeof(nbuf), "%d", count);
            sb_append_n(sb_out, nbuf, strlen(nbuf));
        }
    }
    /* set: replace element at index */
    else if (OP_IS("set")) {
        if (block_count >= 2) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int idx = pap_list_normalize_idx(idx_str, count);
            free(idx_str);
            if (idx >= 0) {
                char *content = pap_process_sv(ctx, raw_blocks[1]);
                free(parts[idx]);
                parts[idx] = content;
                mutated = 1;
            }
        }
    }
    /* push: append to end */
    else if (OP_IS("push")) {
        for (int i = 0; i < block_count; i++) {
            char *content = pap_process_sv(ctx, raw_blocks[i]);
            parts = (char **)realloc(parts, sizeof(char *) * (size_t)(count + 1));
            parts[count++] = content;
            mutated = 1;
        }
    }
    /* pop: remove and emit last element */
    else if (OP_IS("pop")) {
        if (count > 0) {
            if (sb_out) sb_append_n(sb_out, parts[count-1], strlen(parts[count-1]));
            free(parts[count-1]);
            count--;
            mutated = 1;
        }
    }
    /* shift: remove and emit first element */
    else if (OP_IS("shift")) {
        if (count > 0) {
            if (sb_out) sb_append_n(sb_out, parts[0], strlen(parts[0]));
            free(parts[0]);
            if (count > 1) {
                memmove(parts, parts + 1, sizeof(char *) * (size_t)(count - 1));
            }
            count--;
            mutated = 1;
        }
    }
    /* unshift: prepend to front */
    else if (OP_IS("unshift")) {
        for (int i = block_count - 1; i >= 0; i--) {
            char *content = pap_process_sv(ctx, raw_blocks[i]);
            parts = (char **)realloc(parts, sizeof(char *) * (size_t)(count + 1));
            memmove(parts + 1, parts, sizeof(char *) * (size_t)count);
            parts[0] = content;
            count++;
            mutated = 1;
        }
    }
    /* insert: insert before index */
    else if (OP_IS("insert")) {
        if (block_count >= 2) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int raw_idx = atoi(idx_str); free(idx_str);
            if (raw_idx < 0) raw_idx = count + raw_idx;
            if (raw_idx < 0) raw_idx = 0;
            if (raw_idx > count) raw_idx = count;
            char *content = pap_process_sv(ctx, raw_blocks[1]);
            parts = (char **)realloc(parts, sizeof(char *) * (size_t)(count + 1));
            memmove(parts + raw_idx + 1, parts + raw_idx,
                    sizeof(char *) * (size_t)(count - raw_idx));
            parts[raw_idx] = content;
            count++;
            mutated = 1;
        }
    }
    /* remove: delete element at index */
    else if (OP_IS("remove")) {
        if (block_count >= 1 && count > 0) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[0]);
            int idx = pap_list_normalize_idx(idx_str, count);
            free(idx_str);
            if (idx >= 0) {
                free(parts[idx]);
                if (idx < count - 1) {
                    memmove(parts + idx, parts + idx + 1,
                            sizeof(char *) * (size_t)(count - idx - 1));
                }
                count--;
                mutated = 1;
            }
        }
    }
    /* swap: exchange two elements */
    else if (OP_IS("swap")) {
        if (block_count >= 2 && count > 1) {
            char *ia_str = pap_process_sv(ctx, raw_blocks[0]);
            char *ib_str = pap_process_sv(ctx, raw_blocks[1]);
            int ia = pap_list_normalize_idx(ia_str, count);
            int ib = pap_list_normalize_idx(ib_str, count);
            free(ia_str); free(ib_str);
            if (ia >= 0 && ib >= 0 && ia != ib) {
                char *tmp = parts[ia]; parts[ia] = parts[ib]; parts[ib] = tmp;
                mutated = 1;
            }
        }
    }
    /* reverse: invert element order */
    else if (OP_IS("reverse")) {
        if (count > 1) {
            for (int lo = 0, hi = count - 1; lo < hi; lo++, hi--) {
                char *tmp = parts[lo]; parts[lo] = parts[hi]; parts[hi] = tmp;
            }
            mutated = 1;
        }
    }
    /* join: emit list with a different separator (non-mutating) */
    else if (OP_IS("join")) {
        if (block_count >= 1 && sb_out) {
            char *new_sep = pap_process_sv(ctx, raw_blocks[0]);
            char *joined  = pap_list_join(parts, count, new_sep, strlen(new_sep));
            sb_append_n(sb_out, joined, strlen(joined));
            free(joined); free(new_sep);
        }
    }
    /* find: find pattern in list elements and emit WHOLE element */
    else if (OP_IS("find")) {
        if (block_count >= 1 && sb_out) {
            char *pat_str = pap_process_sv(ctx, raw_blocks[0]);
            Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
            for (int i = 0; i < count; i++) {
                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                for (int s = 0; parts[i][s]; s++) {
                    if (match_pattern(ctx, parts[i], (int)strlen(parts[i]), &p, s, &m)) {
                        sb_append_n(sb_out, parts[i], strlen(parts[i]));
                        free_match(&m);
                        free_pattern(&p); free(pat_str);
                        goto done;
                    }
                }
            }
            free_pattern(&p); free(pat_str);
        }
    }
    /* contains: find pattern and emit start index WITHIN the element */
    else if (OP_IS("contains")) {
        if (block_count >= 1 && sb_out) {
            char *pat_str = pap_process_sv(ctx, raw_blocks[0]);
            Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
            for (int i = 0; i < count; i++) {
                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                for (int s = 0; parts[i][s]; s++) {
                    if (match_pattern(ctx, parts[i], (int)strlen(parts[i]), &p, s, &m)) {
                        char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "%d", m.start);
                        sb_append_n(sb_out, nbuf, strlen(nbuf));
                        free_match(&m);
                        free_pattern(&p); free(pat_str);
                        goto done;
                    }
                }
            }
            free_pattern(&p); free(pat_str);
        }
    }
    /* replace: find pattern in elements, replace it, emit old match, update var */
    else if (OP_IS("replace")) {
        if (block_count >= 2) {
            char *pat_str = pap_process_sv(ctx, raw_blocks[0]);
            char *rep_str = pap_process_sv(ctx, raw_blocks[1]);
            Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
            for (int i = 0; i < count; i++) {
                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                for (int s = 0; parts[i][s]; s++) {
                    if (match_pattern(ctx, parts[i], (int)strlen(parts[i]), &p, s, &m)) {
                        if (sb_out) sb_append_n(sb_out, parts[i] + m.start, (size_t)(m.end - m.start));
                        char *r = apply_replacement_ex(rep_str, &m, sym);
                        size_t rl = strlen(r);
                        size_t cl = strlen(parts[i]);
                        size_t new_len = (size_t)m.start + rl + (cl - (size_t)m.end);
                        char *new_parts_i = (char *)malloc(new_len + 1);
                        memcpy(new_parts_i, parts[i], (size_t)m.start);
                        memcpy(new_parts_i + m.start, r, rl);
                        strcpy(new_parts_i + m.start + rl, parts[i] + m.end);
                        free(parts[i]); parts[i] = new_parts_i;
                        mutated = 1;
                        free(r); free_match(&m);
                        free_pattern(&p); free(pat_str); free(rep_str);
                        goto done;
                    }
                }
            }
            free_pattern(&p); free(pat_str); free(rep_str);
        }
    }

    /* slice: returns sub-list from start to end */
    else if (OP_IS("slice")) {
        if (block_count >= 1 && sb_out) {
            char *s1 = pap_process_sv(ctx, raw_blocks[0]);
            int start = atoi(s1); free(s1);
            int end = count;
            if (block_count >= 2) {
                char *s2 = pap_process_sv(ctx, raw_blocks[1]);
                end = atoi(s2); free(s2);
            }
            if (start < 0) start = count + start;
            if (end < 0) end = count + end;
            if (start < 0) start = 0;
            if (end > count) end = count;
            if (start < end) {
                char *res = pap_list_join(parts + start, end - start, sep_str, seplen);
                sb_append_n(sb_out, res, strlen(res));
                free(res);
            }
        }
    }

done:
    /* Persist mutation back to variable */
    if (mutated) {
        char *new_val = pap_list_join(parts, count, sep_str, seplen);
        pap_var_update(ctx, sym, name, nlen, new_val);
        free(new_val);
    }

    pap_list_free(parts, count);
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
            Match m; m.ctx = ctx;
            if (match_pattern(ctx, work, len, &rules[i].pattern, pos, &m)) {
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
    ctx->cmd_count++;
    return 0;
}

void *papagaio_get_command_userdata(Papagaio *ctx, const char *name)
{
    if (!ctx || !name) return NULL;
    for (int i = 0; i < ctx->cmd_count; i++) {
        if (strcmp(ctx->commands[i].name, name) == 0) {
            return ctx->commands[i].userdata;
        }
    }
    return NULL;
}



void papagaio_clear_commands(Papagaio *ctx)
{
    if (!ctx) return;
    for (int i = 0; i < ctx->cmd_count; i++) {
        if (ctx->commands[i].name) {
            free(ctx->commands[i].name);
            ctx->commands[i].name = NULL;
        }
    }
    ctx->cmd_count = 0;
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

void papagaio_add_finalizer(Papagaio *ctx, PapFinalizer fn, void *userdata) {
    if (!ctx || !fn) return;
    if (ctx->fin_count >= ctx->fin_cap) {
        ctx->fin_cap = ctx->fin_cap ? ctx->fin_cap * 2 : 4;
        ctx->finalizers = realloc(ctx->finalizers, ctx->fin_cap * sizeof(RegisteredFinalizer));
    }
    RegisteredFinalizer f;
    f.fn = fn;
    f.userdata = userdata;
    ctx->finalizers[ctx->fin_count++] = f;
}

/* =========================================================================
 * Public C API
 * ====================================================================== */

/* --- Standard Language Commands --- */

static char *cmd_replace(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *ud) {
    (void)name; (void)ud;
    if (!piped_val || argc < 2) return strdup(piped_val ? piped_val : "");
    char *pat_str = papagaio_process_text(ctx, argv[0], argl[0]);
    char *rep_str = papagaio_process_text(ctx, argv[1], argl[1]);
    Symbols sym = make_symbols("$", "{", "}");
    Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, &sym);
    Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
    char *result = strdup(piped_val);
    for (int s = 0; result[s]; s++) {
        if (match_pattern(ctx, result, (int)strlen(result), &p, s, &m)) {
            char *r = apply_replacement_ex(rep_str, &m, &sym);
            size_t rl = strlen(r), cl = strlen(result);
            size_t new_len = (size_t)m.start + rl + (cl - (size_t)m.end);
            char *new_val = (char *)malloc(new_len + 1);
            memcpy(new_val, result, (size_t)m.start);
            memcpy(new_val + m.start, r, rl);
            strcpy(new_val + m.start + rl, result + m.end);
            free(result); result = new_val;
            free(r); free_match(&m);
            break;
        }
    }
    free_pattern(&p); free(pat_str); free(rep_str);
    return result;
}

static char *cmd_slice(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *ud) {
    (void)name; (void)ud;
    if (!piped_val || argc < 1) return strdup(piped_val ? piped_val : "");
    char *s1 = papagaio_process_text(ctx, argv[0], argl[0]);
    int start = atoi(s1); free(s1);
    int end = (int)strlen(piped_val);
    if (argc >= 2) {
        char *s2 = papagaio_process_text(ctx, argv[1], argl[1]);
        end = atoi(s2); free(s2);
    }
    int len_cv = (int)strlen(piped_val);
    if (start < 0) start = len_cv + start;
    if (end < 0) end = len_cv + end;
    if (start < 0) start = 0;
    if (end > len_cv) end = len_cv;
    if (start < end) {
        char *slice_res = (char*)malloc((size_t)(end - start + 1));
        memcpy(slice_res, piped_val + start, (size_t)(end - start));
        slice_res[end - start] = '\0';
        return slice_res;
    }
    return strdup("");
}

static char *cmd_find(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *ud) {
    (void)name; (void)ud;
    if (!piped_val || argc < 1) return strdup("");
    char *pat_str = papagaio_process_text(ctx, argv[0], argl[0]);
    Symbols sym = make_symbols("$", "{", "}");
    Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, &sym);
    Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
    char *match_res = strdup("");
    for (int s = 0; piped_val[s]; s++) {
        if (match_pattern(ctx, piped_val, (int)strlen(piped_val), &p, s, &m)) {
            free(match_res);
            match_res = (char*)malloc((size_t)(m.end - m.start + 1));
            memcpy(match_res, piped_val + m.start, (size_t)(m.end - m.start));
            match_res[m.end - m.start] = '\0';
            free_match(&m);
            break;
        }
    }
    free_pattern(&p); free(pat_str);
    return match_res;
}

static char *cmd_contains(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *ud) {
    (void)name; (void)ud;
    if (!piped_val || argc < 1) return strdup("");
    char *pat_str = papagaio_process_text(ctx, argv[0], argl[0]);
    Symbols sym = make_symbols("$", "{", "}");
    Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, &sym);
    Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
    char *idx_res = strdup("");
    for (int s = 0; piped_val[s]; s++) {
        if (match_pattern(ctx, piped_val, (int)strlen(piped_val), &p, s, &m)) {
            free(idx_res);
            char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "%d", m.start);
            idx_res = strdup(nbuf);
            free_match(&m);
            break;
        }
    }
    free_pattern(&p); free(pat_str);
    return idx_res;
}

static char *cmd_byte(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *ud) {
    (void)name; (void)ud;
    if (argc < 1) return strdup(piped_val ? piped_val : "");
    char *code_str = papagaio_process_text(ctx, argv[0], argl[0]);
    int code = atoi(code_str); free(code_str);
    
    size_t cl = piped_val ? strlen(piped_val) : 0;
    char *nv = (char *)malloc(cl + 2);
    if (nv) {
        if (piped_val && cl > 0) memcpy(nv, piped_val, cl);
        nv[cl] = (char)code;
        nv[cl + 1] = '\0';
        return nv;
    }
    return strdup(piped_val ? piped_val : "");
}

static void papagaio_register_std_commands(Papagaio *ctx) {
    papagaio_register_command(ctx, "replace", cmd_replace, NULL);
    papagaio_register_command(ctx, "slice", cmd_slice, NULL);
    papagaio_register_command(ctx, "find", cmd_find, NULL);
    papagaio_register_command(ctx, "contains", cmd_contains, NULL);
    papagaio_register_command(ctx, "byte", cmd_byte, NULL);
}

Papagaio *papagaio_open(void)
{
    Papagaio *ctx = (Papagaio *)malloc(sizeof(Papagaio));
    if (!ctx) return NULL;

    ctx->commands   = NULL; ctx->cmd_count = 0; ctx->cmd_cap = 0;
    ctx->modifiers  = NULL; ctx->mod_count = 0; ctx->mod_cap = 0;
    ctx->finalizers = NULL; ctx->fin_count = 0; ctx->fin_cap = 0;
    ctx->argc       = 0;    ctx->argv      = NULL;
    ctx->auto_export = 1;
    ctx->global_scope = (Scope *)malloc(sizeof(Scope));
    ctx->global_scope->rules = NULL;
    ctx->global_scope->rule_count = 0;
    ctx->global_scope->rule_cap = 0;
    ctx->global_scope->parent = NULL;
    ctx->current_scope = ctx->global_scope;
    ctx->depth = 0;
    ctx->disable_sandbox = 0;
    ctx->disable_patterns = 0;
    ctx->once_mode = 0;
    ctx->original_doc = NULL; ctx->original_len = 0;
    ctx->dl_handles = NULL; ctx->dl_count = 0; ctx->dl_cap = 0;

    papagaio_register_command(ctx, "include", include_handler, NULL);
    papagaio_register_std_commands(ctx);
    return ctx;
}

void papagaio_close(Papagaio *ctx)
{
    if (!ctx) return;
    for (int i = 0; i < ctx->fin_count; i++) {
        if (ctx->finalizers[i].fn)
            ctx->finalizers[i].fn(ctx->finalizers[i].userdata);
    }
    if (ctx->commands) {
        for (int i = 0; i < ctx->cmd_count; i++) {
            free((void*)ctx->commands[i].name);
        }
        free(ctx->commands);
    }
    if (ctx->modifiers) {
        for (int i = 0; i < ctx->mod_count; i++) {
            free((void*)ctx->modifiers[i].name);
        }
        free(ctx->modifiers);
    }
    free(ctx->finalizers);
    while (ctx->current_scope) {
        Scope *parent = ctx->current_scope->parent;
        clear_scope(ctx->current_scope);
        free(ctx->current_scope);
        ctx->current_scope = parent;
    }
    if (ctx->original_doc) free(ctx->original_doc);
#if defined(PAPAGAIO_USE_WINDOWS_DL)
    if (ctx->dl_handles) {
        for (int i = 0; i < ctx->dl_count; i++) {
            if (ctx->dl_handles[i]) FreeLibrary((HMODULE)ctx->dl_handles[i]);
        }
        free(ctx->dl_handles);
    }
#elif defined(PAPAGAIO_USE_POSIX_DL)
    if (ctx->dl_handles) {
        for (int i = 0; i < ctx->dl_count; i++) {
            if (ctx->dl_handles[i]) dlclose(ctx->dl_handles[i]);
        }
        free(ctx->dl_handles);
    }
#endif
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

void papagaio_set_cli_mode(Papagaio *ctx, int enabled)
{
    if (!ctx) return;
    ctx->cli_mode = enabled;
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
            Match m; m.ctx = ctx;
            if (match_pattern(ctx, input, len, &rules[i].pattern, pos, &m)) {
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

static char *include_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, const char *piped_val, void *ud) {
    (void)ctx; (void)name; (void)ud; (void)argl; (void)piped_val;
    if (argc < 1) return strdup("");
    
    char trim_path[256]; size_t pl = strlen(argv[0]);
    size_t start = 0; while(start < pl && isspace((unsigned char)argv[0][start])) start++;
    size_t end = pl; while(end > start && isspace((unsigned char)argv[0][end-1])) end--;
    size_t len = end - start; if (len >= 255) len = 255;
    memcpy(trim_path, argv[0] + start, len); trim_path[len] = '\0';
    
    FILE *f = fopen(trim_path, "rb");
    if (!f) return strdup("");
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (buf) {
        size_t rb = fread(buf, 1, sz, f);
        buf[rb] = '\0';
    } else {
        buf = strdup("");
    }
    fclose(f);
    return buf;
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

            if (klen == 7 && memcmp(src + ks, "pattern", 7) == 0) {
                /* Preprocess match block, skip/copy raw replacement block */
                size_t j_pat = j;
                while (j_pat < len && isspace((unsigned char)src[j_pat])) j_pat++;
                StrView so = { sym->open, strlen(sym->open) };
                StrView sc = { sym->close, strlen(sym->close) };
                if (j_pat < len && str_pfx(src + j_pat, sym->open)) {
                    StrView match_blk;
                    int next1 = extract_block(src, (int)j_pat, so, sc, &match_blk);
                    size_t k_pat = (size_t)next1;
                    while (k_pat < len && isspace((unsigned char)src[k_pat])) k_pat++;
                    if (k_pat < len && str_pfx(src + k_pat, sym->open)) {
                        StrView repl_blk;
                        int next2 = extract_block(src, (int)k_pat, so, sc, &repl_blk);
                        
                        /* Preprocess the match block recursively (preprocessor only, no rules) */
                        char *match_str = (char *)malloc(match_blk.len + 1);
                        memcpy(match_str, match_blk.ptr, match_blk.len);
                        match_str[match_blk.len] = '\0';
                        char *proc_match = resolve_preprocessor(ctx, match_str, sym);
                        free(match_str);
                        
                        /* Output: sigil + "pattern" + whitespace + open + proc_match + close + anything between blocks + second block raw */
                        sb_append_n(&out, src + i, j_pat - i);
                        sb_append_n(&out, sym->open, strlen(sym->open));
                        sb_append_n(&out, proc_match, strlen(proc_match));
                        sb_append_n(&out, sym->close, strlen(sym->close));
                        free(proc_match);
                        
                        sb_append_n(&out, src + next1, (size_t)next2 - (size_t)next1);
                        i = (size_t)next2;
                        continue;
                    }
                }
            }

            /* Handle ${value}$then/$else/$compare -- braced expression as flow input */
            if (klen == 0 && j < len && str_pfx(src + j, sym->open)) {
                StrView so_b = { sym->open,  strlen(sym->open)  };
                StrView sc_b = { sym->close, strlen(sym->close) };
                StrView blk_b;
                size_t jb = (size_t)extract_block(src, (int)j, so_b, sc_b, &blk_b);
                /* Check if followed by a flow op */
                if (jb < len && src[jb] == '$') {
                    size_t nj = jb + 1, njs = nj;
                    while (nj < len && (isalnum((unsigned char)src[nj]) || src[nj] == '_')) nj++;
                    size_t nfl = nj - njs;
                    if ((nfl == 7 && memcmp(src + njs, "compare",  7) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "then",     4) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "else",     4) == 0) ||
                        (nfl == 6 && memcmp(src + njs, "repeat",   6) == 0) ||
                        (nfl == 5 && memcmp(src + njs, "while",    5) == 0) ||
                        (nfl == 5 && memcmp(src + njs, "until",    5) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "byte",     4) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "find",     4) == 0) ||
                        (nfl == 8 && memcmp(src + njs, "includes", 8) == 0) ||
                        (nfl == 7 && memcmp(src + njs, "replace",  7) == 0)) {
                        /* Process block content as initial value */
                        char *cur_val = pap_process_sv(ctx, blk_b);
                        size_t cp = jb;
                        StrView so_f = { sym->open,  strlen(sym->open)  };
                        StrView sc_f = { sym->close, strlen(sym->close) };
                        while (cp < len && src[cp] == '$') {
                            size_t j2 = cp + 1, ops = j2;
                            while (j2 < len && (isalnum((unsigned char)src[j2]) || src[j2] == '_')) j2++;
                            size_t opl = j2 - ops;
                            int is_cmp      = (opl == 7 && memcmp(src + ops, "compare",  7) == 0);
                            int is_then     = (opl == 4 && memcmp(src + ops, "then",     4) == 0);
                            int is_else     = (opl == 4 && memcmp(src + ops, "else",     4) == 0);
                            int is_repeat   = (opl == 6 && memcmp(src + ops, "repeat",   6) == 0);
                            int is_while    = (opl == 5 && memcmp(src + ops, "while",    5) == 0);
                            int is_until    = (opl == 5 && memcmp(src + ops, "until",    5) == 0);
                            int is_byte     = (opl == 4 && memcmp(src + ops, "byte",     4) == 0);
                            int is_find     = (opl == 4 && memcmp(src + ops, "find",     4) == 0);
                            int is_includes = (opl == 8 && memcmp(src + ops, "includes", 8) == 0);
                            int is_replace  = (opl == 7 && memcmp(src + ops, "replace",  7) == 0);
                            
                            if (!is_cmp && !is_then && !is_else && !is_repeat && !is_while && 
                                !is_until && !is_byte && !is_find && !is_includes && !is_replace) break;

                            size_t j3 = j2;
                            while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                            if (j3 >= len || !str_pfx(src + j3, sym->open)) break;
                            StrView blk; j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &blk);
                            char *arg = (is_repeat || is_while || is_until || is_find || is_includes || is_replace) ? NULL : pap_process_sv(ctx, blk);

                            if (is_cmp) {
                                if (strcmp(cur_val, arg) != 0) { free(cur_val); cur_val = strdup(""); }
                            } else if (is_then) {
                                if (cur_val[0] != '\0') { free(cur_val); cur_val = arg; arg = NULL; }
                                else { free(cur_val); cur_val = strdup(""); }
                            } else if (is_else) {
                                if (cur_val[0] == '\0') { free(cur_val); cur_val = arg; arg = NULL; }
                            } else if (is_byte) {
                                int code = atoi(arg);
                                char b[2] = {(char)code, '\0'};
                                free(cur_val); cur_val = strdup(b);
                            } else if (is_find) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                char *match_res = strdup("");
                                for (int s = 0; cur_val[s]; s++) {
                                    if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                        free(match_res);
                                        match_res = (char*)malloc((size_t)(m.end - m.start + 1));
                                        memcpy(match_res, cur_val + m.start, (size_t)(m.end - m.start));
                                        match_res[m.end - m.start] = '\0';
                                        free_match(&m);
                                        break;
                                    }
                                }
                                free(cur_val); cur_val = match_res;
                                free_pattern(&p); free(pat_str);
                            } else if (is_includes) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                char *idx_res = strdup("");
                                for (int s = 0; cur_val[s]; s++) {
                                    if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                        free(idx_res);
                                        char nbuf[32]; snprintf(nbuf, sizeof(nbuf), "%d", m.start);
                                        idx_res = strdup(nbuf);
                                        free_match(&m);
                                        break;
                                    }
                                }
                                free(cur_val); cur_val = idx_res;
                                free_pattern(&p); free(pat_str);
                            } else if (is_replace) {
                                char *pat_str = pap_process_sv(ctx, blk);
                                while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView rep_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &rep_blk);
                                    char *rep_str = pap_process_sv(ctx, rep_blk);
                                    Pattern p; memset(&p, 0, sizeof(p)); parse_pattern_ex(pat_str, &p, sym);
                                    Match m; memset(&m, 0, sizeof(m)); m.ctx = ctx;
                                    int replaced = 0;
                                    for (int s = 0; cur_val[s]; s++) {
                                        if (match_pattern(ctx, cur_val, (int)strlen(cur_val), &p, s, &m)) {
                                            char *old_match = (char*)malloc((size_t)(m.end - m.start + 1));
                                            memcpy(old_match, cur_val + m.start, (size_t)(m.end - m.start));
                                            old_match[m.end - m.start] = '\0';
                                            char *r = apply_replacement_ex(rep_str, &m, sym);
                                            size_t rl = strlen(r), cl = strlen(cur_val);
                                            size_t new_len = (size_t)m.start + rl + (cl - (size_t)m.end);
                                            char *new_val = (char *)malloc(new_len + 1);
                                            memcpy(new_val, cur_val, (size_t)m.start);
                                            memcpy(new_val + m.start, r, rl);
                                            strcpy(new_val + m.start + rl, cur_val + m.end);
                                            
                                            /* Persistent update if applicable */
                                            if (klen > 0) pap_var_update(ctx, sym, src + ks, klen, new_val);
                                            
                                            free(cur_val); cur_val = old_match;
                                            free(new_val); free(r); free_match(&m);
                                            replaced = 1; break;
                                        }
                                    }
                                    if (!replaced) {
                                        free(cur_val); cur_val = strdup("");
                                    }
                                    free_pattern(&p); free(rep_str);
                                }
                                free(pat_str);
                            } else if (is_repeat) {
                                char *times_str = pap_process_sv(ctx, blk);
                                int times = atoi(times_str); free(times_str);
                                while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    for (int t = 0; t < times; t++) {
                                        char *tmp = pap_process_sv(ctx, code_blk);
                                        free(tmp);
                                    }
                                }
                                free(cur_val); cur_val = strdup("");
                            } else if (is_while || is_until) {
                                StrView pat_blk = blk;
                                char *pat_str = pap_process_sv(ctx, pat_blk);
                                Pattern pat; parse_pattern_ex(pat_str, &pat, sym);
                                free(pat_str);
                                
                                while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    StrBuf comb; sb_init(&comb);
                                    char *last_res = strdup("");
                                    while (1) {
                                        char *iter = pap_process_sv(ctx, code_blk);
                                        Match m; m.ctx = ctx;
                                        int matches = match_pattern(ctx, iter, (int)strlen(iter), &pat, 0, &m);
                                        if (matches) free_match(&m);
                                        
                                        if (is_while) {
                                            if (!matches) { free(iter); break; }
                                            free(last_res); last_res = iter;
                                        } else { /* until */
                                            sb_append_n(&comb, iter, strlen(iter));
                                            if (matches) { free(iter); break; }
                                            free(iter);
                                        }
                                    }
                                    free(cur_val);
                                    if (is_while) { cur_val = last_res; sb_free(&comb); }
                                    else { cur_val = strdup(comb.data); sb_free(&comb); free(last_res); }
                                }
                                free_pattern(&pat);
                            }
                            if (arg) free(arg);
                            cp = j3;
                        }
                        sb_append_n(&out, cur_val, strlen(cur_val));
                        free(cur_val);
                        i = cp;
                        goto next_char;
                    }
                }
            }

            if (klen > 0) {
                /* 1. Dynamic Symbol Operators */
                if (klen == 5 && memcmp(src + ks, "sigil", 5) == 0) {
                    sb_append_n(&out, sym->sigil, strlen(sym->sigil));
                    if (j < len && src[j] == '$') { j++; while (j < len && isspace((unsigned char)src[j])) j++; }
                    i = j; continue;
                }
                if (klen == 4 && memcmp(src + ks, "open", 4) == 0) {
                    sb_append_n(&out, sym->open, strlen(sym->open));
                    if (j < len && src[j] == '$') { j++; while (j < len && isspace((unsigned char)src[j])) j++; }
                    i = j; continue;
                }
                if (klen == 5 && memcmp(src + ks, "close", 5) == 0) {
                    sb_append_n(&out, sym->close, strlen(sym->close));
                    if (j < len && src[j] == '$') { j++; while (j < len && isspace((unsigned char)src[j])) j++; }
                    i = j; continue;
                }
                if (klen == 6 && memcmp(src + ks, "marker", 6) == 0) {
                    sb_append_n(&out, sym->optional, strlen(sym->optional));
                    if (j < len && src[j] == '$') { j++; while (j < len && isspace((unsigned char)src[j])) j++; }
                    i = j; continue;
                }

                /* 2. Whitespace and Special Characters */
                if (klen == 5 && memcmp(src + ks, "space", 5) == 0) {
                    sb_append_char(&out, ' ');
                    if (j < len && src[j] == '$') { j++; while (j < len && isspace((unsigned char)src[j])) j++; }
                    i = j; continue;
                }
                if (klen == 7 && memcmp(src + ks, "newline", 7) == 0) {
                    sb_append_char(&out, '\n');
                    if (j < len && src[j] == '$') { j++; while (j < len && isspace((unsigned char)src[j])) j++; }
                    i = j; continue;
                }
                if (klen == 3 && memcmp(src + ks, "tab", 3) == 0) {
                    sb_append_char(&out, '\t');
                    if (j < len && src[j] == '$') { j++; while (j < len && isspace((unsigned char)src[j])) j++; }
                    i = j; continue;
                }
                if (klen == 5 && memcmp(src + ks, "ascii", 5) == 0) {
                    if (j < len && src[j] == '$') {
                        size_t ns = j + 1, ne = ns;
                        while (ne < len && isdigit((unsigned char)src[ne])) ne++;
                        if (ne > ns) {
                            int code = atoi(src + ns);
                            sb_append_char(&out, (char)code);
                            i = ne; continue;
                        }
                    } else {
                        while(j < len && isspace((unsigned char)src[j])) j++;
                        if (j < len && str_pfx(src + j, sym->open)) {
                            StrView blk;
                            int next = extract_block(src, (int)j, (StrView){sym->open, strlen(sym->open)}, (StrView){sym->close, strlen(sym->close)}, &blk);
                            char *arg = pap_process_sv(ctx, blk);
                            int code = atoi(arg);
                            free(arg);
                            sb_append_char(&out, (char)code);
                            i = (size_t)next; continue;
                        }
                    }
                }
                /* Check for $NAME$from{...} assignment syntax */
                if (j < len && src[j] == '$') {
                    size_t j2 = j + 1;
                    if (j2 + 4 <= len && memcmp(src + j2, "from", 4) == 0) {
                        size_t ks_name = ks;
                        size_t name_len = j - ks;
                        size_t j_next = j2 + 4;
                        while(j_next < len && isspace((unsigned char)src[j_next])) j_next++;
                        
                        StrView so = { sym->open, strlen(sym->open) };
                        StrView sc = { sym->close, strlen(sym->close) };
                        if (j_next < len && str_pfx(src + j_next, sym->open)) {
                            StrView val_blk;
                            j_next = (size_t)extract_block(src, (int)j_next, so, sc, &val_blk);
                            char *processed_val = papagaio_process_text(ctx, val_blk.ptr, val_blk.len);
                            if (processed_val) {
                                char var_name[256];
                                size_t len_to_copy = name_len < 255 ? name_len : 255;
                                strncpy(var_name, src + ks_name, len_to_copy);
                                var_name[len_to_copy] = '\0';
                                
                                pap_var_update(ctx, sym, var_name, name_len, processed_val);
                                free(processed_val);
                            }
                            i = j_next; continue;
                        }
                    }
                }

                /* Check for $NAME$list{sep}$OP{...} list operations */
                if (j < len && src[j] == '$') {
                    size_t j2 = j + 1;
                    size_t ms = j2;
                    while (j2 < len && (isalnum((unsigned char)src[j2]) || src[j2] == '_')) j2++;
                    size_t mlen = j2 - ms;

                    if (mlen == 4 && memcmp(src + ms, "list", 4) == 0) {
                        /* Extract separator block */
                        size_t j3 = j2;
                        StrView so = { sym->open,  strlen(sym->open)  };
                        StrView sc = { sym->close, strlen(sym->close) };
                        while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                        if (j3 < len && str_pfx(src + j3, sym->open)) {
                            StrView sep_blk;
                            j3 = (size_t)extract_block(src, (int)j3, so, sc, &sep_blk);
                            char *sep_str = pap_process_sv(ctx, sep_blk);
                            
                            /* Look for next $command */
                            while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                            if (j3 < len && src[j3] == '$') {
                                size_t ops = j3 + 1;
                                size_t j4 = ops;
                                while (j4 < len && (isalnum((unsigned char)src[j4]) || src[j4] == '_')) j4++;
                                size_t oplen = j4 - ops;
                                
                                /* Extract up to 4 argument blocks {a1}{a2}{a3}{a4} */
                                StrView blocks[4];
                                int bcount = 0;
                                size_t jb = j4;
                                while (bcount < 4) {
                                    size_t jb_saved = jb;
                                    while (jb < len && isspace((unsigned char)src[jb])) jb++;
                                    if (jb >= len || !str_pfx(src + jb, sym->open)) {
                                        jb = jb_saved; break;
                                    }
                                    jb = (size_t)extract_block(src, (int)jb, so, sc, &blocks[bcount++]);
                                }
                                
                                pap_list_op(ctx, sym, &out, src + ks, klen, sep_str, strlen(sep_str), src + ops, oplen, blocks, bcount);
                                i = jb; free(sep_str);
                                continue;
                            }
                            free(sep_str);
                        }
                    }
                }

                /* ---- Flow control: $compare{}, $then{}, $else{} ----
                 * Two entry points:
                 *   A) $NAME$compare/then/else{...}  -- NAME is a known variable
                 *   B) standalone $then{...} / $else{...} / $compare{...} -- input = ""
                 *
                 * Semantics:
                 *   compare{B}: cur==B  → keep cur; cur!=B → cur=""
                 *   then{Y}   : cur!="" → cur=process(Y); cur=="" → cur=""
                 *   else{Y}   : cur=="" → cur=process(Y); cur!="" → pass cur through
                 *
                 * Multiple ops chain: $A$compare{B}$then{C}$else{D}
                 */
                {
                    int cur_is_then     = (klen == 4 && memcmp(src + ks, "then",     4) == 0);
                    int cur_is_else     = (klen == 4 && memcmp(src + ks, "else",     4) == 0);
                    int cur_is_compare  = (klen == 7 && memcmp(src + ks, "compare",  7) == 0);
                    int cur_is_repeat   = (klen == 6 && memcmp(src + ks, "repeat",   6) == 0);
                    int cur_is_while    = (klen == 5 && memcmp(src + ks, "while",    5) == 0);
                    int cur_is_until    = (klen == 5 && memcmp(src + ks, "until",    5) == 0);
                    int cur_is_flow     = cur_is_then || cur_is_else || cur_is_compare ||
                                          cur_is_repeat || cur_is_while || cur_is_until;
                    int cur_is_cmd      = -1;
                    if (!cur_is_flow) {
                        for (int ci = 0; ci < ctx->cmd_count; ci++) {
                            if (strlen(ctx->commands[ci].name) == klen && memcmp(ctx->commands[ci].name, src + ks, klen) == 0) {
                                cur_is_cmd = ci; break;
                            }
                        }
                    }

                    /* Check if current NAME is followed by a flow op */
                    int next_is_flow = 0;
                    if (!cur_is_flow && j < len && src[j] == '$') {
                        size_t nj = j + 1, njs = nj;
                        while (nj < len && (isalnum((unsigned char)src[nj]) || src[nj] == '_')) nj++;
                        size_t nfl = nj - njs;
                        int is_flow = 0;
                        int is_conflicting = 0;
                        if ((nfl == 7 && memcmp(src + njs, "compare",  7) == 0) ||
                            (nfl == 4 && memcmp(src + njs, "then",     4) == 0) ||
                            (nfl == 4 && memcmp(src + njs, "else",     4) == 0) ||
                            (nfl == 6 && memcmp(src + njs, "repeat",   6) == 0) ||
                            (nfl == 5 && memcmp(src + njs, "while",    5) == 0) ||
                            (nfl == 5 && memcmp(src + njs, "until",    5) == 0)) {
                            is_flow = 1;
                        } else {
                            for (int ci = 0; ci < ctx->cmd_count; ci++) {
                                if (strlen(ctx->commands[ci].name) == nfl && memcmp(ctx->commands[ci].name, src + njs, nfl) == 0) {
                                    is_flow = 1; break;
                                }
                            }
                        }
                        
                        if (is_flow) {
                            if (is_conflicting) {
                                char *v = pap_var_lookup(ctx, sym, src + ks, klen);
                                if (v) { next_is_flow = 1; free(v); }
                            } else {
                                next_is_flow = 1;
                            }
                        }
                    }

                    if (cur_is_flow || cur_is_cmd >= 0 || next_is_flow) {
                        StrView so_f = { sym->open,  strlen(sym->open)  };
                        StrView sc_f = { sym->close, strlen(sym->close) };

                        /* Determine initial value and where the chain starts */
                        char *cur_val;
                        size_t cp; /* position in src where we scan flow ops */

                        if (cur_is_flow) {
                            cur_val = strdup("");
                            cp = i; /* Start from the current $ to pick up the first op */
                        } else if (cur_is_cmd >= 0) {
                            cur_val = strdup("");
                            cp = i; /* Start from the $ to process the command */
                        } else {
                            cur_val = pap_var_lookup(ctx, sym, src + ks, klen);
                            if (!cur_val) cur_val = strdup("");
                            cp = j; /* Start from the $ after the name */
                        }



                        while (cp < len && src[cp] == '$') {
                            size_t j2 = cp + 1, ops = j2;
                            while (j2 < len && (isalnum((unsigned char)src[j2]) || src[j2] == '_')) j2++;
                            size_t opl = j2 - ops;

                            int is_cmp      = (opl == 7 && memcmp(src + ops, "compare",  7) == 0);
                            int is_then     = (opl == 4 && memcmp(src + ops, "then",     4) == 0);
                            int is_else     = (opl == 4 && memcmp(src + ops, "else",     4) == 0);
                            int is_repeat   = (opl == 6 && memcmp(src + ops, "repeat",   6) == 0);
                            int is_while    = (opl == 5 && memcmp(src + ops, "while",    5) == 0);
                            int is_until    = (opl == 5 && memcmp(src + ops, "until",    5) == 0);
                            int is_cmd_op = -1;
                            if (!is_cmp && !is_then && !is_else && !is_repeat && !is_while && !is_until) {
                                for (int ci = 0; ci < ctx->cmd_count; ci++) {
                                    if (strlen(ctx->commands[ci].name) == opl && memcmp(ctx->commands[ci].name, src + ops, opl) == 0) {
                                        is_cmd_op = ci; break;
                                    }
                                }
                            }
                            
                            if (!is_cmp && !is_then && !is_else && !is_repeat && !is_while && 
                                !is_until && is_cmd_op < 0) break;

                            /* Consume optional whitespace, then expect a block */
                            size_t j3 = j2;
                            while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                            if (j3 >= len || !str_pfx(src + j3, sym->open)) break;

                            StrView blk;
                            j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &blk);
                            char *arg = (is_repeat || is_while || is_until || is_cmd_op >= 0) ? NULL : pap_process_sv(ctx, blk);

                            if (is_cmp) {
                                if (strcmp(cur_val, arg) != 0) { free(cur_val); cur_val = strdup(""); }
                            } else if (is_then) {
                                if (cur_val[0] != '\0') { free(cur_val); cur_val = arg; arg = NULL; }
                                else { free(cur_val); cur_val = strdup(""); }
                            } else if (is_else) {
                                if (cur_val[0] == '\0') { free(cur_val); cur_val = arg; arg = NULL; }
                            } else if (is_repeat) {
                                char *times_str = pap_process_sv(ctx, blk);
                                int times = atoi(times_str); free(times_str);
                                while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    for (int t = 0; t < times; t++) {
                                        char *tmp = pap_process_sv(ctx, code_blk);
                                        free(tmp);
                                    }
                                }
                                free(cur_val); cur_val = strdup("");
                            } else if (is_while) {
                                StrView pat_blk = blk;
                                char *pat_str = pap_process_sv(ctx, pat_blk);
                                Pattern pat; parse_pattern_ex(pat_str, &pat, sym);
                                
                                while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    char *last_res = strdup("");
                                    while (1) {
                                        char *iter = pap_process_sv(ctx, code_blk);
                                        Match m; m.ctx = ctx;
                                        int matches = match_pattern(ctx, iter, (int)strlen(iter), &pat, 0, &m);
                                        if (matches) free_match(&m);
                                        
                                        if (!matches) { free(iter); break; }
                                        free(last_res); last_res = iter;
                                    }
                                    free(cur_val);
                                    cur_val = last_res;
                                }
                                free_pattern(&pat);
                                free(pat_str);
                            } else if (is_until) {
                                StrView pat_blk = blk;
                                char *pat_str = pap_process_sv(ctx, pat_blk);
                                Pattern pat; parse_pattern_ex(pat_str, &pat, sym);
                                
                                while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                                if (j3 < len && str_pfx(src + j3, sym->open)) {
                                    StrView code_blk;
                                    j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &code_blk);
                                    char *last_res = strdup("");
                                    while (1) {
                                        char *iter = pap_process_sv(ctx, code_blk);
                                        Match m; m.ctx = ctx;
                                        int matches = match_pattern(ctx, iter, (int)strlen(iter), &pat, 0, &m);
                                        if (matches) free_match(&m);
                                        
                                        if (matches) { 
                                            free(last_res); last_res = iter;
                                            break; 
                                        }
                                        free(last_res); last_res = iter;
                                    }
                                    free(cur_val);
                                    cur_val = last_res;
                                }
                                free_pattern(&pat);
                                free(pat_str);
                            } else if (is_cmd_op >= 0) {
                                char *vargv[32]; size_t vargl[32]; int vargc = 0;
                                j3 = j2; /* Rewind to start parsing blocks */
                                while (vargc < 32) {
                                    size_t sj = j3;
                                    while(j3 < len && isspace((unsigned char)src[j3])) j3++;
                                    if (j3 < len && str_pfx(src + j3, sym->open)) {
                                        StrView blk_cmd; j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &blk_cmd);
                                        char *arg_cmd = (char*)malloc(blk_cmd.len + 1);
                                        if (arg_cmd) { memcpy(arg_cmd, blk_cmd.ptr, blk_cmd.len); arg_cmd[blk_cmd.len] = '\0'; }
                                        vargv[vargc] = arg_cmd; vargl[vargc] = blk_cmd.len; vargc++;
                                    } else { j3 = sj; break; }
                                }
                                RegisteredCommand *cmd = &ctx->commands[is_cmd_op];
                                char *res = cmd->handler(ctx, cmd->name, vargc, (const char **)vargv, vargl, cur_val, cmd->userdata);
                                for (int ci = 0; ci < vargc; ci++) if (vargv[ci]) free(vargv[ci]);
                                free(cur_val); cur_val = res ? res : strdup("");
                                
                                /* Persistent update if applicable */
                                if (klen > 0) pap_var_update(ctx, sym, src + ks, klen, cur_val);
                            }
                             if (arg) free(arg);
                            cp = j3;
                        }

                        sb_append_n(&out, cur_val, strlen(cur_val));
                        free(cur_val);
                        i = cp;
                        goto next_char;
                    }
                }

                if (klen == 4 && memcmp(src + ks, "args", 4) == 0 && j < len && src[j] == '$') {
                    j++; /* skip the second $ in $args$ */
                    size_t start = j;
                    while (j < len && (isalnum((unsigned char)src[j]) || src[j] == '_')) j++;
                    size_t vlen = j - start;
                    if (vlen > 0) {
                        int resolved = 0;
                        if (isdigit((unsigned char)src[start])) {
                            /* $args$0 = argv[1] (script name), $args$1 = argv[2], etc. */
                            int idx = atoi(src + start);
                            if (ctx && idx >= 0 && (idx + 1) < ctx->argc) {
                                sb_append_n(&out, ctx->argv[idx + 1], strlen(ctx->argv[idx + 1]));
                                resolved = 1;
                            }
                        } else if (vlen == 5 && memcmp(src + start, "count", 5) == 0) {
                            /* $args$count = total number of args (not counting argv[0], the binary) */
                            char nbuf[32];
                            int count = ctx ? (ctx->argc > 1 ? ctx->argc - 1 : 0) : 0;
                            snprintf(nbuf, sizeof(nbuf), "%d", count);
                            sb_append_n(&out, nbuf, strlen(nbuf));
                            resolved = 1;
                        } else if (vlen == 3 && memcmp(src + start, "all", 3) == 0) {
                            /* $args$all = all args from index 2 onwards (skip binary and script) */
                            if (ctx) {
                                for (int k = 2; k < ctx->argc; k++) {
                                    sb_append_n(&out, ctx->argv[k], strlen(ctx->argv[k]));
                                    if (k + 1 < ctx->argc) sb_append_char(&out, ' ');
                                }
                                resolved = 1;
                            }
                        } else {
                            /* Named variable: scan argv for NAME=VALUE */
                            if (ctx && ctx->argv) {
                                for (int k = ctx->argc - 1; k >= 0; k--) {
                                    const char *arg = ctx->argv[k];
                                    if (strncmp(arg, src + start, vlen) == 0 && arg[vlen] == '=') {
                                        /* Append the value part, but stop at any control char */
                                        const char *val = arg + vlen + 1;
                                        sb_append_n(&out, val, strlen(val));
                                        resolved = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        if (resolved) { i = j; continue; }
                        /* Not resolved: emit literally so it remains visible */
                        sb_append_n(&out, src + i, j - i);
                        i = j; goto next_char;
                    }
                }

                /* Bare $args (without second $) = alias for $args$all */
                if (klen == 4 && memcmp(src + ks, "args", 4) == 0 &&
                    (j >= len || src[j] != '$')) {
                    if (ctx && ctx->argc > 2) {
                        for (int k = 2; k < ctx->argc; k++) {
                            sb_append_n(&out, ctx->argv[k], strlen(ctx->argv[k]));
                            if (k + 1 < ctx->argc) sb_append_char(&out, ' ');
                        }
                    }
                    i = j; goto next_char;
                }

                /* Direct Variable Support: $NAME aliasing $args$NAME
                   Only resolve if it's not a known command or preprocessor directive */
                int is_cmd = 0;
                if (ctx) {
                    for (int ci = 0; ci < ctx->cmd_count; ci++) {
                        if (strlen(ctx->commands[ci].name) == klen &&
                            memcmp(ctx->commands[ci].name, src + ks, klen) == 0) {
                            is_cmd = 1; break;
                        }
                    }
                    if (!is_cmd && klen == 8 && memcmp(src + ks, "document", 8) == 0) is_cmd = 1;
                }

                if (!is_cmd && ctx && ctx->argv) {
                    for (int k = ctx->argc - 1; k >= 0; k--) {
                        const char *arg = ctx->argv[k];
                        if (strncmp(arg, src + ks, klen) == 0 && arg[klen] == '=') {
                            /* Sanitized direct-variable expansion: skip control chars */
                            const char *val = arg + klen + 1;
                            sb_append_n(&out, val, strlen(val));
                            i = j; goto next_char;
                        }
                    }
                }
            }
        }
        sb_append_char(&out, src[i++]);
    next_char:;
    }
    return out.data;
}

static char *dispatch_commands(Papagaio *ctx, const char *src, const Symbols *sym)
{
    if (!ctx || !src) return src ? strdup(src) : NULL;
    StrBuf out; sb_init(&out);
    size_t i = 0, len = strlen(src);
    size_t sl = strlen(sym->sigil);
    char sigil = sym->sigil[0];
    StrView so = { sym->open,  strlen(sym->open)  };
    StrView sc = { sym->close, strlen(sym->close) };

    while (i < len) {
        if (src[i] == sigil) {
            size_t j = i + sl;
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
                    char *res = cmd->handler(ctx, cmd->name, vargc, (const char **)vargv, vargl, NULL, cmd->userdata);
                    if (res) { sb_append_n(&out, res, strlen(res)); free(res); }
                    for (int ci = 0; ci < vargc; ci++) if (vargv[ci]) free(vargv[ci]);
                    i = j; continue;
                }
                
                /* $once{...} logic */
                if (klen == 4 && memcmp(src + ks, "once", 4) == 0) {
                    size_t sj = j;
                    while(j < len && isspace((unsigned char)src[j])) j++;
                    if (j < len && sv_pfx(src + j, so)) {
                        StrView blk; j = (size_t)extract_block(src, (int)j, so, sc, &blk);
                        char *arg = (char*)malloc(blk.len + 1);
                        if (arg) {
                            memcpy(arg, blk.ptr, blk.len); arg[blk.len] = '\0';
                            int old_once = ctx->once_mode;
                            ctx->once_mode = 1;
                            char *res = papagaio_process_text(ctx, arg, blk.len);
                            ctx->once_mode = old_once;
                            if (res) { sb_append_n(&out, res, strlen(res)); free(res); }
                            free(arg);
                        }
                        i = j; continue;
                    }
                    j = sj;
                }

                /* $import logic — CLI-only (ctx->cli_mode must be set) */
                if (klen == 6 && memcmp(src + ks, "import", 6) == 0) {
                    if (!ctx->cli_mode) {
                        /* Not in CLI mode: emit literally and move on */
                        sb_append_n(&out, src + i, j - i);
                        i = j; continue;
                    }
                    size_t sj = j;
                    while(j < len && isspace((unsigned char)src[j])) j++;
                    if (j < len && sv_pfx(src + j, so)) {
                        StrView blk; j = (size_t)extract_block(src, (int)j, so, sc, &blk);
                        char *arg = (char*)malloc(blk.len + 1);
                        if (arg) {
                            memcpy(arg, blk.ptr, blk.len); arg[blk.len] = '\0';
                            char *libname = papagaio_process_text(ctx, arg, blk.len);
                            if (libname) {
#if defined(PAPAGAIO_USE_WINDOWS_DL)
                                void *handle = (void*)LoadLibraryA(libname);
                                if (handle) {
                                    PapagaioPluginInit init_fn = (PapagaioPluginInit)GetProcAddress((HMODULE)handle, "papagaio_plugin_init");
                                    if (init_fn) {
                                        init_fn(ctx);
                                        if (ctx->dl_count >= ctx->dl_cap) {
                                            ctx->dl_cap = ctx->dl_cap ? ctx->dl_cap * 2 : 4;
                                            ctx->dl_handles = realloc(ctx->dl_handles, ctx->dl_cap * sizeof(void*));
                                        }
                                        ctx->dl_handles[ctx->dl_count++] = handle;
                                    } else {
                                        FreeLibrary((HMODULE)handle);
                                    }
                                }
#elif defined(PAPAGAIO_USE_POSIX_DL)
                                void *handle = dlopen(libname, RTLD_NOW | RTLD_LOCAL);
                                if (handle) {
                                    PapagaioPluginInit init_fn = (PapagaioPluginInit)dlsym(handle, "papagaio_plugin_init");
                                    if (init_fn) {
                                        init_fn(ctx);
                                        if (ctx->dl_count >= ctx->dl_cap) {
                                            ctx->dl_cap = ctx->dl_cap ? ctx->dl_cap * 2 : 4;
                                            ctx->dl_handles = realloc(ctx->dl_handles, ctx->dl_cap * sizeof(void*));
                                        }
                                        ctx->dl_handles[ctx->dl_count++] = handle;
                                    } else {
                                        fprintf(stderr, "[ERROR] dlsym failed for '%s': %s\n", libname, dlerror());
                                        dlclose(handle);
                                    }
                                } else {
                                    fprintf(stderr, "[ERROR] dlopen failed for '%s': %s\n", libname, dlerror());
                                }
#endif
                                free(libname);
                            }
                            free(arg);
                        }
                        i = j; continue;
                    }
                    j = sj;
                }

                /* $document logic */
                if (klen == 8 && memcmp(src + ks, "document", 8) == 0) {
                    if (j < len && src[j] == '$') {
                        j++;
                        size_t s2 = j;
                        while (j < len && (isalnum((unsigned char)src[j]) || src[j] == '_')) j++;
                        size_t vlen = j - s2;
                        if (vlen == 8 && memcmp(src + s2, "original", 8) == 0) {
                            if (ctx && ctx->original_doc) sb_append_n(&out, ctx->original_doc, strlen(ctx->original_doc));
                        } else if (vlen == 7 && memcmp(src + s2, "current", 7) == 0) {
                            sb_append_n(&out, src, strlen(src));
                        } else {
                            sb_append_n(&out, src, strlen(src));
                        }
                    } else {
                        sb_append_n(&out, src, strlen(src));
                    }
                    i = j; continue;
                }
            }
        }
        sb_append_char(&out, src[i++]);
    }
    
    char *result = strdup(out.data);
    sb_free(&out);

    /* Pass 2: Automatic Export Attributes for papagaio_ functions */
    /* We look for "papagaio_" and if it looks like a definition at the start of a word, 
       we prepend visibility attribute. */
    if (ctx && ctx->auto_export) {
        StrBuf out2; sb_init(&out2);
        size_t rl = strlen(result);
        for (size_t k = 0; k < rl; k++) {
            if (k + 9 < rl && memcmp(result + k, "papagaio_", 9) == 0) {
                /* Check context: preceded by whitespace/type and followed by '(' later?
                   A simple heuristic: if it's at start of line or preceded by ' ' or '*' or '}' */
                int is_def = (k == 0 || result[k-1] == ' ' || result[k-1] == '\t' || result[k-1] == '\n' || result[k-1] == '*' || result[k-1] == '}');
                if (is_def) {
                    /* Check if it's a function call or definition. 
                       We search forward for '(' before ';' or '='. */
                    int looks_like_func = 0;
                    for (size_t m = k + 9; m < rl && m < k + 128; m++) {
                        if (result[m] == '(') { looks_like_func = 1; break; }
                        if (result[m] == ';' || result[m] == '=' || result[m] == '{') break;
                    }
                    if (looks_like_func) {
                        sb_append_n(&out2, "__attribute__((visibility(\"default\"))) ", 39);
                    }
                }
            }
            sb_append_char(&out2, result[k]);
        }
        free(result);
        result = strdup(out2.data);
        sb_free(&out2);
    }

    return result;
}



char *papagaio_process_text(Papagaio *ctx, const char *input, size_t len)
{
    if (!ctx || !input) return NULL;
    Symbols sym = make_symbols(PAP_SIGIL, PAP_OPEN, PAP_CLOSE);

    ctx->depth++;
    if (ctx->depth == 1) {
        clear_scope(ctx->current_scope);
        if (ctx->original_doc) free(ctx->original_doc);
        ctx->original_doc = (char*)malloc(len + 1);
        if (ctx->original_doc) {
            memcpy(ctx->original_doc, input, len);
            ctx->original_doc[len] = '\0';
            ctx->original_len = len;
        } else {
            ctx->original_len = 0;
        }
    } else if (!ctx->disable_sandbox) {
        push_scope(ctx);
    }



    char *buf = (char *)malloc(len + 1);
    if (!buf) {
        if (ctx->depth > 1 && !ctx->disable_sandbox) pop_scope(ctx);
        ctx->depth--;
        return NULL;
    }
    memcpy(buf, input, len); buf[len] = '\0';

    char *preprocessed = resolve_preprocessor(ctx, buf, &sym); free(buf);
    if (!preprocessed) {
        if (ctx->depth > 1 && !ctx->disable_sandbox) pop_scope(ctx);
        ctx->depth--;
        return NULL;
    }
    
    /* A. Resolve Patterns - Now adds to persistent ctx->current_scope->rules */
    PatternPair *new_pairs = NULL; int new_pc = 0;
    char *text_no_patterns = extract_nested(preprocessed, &sym, &new_pairs, &new_pc);
    free(preprocessed);
    if (!text_no_patterns) {
        free_pairs(new_pairs, new_pc);
        if (ctx->depth > 1 && !ctx->disable_sandbox) pop_scope(ctx);
        ctx->depth--;
        return NULL;
    }

    if (new_pc > 0) {
        if (ctx->current_scope->rule_count + new_pc > ctx->current_scope->rule_cap) {
            ctx->current_scope->rule_cap = ctx->current_scope->rule_cap ? (ctx->current_scope->rule_cap + new_pc) * 2 : (new_pc + 8);
            ctx->current_scope->rules = (PatternPair *)realloc(ctx->current_scope->rules, sizeof(PatternPair) * ctx->current_scope->rule_cap);
        }
        for (int i = 0; i < new_pc; i++) {
            ctx->current_scope->rules[ctx->current_scope->rule_count++] = new_pairs[i];
        }
        free(new_pairs); /* Don't use free_pairs as we moved the strings */
    }
    
    char *cur = text_no_patterns;
    // Collect all scopes from global to current
    int scope_count = 0;
    for (Scope *s = ctx->current_scope; s; s = s->parent) scope_count++;
    Scope **scopes = (Scope **)malloc(sizeof(Scope*) * scope_count);
    int s_idx = scope_count - 1;
    for (Scope *s = ctx->current_scope; s; s = s->parent) scopes[s_idx--] = s;

    // Apply ALL rules from all scopes (Global -> Local)
    for (int si = 0; si < scope_count; si++) {
        Scope *s = scopes[si];
        
        for (int i = 0; i < s->rule_count; i++) {
            /* If patterns are disabled, we ONLY allow aliases/variable rules */
            if (ctx->disable_patterns) {
                int is_aliases_rule = (s->rules[i].m && 
                                       s->rules[i].m[0] == '$' && 
                                       s->rules[i].m[1] == '$' && 
                                       strstr(s->rules[i].m, "$aliases{") != NULL);
                if (!is_aliases_rule) continue;
            }

            StrBuf out; sb_init(&out);
            size_t clen = strlen(cur), pos = 0;
            Pattern pat;
            parse_pattern_ex(s->rules[i].m, &pat, &sym);
            
            while (pos < clen) {
                Match m; m.ctx = ctx;
                if (match_pattern(ctx, cur, (int)clen, &pat, (int)pos, &m)) {
                    char *r = apply_replacement_ex(s->rules[i].r, &m, &sym);
                    int is_aliases_rule = (s->rules[i].m && 
                                           s->rules[i].m[0] == '$' && 
                                           s->rules[i].m[1] == '$' && 
                                           strstr(s->rules[i].m, "$aliases{") != NULL);
                    if (is_aliases_rule) {
                        sb_append_n(&out, r, strlen(r));
                    } else {
                        char *evaluated_r;
                        if (ctx->once_mode) {
                            ctx->disable_patterns++;
                            evaluated_r = papagaio_process_text(ctx, r, strlen(r));
                            ctx->disable_patterns--;
                        } else {
                            evaluated_r = strdup(r);
                        }
                        sb_append_n(&out, evaluated_r, strlen(evaluated_r));
                        free(evaluated_r);
                    }
                    free(r);
                    pos = (size_t)m.end;
                    free_match(&m);
                } else {
                    sb_append_char(&out, cur[pos++]);
                }
            }
            free_pattern(&pat);
            char *next_cur = strdup(out.data);
            sb_free(&out);
            free(cur);
            cur = next_cur;
        }
    }
    free(scopes);

    char *final = dispatch_commands(ctx, cur, &sym);
    free(cur);
    
    if (ctx->depth > 1 && !ctx->disable_sandbox) {
        pop_scope(ctx);
    }
    ctx->depth--;
    return final;
}

