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
#include <regex.h>


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
    regex_t    *re;       /* compiled POSIX regex */
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
        regmatch_t *capture;
        int         capture_count;
        size_t      match_start;
        size_t      match_end;
        const char     *src;
    } regex;
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
static char *file_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud);

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
    PatternPair *rules;
    int          rule_count, rule_cap;
    int          depth;

    /* original document for $document$original */
    char        *original_doc;
    size_t       original_len;
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
        if (p->t[i].re) {
            regfree(p->t[i].re);
            free(p->t[i].re);
        }
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
    if (m->regex.capture) { free(m->regex.capture); m->regex.capture = NULL; }
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
            /* Trailing sigil after var/modifier: next whitespace is consumed (optional) */
            if (i < n && str_pfx(pat + i, sym->sigil)) {
                size_t sl2 = strlen(sym->sigil);
                size_t j2 = i + sl2;
                /* Only treat as ws_consume if NOT followed by alphanum (would start new var) */
                if (j2 >= (size_t)n || (!isalnum((unsigned char)pat[j2]) && pat[j2] != '_')) {
                    t->ws_consume = 1;
                    i += sl2;
                }
            }
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
        /* Trailing sigil on literal: consume whitespace after match */
        if (i < n && str_pfx(pat + i, sym->sigil)) {
            size_t sl2 = strlen(sym->sigil);
            size_t j2 = i + sl2;
            if (j2 >= (size_t)n || (!isalnum((unsigned char)pat[j2]) && pat[j2] != '_')) {
                p->t[p->count-1].ws_consume = 1;
                i += sl2;
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

        if (t->type == TOK_REGEX) {
            if (!t->re && t->value.len > 0) {
                t->re = (regex_t *)malloc(sizeof(regex_t));
                char *pat = (char *)malloc(t->value.len + 1);
                if (pat) {
                    memcpy(pat, t->value.ptr, t->value.len);
                    pat[t->value.len] = '\0';
                    if (regcomp(t->re, pat, REG_EXTENDED) != 0) {
                        free(t->re); t->re = NULL;
                    }
                    free(pat);
                }
            }
            if (!t->re) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                continue;
            }

            regmatch_t pmatch[10];
            if (regexec(t->re, src + pos, 10, pmatch, 0) != 0 || pmatch[0].rm_so != 0) {
                if (!t->optional) goto fail;
                ensure_cap(m); m->cap[m->count++] = (Capture){ t->var, { "", 0 }, NULL };
                continue;
            }

            size_t match_start = (size_t)pos + pmatch[0].rm_so;
            size_t match_end   = (size_t)pos + pmatch[0].rm_eo;

            /* keep last regex capture info */
            if (m->regex.capture) { free(m->regex.capture); m->regex.capture = NULL; }
            m->regex.capture = (regmatch_t *)malloc(sizeof(regmatch_t) * 10);
            if (m->regex.capture) {
                memcpy(m->regex.capture, pmatch, sizeof(regmatch_t) * 10);
                m->regex.capture_count = 10;
            }
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
    for (int i = 0; i < ctx->rule_count; i++) {
        if (ctx->rules[i].m && strcmp(ctx->rules[i].m, pat) == 0) {
            result = strdup(ctx->rules[i].r);
            break;
        }
    }
    free(pat);
    return result;
}

/* Create or update a $NAME variable in ctx->rules (same logic as $from). */
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
    int found_idx = -1;
    for (int ri = 0; ri < ctx->rule_count; ri++) {
        if (ctx->rules[ri].m && strcmp(ctx->rules[ri].m, pat_str) == 0) {
            found_idx = ri; break;
        }
    }
    if (found_idx >= 0) {
        free(ctx->rules[found_idx].r);
        ctx->rules[found_idx].r = strdup(new_value);
        free(pat_str);
    } else {
        if (ctx->rule_count + 1 > ctx->rule_cap) {
            ctx->rule_cap = ctx->rule_cap ? ctx->rule_cap * 2 : 8;
            ctx->rules = (PatternPair *)realloc(ctx->rules,
                          sizeof(PatternPair) * ctx->rule_cap);
        }
        ctx->rules[ctx->rule_count].m = pat_str;
        ctx->rules[ctx->rule_count].r = strdup(new_value);
        ctx->rule_count++;
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
    char *out = papagaio_process_text(ctx, tmp, sv.len);
    free(tmp);
    return out ? out : strdup("");
}

/* Central dispatcher for $VAR$list$OP{sep}{...} operations.
   raw_blocks[0] = sep block, raw_blocks[1..] = extra argument blocks.
   Emitting ops write to sb_out (may be NULL for pure-mutating ops).
   Mutating ops update ctx->rules via pap_var_update. */
static void pap_list_op(Papagaio *ctx, const Symbols *sym,
                         StrBuf *sb_out,
                         const char *name, size_t nlen,
                         const char *op,   size_t oplen,
                         StrView *raw_blocks, int block_count)
{
    if (block_count < 1) return;

    /* --- Process separator (always block[0]) --- */
    char *sep_str = pap_process_sv(ctx, raw_blocks[0]);
    size_t seplen = strlen(sep_str);

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
        if (block_count >= 2 && sb_out) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[1]);
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
        if (block_count >= 3) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[1]);
            int idx = pap_list_normalize_idx(idx_str, count);
            free(idx_str);
            if (idx >= 0) {
                char *content = pap_process_sv(ctx, raw_blocks[2]);
                free(parts[idx]);
                parts[idx] = content;
                mutated = 1;
            }
        }
    }
    /* push: append to end */
    else if (OP_IS("push")) {
        if (block_count >= 2) {
            char *content = pap_process_sv(ctx, raw_blocks[1]);
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
            memmove(parts, parts + 1, sizeof(char *) * (size_t)(count - 1));
            count--;
            mutated = 1;
        }
    }
    /* unshift: prepend to front */
    else if (OP_IS("unshift")) {
        if (block_count >= 2) {
            char *content = pap_process_sv(ctx, raw_blocks[1]);
            parts = (char **)realloc(parts, sizeof(char *) * (size_t)(count + 1));
            memmove(parts + 1, parts, sizeof(char *) * (size_t)count);
            parts[0] = content;
            count++;
            mutated = 1;
        }
    }
    /* insert: insert before index (clamped, allows == count for append) */
    else if (OP_IS("insert")) {
        if (block_count >= 3) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[1]);
            int raw_idx = atoi(idx_str); free(idx_str);
            if (raw_idx < 0) raw_idx = count + raw_idx;
            if (raw_idx < 0) raw_idx = 0;
            if (raw_idx > count) raw_idx = count;
            char *content = pap_process_sv(ctx, raw_blocks[2]);
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
        if (block_count >= 2 && count > 0) {
            char *idx_str = pap_process_sv(ctx, raw_blocks[1]);
            int idx = pap_list_normalize_idx(idx_str, count);
            free(idx_str);
            if (idx >= 0) {
                free(parts[idx]);
                memmove(parts + idx, parts + idx + 1,
                        sizeof(char *) * (size_t)(count - idx - 1));
                count--;
                mutated = 1;
            }
        }
    }
    /* swap: exchange two elements */
    else if (OP_IS("swap")) {
        if (block_count >= 3 && count > 1) {
            char *ia_str = pap_process_sv(ctx, raw_blocks[1]);
            char *ib_str = pap_process_sv(ctx, raw_blocks[2]);
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
        if (block_count >= 2 && sb_out) {
            char *new_sep = pap_process_sv(ctx, raw_blocks[1]);
            char *joined  = pap_list_join(parts, count, new_sep, strlen(new_sep));
            sb_append_n(sb_out, joined, strlen(joined));
            free(joined); free(new_sep);
        }
    }

#undef OP_IS

    /* Persist mutation back to variable */
    if (mutated) {
        char *new_val = pap_list_join(parts, count, sep_str, seplen);
        pap_var_update(ctx, sym, name, nlen, new_val);
        free(new_val);
    }

    pap_list_free(parts, count);
    free(sep_str);
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

/* =========================================================================
 * Public C API
 * ====================================================================== */

Papagaio *papagaio_open(void)
{
    Papagaio *ctx = (Papagaio *)malloc(sizeof(Papagaio));
    if (!ctx) return NULL;

    ctx->commands   = NULL; ctx->cmd_count = 0; ctx->cmd_cap = 0;
    ctx->modifiers  = NULL; ctx->mod_count = 0; ctx->mod_cap = 0;
    ctx->finalizers = NULL; ctx->fin_count = 0; ctx->fin_cap = 0;
    ctx->argc       = 0;    ctx->argv      = NULL;
    ctx->auto_export = 1;
    ctx->rules = NULL; ctx->rule_count = 0; ctx->rule_cap = 0;
    ctx->depth = 0;
    ctx->original_doc = NULL; ctx->original_len = 0;

    papagaio_register_command(ctx, "file", file_handler, NULL);
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
    if (ctx->rules) {
        for (int i = 0; i < ctx->rule_count; i++) {
            free(ctx->rules[i].m);
            free(ctx->rules[i].r);
        }
        free(ctx->rules);
    }
    if (ctx->original_doc) free(ctx->original_doc);
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

static char *file_handler(Papagaio *ctx, const char *name, int argc, const char **argv, const size_t *argl, void *ud) {
    (void)ctx; (void)name; (void)ud; (void)argl;
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
    size_t sl = strlen(sym->sigil);

    while (i < len) {
        if (src[i] == '$') { /* FIXED SIGIL for preprocessor */
            size_t j = i + 1;
            size_t ks = j;
            while (j < len && (isalnum((unsigned char)src[j]) || src[j] == '_')) j++;
            size_t klen = j - ks;

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
                    if ((nfl == 7 && memcmp(src + njs, "compare", 7) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "then",    4) == 0) ||
                        (nfl == 4 && memcmp(src + njs, "else",    4) == 0)) {
                        /* Process block content as initial value */
                        char *cur_val = pap_process_sv(ctx, blk_b);
                        size_t cp = jb;
                        StrView so_f = { sym->open,  strlen(sym->open)  };
                        StrView sc_f = { sym->close, strlen(sym->close) };
                        while (cp < len && src[cp] == '$') {
                            size_t j2 = cp + 1, ops = j2;
                            while (j2 < len && (isalnum((unsigned char)src[j2]) || src[j2] == '_')) j2++;
                            size_t opl = j2 - ops;
                            int is_cmp  = (opl == 7 && memcmp(src + ops, "compare", 7) == 0);
                            int is_then = (opl == 4 && memcmp(src + ops, "then",    4) == 0);
                            int is_else = (opl == 4 && memcmp(src + ops, "else",    4) == 0);
                            if (!is_cmp && !is_then && !is_else) break;
                            size_t j3 = j2;
                            while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                            if (j3 >= len || !str_pfx(src + j3, sym->open)) break;
                            StrView blk; j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &blk);
                            char *arg = pap_process_sv(ctx, blk);
                            if (is_cmp) {
                                if (strcmp(cur_val, arg) != 0) { free(cur_val); cur_val = strdup(""); }
                                free(arg);
                            } else if (is_then) {
                                if (cur_val[0] != '\0') { free(cur_val); cur_val = arg; arg = NULL; }
                                else free(arg);
                            } else {
                                if (cur_val[0] == '\0') { free(cur_val); cur_val = arg; arg = NULL; }
                                else free(arg);
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
                                /* Pattern: $$NAME$aliases{NAME} for exact literal match */
                                size_t pat_len = sl * 3 + name_len + 7 + strlen(sym->open) + name_len + strlen(sym->close);
                                char *pat_str = (char *)malloc(pat_len + 1);
                                if (pat_str) {
                                    char *p = pat_str;
                                    memcpy(p, sym->sigil, sl); p += sl;
                                    memcpy(p, sym->sigil, sl); p += sl;
                                    memcpy(p, src + ks_name, name_len); p += name_len;
                                    memcpy(p, sym->sigil, sl); p += sl;
                                    memcpy(p, "aliases", 7); p += 7;
                                    memcpy(p, sym->open, strlen(sym->open)); p += strlen(sym->open);
                                    memcpy(p, src + ks_name, name_len); p += name_len;
                                    memcpy(p, sym->close, strlen(sym->close)); p += strlen(sym->close);
                                    *p = '\0';
                                    
                                    /* Check for redefinition */
                                    int found_idx = -1;
                                    for (int ri = 0; ri < ctx->rule_count; ri++) {
                                        if (strcmp(ctx->rules[ri].m, pat_str) == 0) {
                                            found_idx = ri; break;
                                        }
                                    }
                                    
                                    if (found_idx >= 0) {
                                        free(ctx->rules[found_idx].r);
                                        ctx->rules[found_idx].r = strdup(processed_val);
                                        free(pat_str);
                                    } else {
                                        if (ctx->rule_count + 1 > ctx->rule_cap) {
                                            ctx->rule_cap = ctx->rule_cap ? ctx->rule_cap * 2 : 8;
                                            ctx->rules = (PatternPair *)realloc(ctx->rules, sizeof(PatternPair) * ctx->rule_cap);
                                        }
                                        ctx->rules[ctx->rule_count].m = pat_str;
                                        ctx->rules[ctx->rule_count].r = strdup(processed_val);
                                        ctx->rule_count++;
                                    }
                                }
                                free(processed_val);
                            }
                            i = j_next; continue;
                        }
                    }
                }

                /* Check for $NAME$list$OP{sep}{...} list operations */
                if (j < len && src[j] == '$') {
                    size_t j2 = j + 1;
                    size_t ms = j2;
                    while (j2 < len && (isalnum((unsigned char)src[j2]) || src[j2] == '_')) j2++;
                    size_t mlen = j2 - ms;

                    if (mlen == 4 && memcmp(src + ms, "list", 4) == 0 &&
                        j2 < len && src[j2] == '$') {
                        size_t j3 = j2 + 1;
                        size_t ops = j3;
                        while (j3 < len && (isalnum((unsigned char)src[j3]) || src[j3] == '_')) j3++;
                        size_t oplen = j3 - ops;

                        if (oplen > 0) {
                            /* Extract up to 4 argument blocks {sep}{a1}{a2}{a3} */
                            StrView blocks[4];
                            int block_count = 0;
                            StrView so = { sym->open,  strlen(sym->open)  };
                            StrView sc = { sym->close, strlen(sym->close) };
                            size_t jb = j3;
                            while (block_count < 4) {
                                size_t jb_saved = jb; /* save before consuming ws */
                                while (jb < len && isspace((unsigned char)src[jb])) jb++;
                                if (jb >= len || !str_pfx(src + jb, sym->open)) {
                                    jb = jb_saved; /* restore: don't eat trailing ws */
                                    break;
                                }
                                StrView blk;
                                jb = (size_t)extract_block(src, (int)jb, so, sc, &blk);
                                blocks[block_count++] = blk;
                            }

                            pap_list_op(ctx, sym, &out,
                                        src + ks, klen,
                                        src + ops, oplen,
                                        blocks, block_count);
                            i = jb; continue;
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
                    int cur_is_then    = (klen == 4 && memcmp(src + ks, "then",    4) == 0);
                    int cur_is_else    = (klen == 4 && memcmp(src + ks, "else",    4) == 0);
                    int cur_is_compare = (klen == 7 && memcmp(src + ks, "compare", 7) == 0);
                    int cur_is_flow    = cur_is_then || cur_is_else || cur_is_compare;

                    /* Check if current NAME is followed by a flow op */
                    int next_is_flow = 0;
                    if (!cur_is_flow && j < len && src[j] == '$') {
                        size_t nj = j + 1, njs = nj;
                        while (nj < len && (isalnum((unsigned char)src[nj]) || src[nj] == '_')) nj++;
                        size_t nfl = nj - njs;
                        if ((nfl == 7 && memcmp(src + njs, "compare", 7) == 0) ||
                            (nfl == 4 && memcmp(src + njs, "then",    4) == 0) ||
                            (nfl == 4 && memcmp(src + njs, "else",    4) == 0))
                            next_is_flow = 1;
                    }

                    if (cur_is_flow || next_is_flow) {
                        StrView so_f = { sym->open,  strlen(sym->open)  };
                        StrView sc_f = { sym->close, strlen(sym->close) };

                        /* Determine initial value and where the chain starts */
                        char *cur_val;
                        size_t cp; /* position in src where we scan flow ops */

                        if (cur_is_flow) {
                            /* Standalone: input is empty string, chain starts here */
                            cur_val = strdup("");
                            cp = i; /* points to '$' of "then"/"else"/"compare" */
                        } else {
                            /* $NAME$op...: resolve NAME via pap_var_lookup */
                            char *lu = pap_var_lookup(ctx, sym, src + ks, klen);
                            cur_val = lu ? lu : strdup("");
                            cp = j; /* points to '$' of first flow op */
                        }

                        /* Process chain of $compare/$then/$else */
                        while (cp < len && src[cp] == '$') {
                            size_t j2 = cp + 1, ops = j2;
                            while (j2 < len && (isalnum((unsigned char)src[j2]) || src[j2] == '_')) j2++;
                            size_t opl = j2 - ops;

                            int is_cmp  = (opl == 7 && memcmp(src + ops, "compare", 7) == 0);
                            int is_then = (opl == 4 && memcmp(src + ops, "then",    4) == 0);
                            int is_else = (opl == 4 && memcmp(src + ops, "else",    4) == 0);
                            if (!is_cmp && !is_then && !is_else) break;

                            /* Consume optional whitespace, then expect a block */
                            size_t j3 = j2;
                            while (j3 < len && isspace((unsigned char)src[j3])) j3++;
                            if (j3 >= len || !str_pfx(src + j3, sym->open)) break;

                            StrView blk;
                            j3 = (size_t)extract_block(src, (int)j3, so_f, sc_f, &blk);
                            char *arg = pap_process_sv(ctx, blk);

                             if (is_cmp) {
                                 /* compare: equal → keep cur_val, unequal → "" */
                                 if (strcmp(cur_val, arg) != 0) {
                                     free(cur_val); cur_val = strdup("");
                                 }
                                 free(arg); arg = NULL;
                             } else if (is_then) {
                                 /* then: non-empty → replace with arg; empty → stay "" */
                                 if (cur_val[0] != '\0') {
                                     free(cur_val); cur_val = arg; arg = NULL;
                                 } else {
                                     free(arg); arg = NULL;
                                 }
                             } else { /* else */
                                 /* else: empty → replace with arg; non-empty → pass cur through */
                                 if (cur_val[0] == '\0') {
                                     free(cur_val); cur_val = arg; arg = NULL;
                                 } else {
                                     free(arg); arg = NULL; /* keep cur_val as-is */
                                 }
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
                    char *res = cmd->handler(ctx, cmd->name, vargc, (const char **)vargv, vargl, cmd->userdata);
                    if (res) { sb_append_n(&out, res, strlen(res)); free(res); }
                    for (int ci = 0; ci < vargc; ci++) if (vargv[ci]) free(vargv[ci]);
                    i = j; continue;
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

typedef struct {
    int priority;
    size_t start;
    size_t end;
    char *content;
    char *result;
    int is_priority_block;
    int original_index;
} PChunk;

static int compare_pchunks_priority(const void *a, const void *b) {
    const PChunk *ca = (const PChunk *)a;
    const PChunk *cb = (const PChunk *)b;
    if (ca->priority < cb->priority) return -1;
    if (ca->priority > cb->priority) return 1;
    return ca->original_index - cb->original_index;
}

static int compare_pchunks_original(const void *a, const void *b) {
    const PChunk *ca = (const PChunk *)a;
    const PChunk *cb = (const PChunk *)b;
    return (int)ca->original_index - (int)cb->original_index;
}

static char *handle_priorities(Papagaio *ctx, const char *src, size_t len, const Symbols *sym) {
    size_t i = 0;
    size_t last_pos = 0;
    size_t sl = strlen(sym->sigil);
    
    PChunk *chunks = NULL;
    int chunk_count = 0;
    int chunk_cap = 0;
    int found_any = 0;

    while (i < len) {
        if (i + sl + 8 + sl < len && 
            memcmp(src + i, sym->sigil, sl) == 0 &&
            memcmp(src + i + sl, "priority", 8) == 0 &&
            memcmp(src + i + sl + 8, sym->sigil, sl) == 0) {
            
            size_t ps = i;
            size_t j = i + sl + 8 + sl;
            int prio = 0;
            int has_prio_val = 0;
            if (j + 3 <= len && memcmp(src + j, "max", 3) == 0) {
                prio = INT_MIN + 1;
                j += 3;
                has_prio_val = 1;
            } else if (j + 3 <= len && memcmp(src + j, "min", 3) == 0) {
                prio = INT_MAX - 1;
                j += 3;
                has_prio_val = 1;
            } else {
                int sign = 1;
                if (j < len && src[j] == '-') {
                    sign = -1;
                    j++;
                }
                while (j < len && isdigit((unsigned char)src[j])) {
                    prio = prio * 10 + (src[j] - '0');
                    j++;
                    has_prio_val = 1;
                }
                prio *= sign;
            }
            if (has_prio_val) {
                while (j < len && isspace((unsigned char)src[j])) j++;
                if (j < len && str_pfx(src + j, sym->open)) {
                    found_any = 1;
                    
                    /* Add non-priority chunk before this */
                    if (ps > last_pos) {
                        if (chunk_count >= chunk_cap) {
                            chunk_cap = chunk_cap ? chunk_cap * 2 : 8;
                            chunks = (PChunk *)realloc(chunks, sizeof(PChunk) * chunk_cap);
                        }
                        chunks[chunk_count].priority = INT_MAX - 1;
                        chunks[chunk_count].start = last_pos;
                        chunks[chunk_count].end = ps;
                        chunks[chunk_count].content = NULL;
                        chunks[chunk_count].result = NULL;
                        chunks[chunk_count].is_priority_block = 0;
                        chunks[chunk_count].original_index = chunk_count;
                        chunk_count++;
                    }

                    StrView v;
                    StrView so = { sym->open, strlen(sym->open) };
                    StrView sc = { sym->close, strlen(sym->close) };
                    int next = extract_block(src, (int)j, so, sc, &v);
                    
                    if (chunk_count >= chunk_cap) {
                        chunk_cap = chunk_cap ? chunk_cap * 2 : 8;
                        chunks = (PChunk *)realloc(chunks, sizeof(PChunk) * chunk_cap);
                    }
                    chunks[chunk_count].priority = prio;
                    chunks[chunk_count].start = ps;
                    chunks[chunk_count].end = (size_t)next;
                    chunks[chunk_count].content = (char *)malloc(v.len + 1);
                    memcpy(chunks[chunk_count].content, v.ptr, v.len);
                    chunks[chunk_count].content[v.len] = '\0';
                    chunks[chunk_count].result = NULL;
                    chunks[chunk_count].is_priority_block = 1;
                    chunks[chunk_count].original_index = chunk_count;
                    chunk_count++;
                    
                    i = (size_t)next;
                    last_pos = i;
                    continue;
                }
            }
        }
        i++;
    }

    if (!found_any) {
        if (chunks) free(chunks);
        return NULL;
    }

    /* Add trailing chunk */
    if (last_pos < len) {
        if (chunk_count >= chunk_cap) {
            chunk_cap = chunk_cap ? chunk_cap * 2 : 8;
            chunks = (PChunk *)realloc(chunks, sizeof(PChunk) * chunk_cap);
        }
        chunks[chunk_count].priority = INT_MAX - 1;
        chunks[chunk_count].start = last_pos;
        chunks[chunk_count].end = len;
        chunks[chunk_count].content = NULL;
        chunks[chunk_count].result = NULL;
        chunks[chunk_count].is_priority_block = 0;
        chunks[chunk_count].original_index = chunk_count;
        chunk_count++;
    }

    /* Sort by priority */
    qsort(chunks, chunk_count, sizeof(PChunk), compare_pchunks_priority);

    /* Process in priority order */
    for (int j = 0; j < chunk_count; j++) {
        if (chunks[j].is_priority_block) {
            chunks[j].result = papagaio_process_text(ctx, chunks[j].content, strlen(chunks[j].content));
        } else {
            const char *chunk_text = src + chunks[j].start;
            size_t chunk_len = chunks[j].end - chunks[j].start;
            chunks[j].result = papagaio_process_text(ctx, chunk_text, chunk_len);
        }
    }

    /* Sort back to original order for reassembly */
    qsort(chunks, chunk_count, sizeof(PChunk), compare_pchunks_original);

    StrBuf out; sb_init(&out);
    for (int j = 0; j < chunk_count; j++) {
        if (chunks[j].result) {
            sb_append_n(&out, chunks[j].result, strlen(chunks[j].result));
            free(chunks[j].result);
        }
        if (chunks[j].content) free(chunks[j].content);
    }
    free(chunks);
    
    char *final_res = strdup(out.data);
    sb_free(&out);
    return final_res;
}

char *papagaio_process_text(Papagaio *ctx, const char *input, size_t len)
{
    if (!ctx || !input) return NULL;
    Symbols sym = make_symbols(PAP_SIGIL, PAP_OPEN, PAP_CLOSE);

    ctx->depth++;
    if (ctx->depth == 1) {
        if (ctx->rules) {
            for (int i = 0; i < ctx->rule_count; i++) {
                free(ctx->rules[i].m);
                free(ctx->rules[i].r);
            }
            ctx->rule_count = 0;
        }
        if (ctx->original_doc) free(ctx->original_doc);
        ctx->original_doc = (char*)malloc(len + 1);
        if (ctx->original_doc) {
            memcpy(ctx->original_doc, input, len);
            ctx->original_doc[len] = '\0';
            ctx->original_len = len;
        } else {
            ctx->original_len = 0;
        }
    }

    /* 0. Handle Priorities */
    char *prio_res = handle_priorities(ctx, input, len, &sym);
    if (prio_res) {
        ctx->depth--;
        return prio_res;
    }

    char *buf = (char *)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, input, len); buf[len] = '\0';

    char *preprocessed = resolve_preprocessor(ctx, buf, &sym); free(buf);
    if (!preprocessed) return NULL;
    
    /* A. Resolve Patterns - Now adds to persistent ctx->rules */
    PatternPair *new_pairs = NULL; int new_pc = 0;
    char *text_no_patterns = extract_nested(preprocessed, &sym, &new_pairs, &new_pc);
    free(preprocessed);
    if (!text_no_patterns) { free_pairs(new_pairs, new_pc); return NULL; }

    if (new_pc > 0) {
        if (ctx->rule_count + new_pc > ctx->rule_cap) {
            ctx->rule_cap = ctx->rule_cap ? (ctx->rule_cap + new_pc) * 2 : (new_pc + 8);
            ctx->rules = (PatternPair *)realloc(ctx->rules, sizeof(PatternPair) * ctx->rule_cap);
        }
        for (int i = 0; i < new_pc; i++) {
            ctx->rules[ctx->rule_count++] = new_pairs[i];
        }
        free(new_pairs); /* Don't use free_pairs as we moved the strings */
    }
    
    char *cur = text_no_patterns;
    /* Apply ALL rules (both just found and previous persistent ones) */
    for (int i = 0; i < ctx->rule_count; i++) {
        StrBuf out; sb_init(&out);
        size_t clen = strlen(cur), pos = 0;
        Pattern pat;
        parse_pattern_ex(ctx->rules[i].m, &pat, &sym);
        
        while (pos < clen) {
            Match m; m.ctx = ctx;
            if (match_pattern(ctx, cur, (int)clen, &pat, (int)pos, &m)) {
                char *r = apply_replacement_ex(ctx->rules[i].r, &m, &sym);
                sb_append_n(&out, r, strlen(r));
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

    char *final = dispatch_commands(ctx, cur, &sym);
    free(cur);
    
    ctx->depth--;
    return final;
}

