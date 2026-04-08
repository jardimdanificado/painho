/*
 * scanf.c — implementação bare-metal de scanf/sscanf para WASM
 *
 * Especificadores suportados:
 *   %d %i   — inteiro decimal/auto com sinal     → int *
 *   %u      — inteiro sem sinal                  → unsigned int *
 *   %o      — octal                              → unsigned int *
 *   %x %X   — hexadecimal                        → unsigned int *
 *   %f %e %g — ponto flutuante                   → float *
 *   %lf %le %lg — ponto flutuante (double)       → double *
 *   %s      — string (para no whitespace)        → char *
 *   %c      — caractere único (não pula espaço)  → char *
 *   %[...]  — scanset                            → char *
 *   %n      — conta chars consumidos             → int *
 *   %%      — literal '%'
 *
 * Modificadores:
 *   *       — suprime atribuição (lê mas descarta)
 *   largura — número máximo de chars a consumir
 *   h hh    — short / char
 *   l ll    — long / long long / double
 *   L       — long double (tratado como double)
 *
 * scanf: lê de um buffer de linha importado do host (__host_readline).
 * sscanf: lê de string em memória — sem imports.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* ── fonte de caracteres ─────────────────────────────────────────────────── */

typedef struct {
    const char *buf;   /* NULL = leitura de host via __host_readline */
    int         pos;
    int         len;
    int         consumed;
} scan_src_t;

/* import do host para scanf interativo — opcional */
__attribute__((import_module("env"), import_name("__host_readline")))
extern int __host_readline(char *buf, int maxlen);

#define SCANF_BUF_SIZE 1024
static char _scanf_buf[SCANF_BUF_SIZE];
static int  _scanf_pos = 0;
static int  _scanf_len = 0;

static int src_getc(scan_src_t *s) {
    int c = -1;
    if (s->buf) {
        if (s->pos < s->len) c = (unsigned char)s->buf[s->pos++];
    } else {
        /* stdin via host */
        if (_scanf_pos >= _scanf_len) {
            _scanf_len = __host_readline(_scanf_buf, SCANF_BUF_SIZE - 1);
            _scanf_pos = 0;
            if (_scanf_len <= 0) return -1;
            _scanf_buf[_scanf_len] = '\0';
        }
        c = (unsigned char)_scanf_buf[_scanf_pos++];
    }
    if (c != -1) s->consumed++;
    return c;
}

static void src_ungetc(scan_src_t *s) {
    if (s->buf) { if (s->pos > 0) { s->pos--; s->consumed--; } }
    else        { if (_scanf_pos > 0) { _scanf_pos--; s->consumed--; } }
}

/* ── parse de número inteiro ─────────────────────────────────────────────── */

static int scan_int(scan_src_t *s, int base, int width,
                    unsigned long long *out, int *negative) {
    int c, n = 0;
    *negative = 0;
    unsigned long long val = 0;

    /* sinal */
    c = src_getc(s);
    if (c == '-') { *negative = 1; c = src_getc(s); n++; }
    else if (c == '+') { c = src_getc(s); n++; }

    /* prefixo 0x / 0 */
    if (base == 0 || base == 16) {
        if (c == '0') {
            int c2 = src_getc(s);
            if (c2 == 'x' || c2 == 'X') {
                base = 16; c = src_getc(s); n += 2;
            } else {
                src_ungetc(s);
                if (base == 0) base = 8;
            }
        } else if (base == 0) base = 10;
    }

    /* dígitos */
    int got_digit = 0;
    while (c != -1 && (width <= 0 || n < width)) {
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else { break; }
        if (digit >= base) { break; }
        val = val * (unsigned long long)base + (unsigned long long)digit;
        got_digit = 1;
        n++;
        c = src_getc(s);
    }
    if (c != -1) src_ungetc(s);
    *out = val;
    return got_digit ? n : 0;
}

/* ── parse de número de ponto flutuante ──────────────────────────────────── */

