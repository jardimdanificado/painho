/*
 * printf.c — implementação bare-metal de printf para WASM
 *
 * Especificadores suportados:
 *   %d %i   — inteiro decimal com sinal
 *   %u      — inteiro decimal sem sinal
 *   %x %X   — hexadecimal (minúsculo / maiúsculo)
 *   %o      — octal
 *   %b      — binário (extensão)
 *   %c      — caractere
 *   %s      — string
 *   %p      — ponteiro (como %08x)
 *   %f      — float/double (implementação básica)
 *   %e %E   — notação científica
 *   %g %G   — escolhe %f ou %e automaticamente
 *   %%      — literal '%'
 *
 * Modificadores:
 *   - flags:  '-' (left-align), '0' (zero-fill), '+' (força sinal), ' ', '#'
 *   - largura: campo numérico ou '*'
 *   - precisão: '.' seguido de número ou '*'
 *   - length:  'l' (long), 'll' (long long), 'h' (short), 'hh' (char)
 *
 * Saída: vsnprintf escreve em buffer; printf chama __host_write.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ── contexto de escrita ─────────────────────────────────────────────────── */

typedef struct {
    char  *buf;
    size_t pos;
    size_t cap;     /* 0 = ilimitado (apenas conta) */
    int    to_host; /* 1 = flush direto para __host_write */
} fmt_ctx_t;

static void ctx_putc(fmt_ctx_t *ctx, char c) {
    if (ctx->buf && ctx->cap && ctx->pos < ctx->cap - 1)
        ctx->buf[ctx->pos] = c;
    ctx->pos++;
}

/* ── utilitários numéricos ───────────────────────────────────────────────── */

static const char hex_lower[] = "0123456789abcdef";
static const char hex_upper[] = "0123456789ABCDEF";

/* Converte inteiro sem sinal para string em `base`. Retorna comprimento. */
static int uint_to_str(uint64_t val, int base, const char *digits,
                        char *out, int max_out) {
    if (max_out < 2) return 0;
    if (val == 0) { out[0] = '0'; out[1] = '\0'; return 1; }

    char tmp[66];
    int  len = 0;
    while (val) {
        tmp[len++] = digits[val % (uint64_t)base];
        val /= (uint64_t)base;
    }
    /* inverte */
    for (int i = 0; i < len && i < max_out - 1; i++)
        out[i] = tmp[len - 1 - i];
    out[len] = '\0';
    return len;
}

/* Converte double para string decimal. Precisão = número de casas. */
static int double_to_str(double val, int prec, char *out, int max_out) {
    if (max_out < 4) return 0;
    int pos = 0;

    /* NaN / Inf */
    if (val != val)  { memcpy(out, "nan", 4); return 3; }
    if (val > 1e308) { memcpy(out, "inf", 4); return 3; }
    if (val <-1e308) { memcpy(out, "-inf", 5); return 4; }

    if (val < 0) { out[pos++] = '-'; val = -val; }

    /* parte inteira */
    uint64_t integer = (uint64_t)val;
    char     ibuf[24];
    int      ilen = uint_to_str(integer, 10, hex_lower, ibuf, (int)sizeof(ibuf));
    for (int i = 0; i < ilen && pos < max_out - 1; i++)
        out[pos++] = ibuf[i];

    if (prec <= 0) { out[pos] = '\0'; return pos; }

    out[pos++] = '.';

    /* parte fracionária */
    double frac = val - (double)integer;
    for (int i = 0; i < prec && pos < max_out - 1; i++) {
        frac *= 10.0;
        int digit = (int)frac;
        out[pos++] = (char)('0' + digit);
        frac -= (double)digit;
    }
    out[pos] = '\0';
    return pos;
}

/* ── engine de formatação ────────────────────────────────────────────────── */

#define FLAG_LEFT   0x01
#define FLAG_PLUS   0x02
#define FLAG_SPACE  0x04
#define FLAG_ZERO   0x08
#define FLAG_HASH   0x10

