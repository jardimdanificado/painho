#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* import do host para abort */
__attribute__((import_module("env"), import_name("__host_abort")))
extern void __host_abort(const char *msg);

/* ── conversão de strings ────────────────────────────────────────────────── */

/*
 * strtoll: base da família strtol.
 * Suporta prefixos 0x (hex) e 0 (octal) quando base == 0.
 */
long long strtoll(const char *s, char **endptr, int base) {
    /* pula espaços */
    while (*s == ' ' || *s == '\t' || *s == '\n' ||
           *s == '\r' || *s == '\f' || *s == '\v') s++;

    int neg = 0;
    if (*s == '+') s++;
    else if (*s == '-') { neg = 1; s++; }

    /* infere base pelo prefixo */
    if (base == 0) {
        if (*s == '0') {
            if (s[1] == 'x' || s[1] == 'X') { base = 16; s += 2; }
            else { base = 8; s++; }
        } else base = 10;
    } else if (base == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
    }

    uint64_t result = 0;
    int any = 0;
    while (1) {
        int digit;
        char c = *s;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'z') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z') digit = c - 'A' + 10;
        else break;
        if (digit >= base) break;
        result = result * (uint64_t)base + (uint64_t)digit;
        any = 1;
        s++;
    }

    if (endptr) *endptr = any ? (char *)s : (char *)(s - (neg ? 1 : 0));

    long long v = (long long)result;
    return neg ? -v : v;
}

long strtol(const char *s, char **endptr, int base) {
    return (long)strtoll(s, endptr, base);
}

unsigned long long strtoull(const char *s, char **endptr, int base) {
    return (unsigned long long)strtoll(s, endptr, base);
}

unsigned long strtoul(const char *s, char **endptr, int base) {
    return (unsigned long)strtoull(s, endptr, base);
}

long long atoll(const char *s) { return strtoll(s, (char **)0, 10); }
long      atol (const char *s) { return strtol (s, (char **)0, 10); }
int       atoi (const char *s) { return (int)atol(s); }

/* ── itoa ────────────────────────────────────────────────────────────────── */

char *itoa(int value, char *str, int base) {
    if (base < 2 || base > 36) { *str = '\0'; return str; }

    char *p    = str;
    char *rev  = str;
    unsigned int uval;

    if (base == 10 && value < 0) {
        *p++ = '-';
        rev++;
        uval = (unsigned int)(-(value + 1)) + 1u; /* evita UB no INT_MIN */
    } else {
        uval = (unsigned int)value;
    }

    do {
        int d = (int)(uval % (unsigned)base);
        *p++ = (char)(d < 10 ? '0' + d : 'a' + d - 10);
        uval /= (unsigned)base;
    } while (uval);

    *p-- = '\0';

    /* inverte a parte numérica */
    while (rev < p) {
        char tmp = *rev;
        *rev++ = *p;
        *p--   = tmp;
    }
    return str;
}

/* ── aritmética ──────────────────────────────────────────────────────────── */

int       abs  (int x)       { return x < 0 ? -x : x; }
long      labs (long x)      { return x < 0 ? -x : x; }
long long llabs(long long x) { return x < 0 ? -x : x; }

div_t div(int numer, int denom) {
    div_t r;
    r.quot = numer / denom;
    r.rem  = numer % denom;
    return r;
}

ldiv_t ldiv(long numer, long denom) {
    ldiv_t r;
    r.quot = numer / denom;
    r.rem  = numer % denom;
    return r;
}

lldiv_t lldiv(long long numer, long long denom) {
    lldiv_t r;
    r.quot = numer / denom;
    r.rem  = numer % denom;
    return r;
}

/* ── rand: xorshift32 (rápido, zero dependências) ────────────────────────── */

static uint32_t rand_state = 12345678u;

int rand(void) {
    uint32_t x = rand_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rand_state = x;
    return (int)(x & (uint32_t)RAND_MAX);
}

void srand(unsigned int seed) {
    rand_state = seed ? seed : 1u;
}

/* ── abort ───────────────────────────────────────────────────────────────── */

void abort(void) {
    __host_abort("abort() called");
    /* unreachable — faz o compilador não reclamar de função noreturn */
    while (1) {}
}

/* ── qsort: introsort simplificado (insertion + quicksort) ──────────────── */

/*
 * Insertion sort para sub-arrays pequenos (n <= ISORT_THRESH).
 * Muito eficiente para n pequeno por ter overhead zero.
 */
#define ISORT_THRESH 16

static void swap_bytes(char *a, char *b, size_t size) {
    while (size--) {
        char tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    }
}

static void isort(char *base, size_t n, size_t size,
                  int (*cmp)(const void *, const void *)) {
    for (size_t i = 1; i < n; i++) {
        char *key = base + i * size;
        char *j   = base + i * size;
        while (j > base && cmp(j - size, j) > 0) {
            swap_bytes(j - size, j, size);
            j -= size;
        }
        (void)key;
    }
}

/*
 * Quicksort com pivot mediana-de-3.
 * Degrada para insertion sort em arrays pequenos.
 */
static void qsort_r_impl(char *base, size_t n, size_t size,
                          int (*cmp)(const void *, const void *)) {
    if (n <= 1) return;
    if (n <= ISORT_THRESH) { isort(base, n, size, cmp); return; }

    /* mediana de 3 como pivot */
    char *mid  = base + (n / 2) * size;
    char *last = base + (n - 1) * size;
    if (cmp(base, mid)  > 0) swap_bytes(base, mid,  size);
    if (cmp(base, last) > 0) swap_bytes(base, last, size);
    if (cmp(mid, last)  > 0) swap_bytes(mid,  last, size);
    /* pivot está em mid — move para penúltima posição */
    swap_bytes(mid, last, size);
    char *pivot = last;

    /* partição de Lomuto */
    char *i = base - size;
    for (char *j = base; j < last; j += size) {
        if (cmp(j, pivot) <= 0) {
            i += size;
            swap_bytes(i, j, size);
        }
    }
    i += size;
    swap_bytes(i, pivot, size);

    size_t left  = (size_t)(i - base) / size;
    size_t right = n - left - 1;
    qsort_r_impl(base, left, size, cmp);
    qsort_r_impl(i + size, right, size, cmp);
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*cmp)(const void *, const void *)) {
    if (nmemb < 2 || size == 0) return;
    qsort_r_impl((char *)base, nmemb, size, cmp);
}

/* ── bsearch ─────────────────────────────────────────────────────────────── */

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*cmp)(const void *, const void *)) {
    const char *lo = (const char *)base;
    const char *hi = lo + nmemb * size;
    while (lo < hi) {
        const char *mid = lo + ((size_t)(hi - lo) / size / 2) * size;
        int c = cmp(key, mid);
        if      (c < 0) hi  = mid;
        else if (c > 0) lo  = mid + size;
        else            return (void *)mid;
    }
    return (void *)0;
}