/* forward declaration — definida mais abaixo neste arquivo */
static double strtod_local(const char *s, char **endptr);

static int scan_float(scan_src_t *s, int width, double *out) {
    char buf[64];
    int  n = 0, c;

    c = src_getc(s);
    if (c == '-' || c == '+') { buf[n++] = (char)c; c = src_getc(s); }

    /* nan / inf */
    if (c == 'n' || c == 'N') {
        buf[n++] = (char)c; c = src_getc(s);
        if (c == 'a' || c == 'A') { buf[n++] = (char)c; c = src_getc(s); }
        if (c == 'n' || c == 'N') { buf[n++] = (char)c; } else src_ungetc(s);
        buf[n] = '\0'; *out = 0.0/0.0; return n;
    }
    if (c == 'i' || c == 'I') {
        buf[n++] = (char)c; c = src_getc(s);
        if (c == 'n' || c == 'N') { buf[n++] = (char)c; c = src_getc(s); }
        if (c == 'f' || c == 'F') { buf[n++] = (char)c; } else src_ungetc(s);
        buf[n] = '\0'; *out = 1.0/0.0 * (buf[0]=='-' ? -1.0 : 1.0); return n;
    }

    /* parte inteira */
    while (c >= '0' && c <= '9' && (width <= 0 || n < width)) {
        buf[n++] = (char)c; c = src_getc(s);
    }
    /* parte fracionária */
    if (c == '.' && (width <= 0 || n < width)) {
        buf[n++] = (char)c; c = src_getc(s);
        while (c >= '0' && c <= '9' && (width <= 0 || n < width)) {
            buf[n++] = (char)c; c = src_getc(s);
        }
    }
    /* expoente */
    if ((c == 'e' || c == 'E') && (width <= 0 || n < width)) {
        buf[n++] = (char)c; c = src_getc(s);
        if ((c == '+' || c == '-') && (width <= 0 || n < width)) {
            buf[n++] = (char)c; c = src_getc(s);
        }
        while (c >= '0' && c <= '9' && (width <= 0 || n < width)) {
            buf[n++] = (char)c; c = src_getc(s);
        }
    }
    if (c != -1) src_ungetc(s);
    if (n == 0) return 0;
    buf[n] = '\0';
    *out = strtod_local(buf, (char **)0);
    return n;
}

/* strtod: wrapper sobre strtoll + lógica de ponto flutuante */
static double strtod_local(const char *s, char **endptr) {
    /* pula espaços */
    while (isspace((unsigned char)*s)) s++;
    const char *start = s;
    int neg = 0;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;

    double result = 0.0;
    while (*s >= '0' && *s <= '9') {
        result = result * 10.0 + (double)(*s - '0');
        s++;
    }
    if (*s == '.') {
        s++;
        double frac = 0.1;
        while (*s >= '0' && *s <= '9') {
            result += (double)(*s - '0') * frac;
            frac *= 0.1;
            s++;
        }
    }
    if (*s == 'e' || *s == 'E') {
        s++;
        int eneg = 0;
        if (*s == '-') { eneg = 1; s++; }
        else if (*s == '+') s++;
        int exp = 0;
        while (*s >= '0' && *s <= '9') { exp = exp*10 + (*s - '0'); s++; }
        /* multiplica por 10^exp */
        double base = 10.0;
        while (exp > 0) {
            if (exp & 1) result = eneg ? result/base : result*base;
            base *= base;
            exp >>= 1;
        }
    }
    if (endptr) *endptr = (char *)(s == start ? start : s);
    return neg ? -result : result;
}

/* ── parse de scanset %[...] ─────────────────────────────────────────────── */

/*
 * Analisa o scanset depois do '[' e preenche uma tabela de 256 bits.
 * Retorna ponteiro para o char após o ']'.
 */