static void emit_padded(fmt_ctx_t *ctx, const char *s, int len,
                         int width, int flags, char pad_char) {
    int padding = width - len;

    if (!(flags & FLAG_LEFT) && padding > 0)
        for (int i = 0; i < padding; i++) ctx_putc(ctx, pad_char);

    for (int i = 0; i < len; i++) ctx_putc(ctx, s[i]);

    if ((flags & FLAG_LEFT) && padding > 0)
        for (int i = 0; i < padding; i++) ctx_putc(ctx, ' ');
}

static int do_fmt(fmt_ctx_t *ctx, const char *fmt, va_list ap) {
    while (*fmt) {
        if (*fmt != '%') { ctx_putc(ctx, *fmt++); continue; }
        fmt++; /* pula '%' */

        /* ── flags ── */
        int flags = 0;
        for (;;) {
            switch (*fmt) {
                case '-': flags |= FLAG_LEFT;  fmt++; continue;
                case '+': flags |= FLAG_PLUS;  fmt++; continue;
                case ' ': flags |= FLAG_SPACE; fmt++; continue;
                case '0': flags |= FLAG_ZERO;  fmt++; continue;
                case '#': flags |= FLAG_HASH;  fmt++; continue;
            }
            break;
        }

        /* ── largura ── */
        int width = 0;
        if (*fmt == '*') { width = va_arg(ap, int); fmt++; }
        else while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');

        /* ── precisão ── */
        int prec = -1;
        if (*fmt == '.') {
            fmt++;
            prec = 0;
            if (*fmt == '*') { prec = va_arg(ap, int); fmt++; }
            else while (*fmt >= '0' && *fmt <= '9') prec = prec * 10 + (*fmt++ - '0');
        }

        /* ── length modifier ── */
        int length = 0; /* 0=int, 1=long, 2=long long, -1=short, -2=char */
        if (*fmt == 'l') {
            fmt++;
            if (*fmt == 'l') { length = 2; fmt++; }
            else length = 1;
        } else if (*fmt == 'h') {
            fmt++;
            if (*fmt == 'h') { length = -2; fmt++; }
            else length = -1;
        } else if (*fmt == 'z') {
            length = 1; /* size_t é 32-bit no WASM, mas tratamos como long (32-bit tbm) */
            fmt++;
        }

        char spec = *fmt++;
        if (spec == '\0') break;

        /* ── especificadores ── */
        char tmp[128];
        int  tlen;
        char pad_char = (flags & FLAG_ZERO) && !(flags & FLAG_LEFT) ? '0' : ' ';

        switch (spec) {

        /* inteiros */
        case 'd': case 'i': {
            int64_t v;
            if      (length ==  2) v = va_arg(ap, long long);
            else if (length ==  1) v = va_arg(ap, long);
            else                   v = va_arg(ap, int);
            if (length == -1) v = (short)v;
            if (length == -2) v = (signed char)v;

            int  neg = (v < 0);
            uint64_t uv = neg ? (uint64_t)(-v) : (uint64_t)v;

            char nbuf[24];
            tlen = uint_to_str(uv, 10, hex_lower, nbuf, (int)sizeof(nbuf));

            /* precisão → zero-pad o número */
            char pbuf[32]; int plen = 0;
            char *sign = neg ? "-" : (flags & FLAG_PLUS) ? "+" :
                               (flags & FLAG_SPACE) ? " " : "";
            int signlen = (int)strlen(sign);

            for (int i = 0; i < (prec - tlen) && plen < (int)sizeof(pbuf)-1; i++)
                pbuf[plen++] = '0';
            for (int i = 0; i < tlen; i++) pbuf[plen++] = nbuf[i];
            pbuf[plen] = '\0';

            /* zero-padding inclui o sinal */
            if ((flags & FLAG_ZERO) && !(flags & FLAG_LEFT) && prec < 0) {
                for (int i = 0; i < signlen; i++) ctx_putc(ctx, sign[i]);
                int pad = width - plen - signlen;
                for (int i = 0; i < pad; i++) ctx_putc(ctx, '0');
                for (int i = 0; i < plen; i++) ctx_putc(ctx, pbuf[i]);
            } else {
                char full[40]; int flen = 0;
                for (int i = 0; i < signlen; i++) full[flen++] = sign[i];
                for (int i = 0; i < plen; i++) full[flen++] = pbuf[i];
                emit_padded(ctx, full, flen, width, flags, ' ');
            }
            continue;
        }

        case 'u': case 'x': case 'X': case 'o': case 'b': {
            uint64_t uv;
            if      (length ==  2) uv = va_arg(ap, unsigned long long);
            else if (length ==  1) uv = va_arg(ap, unsigned long);
            else                   uv = va_arg(ap, unsigned int);
            if (length == -1) uv = (unsigned short)uv;
            if (length == -2) uv = (unsigned char)uv;

            int base;
            const char *digits;
            const char *prefix = "";
            switch (spec) {
                case 'x': base = 16; digits = hex_lower;
                    if (flags & FLAG_HASH) { prefix = "0x"; } break;
                case 'X': base = 16; digits = hex_upper;
                    if (flags & FLAG_HASH) { prefix = "0X"; } break;
                case 'o': base = 8;  digits = hex_lower;
                    if (flags & FLAG_HASH) { prefix = "0"; }  break;
                case 'b': base = 2;  digits = hex_lower;
                    if (flags & FLAG_HASH) { prefix = "0b"; } break;
                default:  base = 10; digits = hex_lower; break;
            }

            char nbuf[68];
            tlen = uint_to_str(uv, base, digits, nbuf, (int)sizeof(nbuf));
            int prelen = (int)strlen(prefix);

            if ((flags & FLAG_ZERO) && !(flags & FLAG_LEFT)) {
                for (int i = 0; i < prelen; i++) ctx_putc(ctx, prefix[i]);
                int pad = width - tlen - prelen;
                for (int i = 0; i < pad; i++) ctx_putc(ctx, '0');
                for (int i = 0; i < tlen; i++) ctx_putc(ctx, nbuf[i]);
            } else {
                char full[80]; int flen = 0;
                for (int i = 0; i < prelen; i++) full[flen++] = prefix[i];
                for (int i = 0; i < tlen; i++) full[flen++] = nbuf[i];
                emit_padded(ctx, full, flen, width, flags, ' ');
            }
            continue;
        }

        case 'c': {
            char c = (char)va_arg(ap, int);
            emit_padded(ctx, &c, 1, width, flags, ' ');
            continue;
        }

        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s) s = "(null)";
            tlen = (int)strlen(s);
            if (prec >= 0 && tlen > prec) tlen = prec;
            emit_padded(ctx, s, tlen, width, flags, ' ');
            continue;
        }

        case 'p': {
            uintptr_t v = (uintptr_t)va_arg(ap, void *);
            tlen = uint_to_str((uint64_t)v, 16, hex_lower, tmp, (int)sizeof(tmp));
            /* padding com zeros até 8 dígitos */
            char full[16] = "0x00000000";
            int  off = 10 - tlen;
            for (int i = 0; i < tlen; i++) full[off + i] = tmp[i];
            emit_padded(ctx, full, 10, width, flags, ' ');
            continue;
        }

        case 'f': {
            double v = va_arg(ap, double);
            int p = prec < 0 ? 6 : prec;
            char sign = (v < 0) ? '-' :
                        (flags & FLAG_PLUS) ? '+' : 0;
            if (v < 0) v = -v;
            tlen = double_to_str(v, p, tmp, (int)sizeof(tmp));
            if (sign) {
                char full[130]; full[0] = sign;
                memcpy(full + 1, tmp, (size_t)tlen + 1);
                emit_padded(ctx, full, tlen + 1, width, flags, pad_char);
            } else {
                emit_padded(ctx, tmp, tlen, width, flags, pad_char);
            }
            continue;
        }

        /* notação científica simplificada */
        case 'e': case 'E': {
            double v = va_arg(ap, double);
            int    p = prec < 0 ? 6 : prec;
            int    exp = 0;
            if (v != 0.0) {
                while (v >= 10.0)  { v /= 10.0; exp++; }
                while (v <  1.0 && v > -1.0 && v != 0.0) { v *= 10.0; exp--; }
            }
            int n = double_to_str(v, p, tmp, 64);
            /* adiciona expoente */
            char emark = (spec == 'e') ? 'e' : 'E';
            char ebuf[8];
            int elen = 0;
            ebuf[elen++] = emark;
            ebuf[elen++] = exp < 0 ? '-' : '+';
            int ae = exp < 0 ? -exp : exp;
            if (ae < 10) ebuf[elen++] = '0';
            char expstr[8]; int explen = uint_to_str((uint64_t)ae, 10, hex_lower, expstr, 8);
            for (int i = 0; i < explen; i++) ebuf[elen++] = expstr[i];
            /* junta */
            char full[140]; int flen = 0;
            for (int i = 0; i < n; i++) full[flen++] = tmp[i];
            for (int i = 0; i < elen; i++) full[flen++] = ebuf[i];
            emit_padded(ctx, full, flen, width, flags, pad_char);
            continue;
        }

        case 'g': case 'G':
            /* simplificação: delega para %f quando prec não é muito pequeno */
            /* (implementação completa requereria mais lógica de expoente)    */
            {
                double v = va_arg(ap, double);
                int p = prec < 0 ? 6 : (prec == 0 ? 1 : prec);
                tlen = double_to_str(v, p, tmp, (int)sizeof(tmp));
                emit_padded(ctx, tmp, tlen, width, flags, pad_char);
            }
            continue;

        case 'n':
            if (length == 2)      *va_arg(ap, long long *) = (long long)ctx->pos;
            else if (length == 1) *va_arg(ap, long *)      = (long)ctx->pos;
            else                  *va_arg(ap, int *)       = (int)ctx->pos;
            continue;

        case '%':
            ctx_putc(ctx, '%');
            continue;

        default:
            ctx_putc(ctx, '%');
            ctx_putc(ctx, spec);
            continue;
        }
        (void)tlen;
    }
    return (int)ctx->pos;
}