static const char *parse_scanset(const char *fmt, uint8_t table[32]) {
    int negate = 0;
    memset(table, 0, 32);

    if (*fmt == '^') { negate = 1; fmt++; }

    /* ']' imediatamente após '[' ou '[^' é literal */
    if (*fmt == ']') {
        table[']' / 8] |= (uint8_t)(1u << (']' % 8));
        fmt++;
    }

    while (*fmt && *fmt != ']') {
        unsigned char lo = (unsigned char)*fmt;
        if (fmt[1] == '-' && fmt[2] && fmt[2] != ']') {
            unsigned char hi = (unsigned char)fmt[2];
            for (unsigned char c = lo; c <= hi; c++)
                table[c / 8] |= (uint8_t)(1u << (c % 8));
            fmt += 3;
        } else {
            table[lo / 8] |= (uint8_t)(1u << (lo % 8));
            fmt++;
        }
    }
    if (*fmt == ']') fmt++;

    if (negate) {
        for (int i = 0; i < 32; i++) table[i] ^= 0xFF;
    }
    return fmt;
}

static int in_scanset(uint8_t table[32], int c) {
    if (c < 0 || c > 255) return 0;
    return (table[c / 8] >> (c % 8)) & 1;
}

/* ── engine principal ────────────────────────────────────────────────────── */