/* ── funções públicas ────────────────────────────────────────────────────── */

int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
    fmt_ctx_t ctx = { buf, 0, size, 0 };
    int n = do_fmt(&ctx, fmt, ap);
    if (buf && size > 0) {
        size_t term = ctx.pos < size - 1 ? ctx.pos : size - 1;
        buf[term] = '\0';
    }
    return n;
}

int vsprintf(char *buf, const char *fmt, va_list ap) {
    return vsnprintf(buf, (size_t)-1, fmt, ap);
}

int vprintf(const char *fmt, va_list ap) {
    char buf[1024];
    int  n = vsnprintf(buf, sizeof(buf), fmt, ap);
    int  out = n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1;
    __host_write(buf, out);
    return n;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return n;
}

int sprintf(char *buf, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int n = vsprintf(buf, fmt, ap);
    va_end(ap);
    return n;
}

int printf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int n = vprintf(fmt, ap);
    va_end(ap);
    return n;
}

int fprintf_stdout(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int n = vprintf(fmt, ap);
    va_end(ap);
    return n;
}

int fprintf_stderr(const char *fmt, ...) {
    char buf[1024];
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    int out = n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1;
    __host_write_err(buf, out);
    return n;
}

int putchar(int c) {
    char b = (char)c;
    __host_write(&b, 1);
    return c;
}

int puts(const char *s) {
    int len = (int)strlen(s);
    __host_write(s, len);
    __host_write("\n", 1);
    return len + 1;
}