static int do_scan(scan_src_t *src, const char *fmt, va_list ap) {
    int assigned = 0;

    while (*fmt) {
        /* literal whitespace: consome qualquer quantidade de whitespace */
        if (isspace((unsigned char)*fmt)) {
            int c;
            while ((c = src_getc(src)) != -1 && isspace(c));
            if (c != -1) src_ungetc(src);
            while (isspace((unsigned char)*fmt)) fmt++;
            continue;
        }

        /* literal não-'%' */
        if (*fmt != '%') {
            int c = src_getc(src);
            if (c == -1) return assigned ? assigned : -1;
            if (c != (unsigned char)*fmt) { src_ungetc(src); return assigned; }
            /* src->consumed incrementado via src_getc */
            fmt++;
            continue;
        }
        fmt++; /* pula '%' */

        /* supressão */
        int suppress = 0;
        if (*fmt == '*') { suppress = 1; fmt++; }

        /* largura */
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') width = width*10 + (*fmt++ - '0');

        /* length modifier */
        int length = 0; /* 0=int 1=long 2=llong -1=short -2=char */
        if (*fmt == 'h') {
            fmt++;
            if (*fmt == 'h') { length = -2; fmt++; } else length = -1;
        } else if (*fmt == 'l') {
            fmt++;
            if (*fmt == 'l') { length = 2; fmt++; } else length = 1;
        } else if (*fmt == 'L') {
            length = 2; fmt++;
        }

        char spec = *fmt++;
        if (spec == '\0') break;

        /* pula whitespace antes de conversões (exceto %c e %[) */
        if (spec != 'c' && spec != '[' && spec != 'n') {
            int c;
            while ((c = src_getc(src)) != -1 && isspace(c));
            if (c != -1) src_ungetc(src);
            else if (spec != '%') return assigned ? assigned : -1;
        }

        switch (spec) {

        case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': {
            int base;
            switch (spec) {
                case 'o': base = 8;  break;
                case 'x': case 'X': base = 16; break;
                case 'i': base = 0;  break;
                default:  base = 10; break;
            }
            unsigned long long val;
            int neg;
            int n = scan_int(src, base, width, &val, &neg);
            if (n == 0) return assigned ? assigned : -1;
            /* src->consumed acumulado via src_getc */
            if (!suppress) {
                long long sval = neg ? -(long long)val : (long long)val;
                if (spec == 'u' || spec == 'o' || spec == 'x' || spec == 'X') {
                    if      (length ==  2) *va_arg(ap, unsigned long long *) = val;
                    else if (length ==  1) *va_arg(ap, unsigned long *)      = (unsigned long)val;
                    else if (length == -1) *va_arg(ap, unsigned short *)     = (unsigned short)val;
                    else if (length == -2) *va_arg(ap, unsigned char *)      = (unsigned char)val;
                    else                   *va_arg(ap, unsigned int *)        = (unsigned int)val;
                } else {
                    if      (length ==  2) *va_arg(ap, long long *) = sval;
                    else if (length ==  1) *va_arg(ap, long *)       = (long)sval;
                    else if (length == -1) *va_arg(ap, short *)      = (short)sval;
                    else if (length == -2) *va_arg(ap, signed char *) = (signed char)sval;
                    else                   *va_arg(ap, int *)          = (int)sval;
                }
                assigned++;
            }
            break;
        }

        case 'f': case 'e': case 'E': case 'g': case 'G': {
            double val;
            int n = scan_float(src, width, &val);
            if (n == 0) return assigned ? assigned : -1;
            /* src->consumed acumulado via src_getc */
            if (!suppress) {
                if (length >= 1) *va_arg(ap, double *) = val;
                else             *va_arg(ap, float *)  = (float)val;
                assigned++;
            }
            break;
        }

        case 'c': {
            int count = width > 0 ? width : 1;
            char *dst = suppress ? (char *)0 : va_arg(ap, char *);
            for (int i = 0; i < count; i++) {
                int c = src_getc(src);
                if (c == -1) return assigned ? assigned : -1;
                if (!suppress) dst[i] = (char)c;
                /* incrementado via src_getc */
            }
            if (!suppress) assigned++;
            break;
        }

        case 's': {
            char *dst = suppress ? (char *)0 : va_arg(ap, char *);
            int  n = 0, c;
            while ((c = src_getc(src)) != -1 &&
                   !isspace(c) &&
                   (width <= 0 || n < width)) {
                if (!suppress) dst[n] = (char)c;
                n++; /* incrementado via src_getc */
            }
            if (c != -1 && (isspace(c) || (width > 0 && n >= width)))
                src_ungetc(src);
            if (n == 0) return assigned ? assigned : -1;
            if (!suppress) { dst[n] = '\0'; assigned++; }
            break;
        }

        case '[': {
            uint8_t table[32];
            fmt = parse_scanset(fmt, table);
            char *dst = suppress ? (char *)0 : va_arg(ap, char *);
            int n = 0, c;
            while ((c = src_getc(src)) != -1 &&
                   in_scanset(table, c) &&
                   (width <= 0 || n < width)) {
                if (!suppress) dst[n] = (char)c;
                n++; /* incrementado via src_getc */
            }
            if (c != -1) src_ungetc(src);
            if (n == 0) return assigned ? assigned : -1;
            if (!suppress) { dst[n] = '\0'; assigned++; }
            break;
        }

        case 'n':
            if (!suppress) {
                if      (length ==  2) *va_arg(ap, long long *) = src->consumed;
                else if (length ==  1) *va_arg(ap, long *)      = (long)src->consumed;
                else if (length == -1) *va_arg(ap, short *)     = (short)src->consumed;
                else if (length == -2) *va_arg(ap, signed char *) = (signed char)src->consumed;
                else                   *va_arg(ap, int *)        = (int)src->consumed;
            }
            break;

        case '%': {
            int c = src_getc(src);
            if (c != '%') { src_ungetc(src); return assigned; }
            /* incrementado via src_getc */
            break;
        }

        default:
            break;
        }
    }
    return assigned;
}

/* ── API pública ─────────────────────────────────────────────────────────── */

int vsscanf(const char *buf, const char *fmt, va_list ap) {
    scan_src_t src = { buf, 0, (int)strlen(buf), 0 };
    return do_scan(&src, fmt, ap);
}

int sscanf(const char *buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsscanf(buf, fmt, ap);
    va_end(ap);
    return n;
}

int vscanf(const char *fmt, va_list ap) {
    scan_src_t src = { (char *)0, 0, 0, 0 };
    return do_scan(&src, fmt, ap);
}

int scanf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vscanf(fmt, ap);
    va_end(ap);
    return n;
}
