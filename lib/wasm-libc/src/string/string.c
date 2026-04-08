#include <string.h>
#include <stdint.h>

/* ── memória ─────────────────────────────────────────────────────────────── */

/*
 * memcpy: cópia sem overlap garantido.
 * Otimização: copia word-by-word quando ambos os ponteiros estão alinhados.
 */
void *memcpy(void *dst, const void *src, size_t n) {
    unsigned char       *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    /* fast path: alinhamento 4 bytes */
    if (n >= 4 && ((uintptr_t)d & 3) == 0 && ((uintptr_t)s & 3) == 0) {
        uint32_t       *dw = (uint32_t *)d;
        const uint32_t *sw = (const uint32_t *)s;
        size_t words = n >> 2;
        while (words--) *dw++ = *sw++;
        d = (unsigned char *)dw;
        s = (const unsigned char *)sw;
        n &= 3;
    }

    while (n--) *d++ = *s++;
    return dst;
}

/*
 * memmove: cópia segura com overlap.
 * Copia de trás para frente quando dst > src e regiões se sobrepõem.
 */
void *memmove(void *dst, const void *src, size_t n) {
    unsigned char       *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s || n == 0) return dst;

    if (d < s || d >= s + n) {
        /* sem overlap ou dst antes de src: cópia forward */
        while (n--) *d++ = *s++;
    } else {
        /* overlap com dst depois de src: cópia backward */
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

/*
 * memset: preenche n bytes com o valor c.
 * Otimização: expande c para word de 32 bits e preenche em blocos.
 */
void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    unsigned char  b = (unsigned char)c;

    /* fast path word-aligned */
    if (n >= 4 && ((uintptr_t)p & 3) == 0) {
        uint32_t w = (uint32_t)b;
        w |= w << 8;
        w |= w << 16;
        uint32_t *pw = (uint32_t *)p;
        size_t words = n >> 2;
        while (words--) *pw++ = w;
        p = (unsigned char *)pw;
        n &= 3;
    }

    while (n--) *p++ = b;
    return s;
}

/*
 * memcmp: compara n bytes. Retorna <0, 0 ou >0.
 */
int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    while (n--) {
        if (*a != *b) return (int)*a - (int)*b;
        a++; b++;
    }
    return 0;
}

/*
 * memchr: busca byte c nos primeiros n bytes.
 */
void *memchr(const void *s, int c, size_t n) {
    const unsigned char *p = (const unsigned char *)s;
    unsigned char        b = (unsigned char)c;
    while (n--) {
        if (*p == b) return (void *)p;
        p++;
    }
    return (void *)0;
}

#include <stdlib.h>

/* ── comprimento ─────────────────────────────────────────────────────────── */

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

size_t strnlen(const char *s, size_t maxlen) {
    const char *p = s;
    while (maxlen-- && *p) p++;
    return (size_t)(p - s);
}

char *strdup(const char *s) {
    size_t len = strlen(s);
    char *res = (char *)malloc(len + 1);
    if (res) memcpy(res, s, len + 1);
    return res;
}

/* ── cópia ───────────────────────────────────────────────────────────────── */

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

/*
 * strncpy: copia até n bytes; se src < n, preenche o resto com '\0'.
 * Nota: NÃO garante nul-terminação se src tem exatamente n bytes.
 */
char *strncpy(char *dst, const char *src, size_t n) {
    char *d = dst;
    while (n && (*d++ = *src++)) n--;
    while (n--) *d++ = '\0';   /* padding com zeros */
    return dst;
}

/* ── concatenação ────────────────────────────────────────────────────────── */

char *strcat(char *dst, const char *src) {
    char *d = dst + strlen(dst);
    while ((*d++ = *src++));
    return dst;
}

char *strncat(char *dst, const char *src, size_t n) {
    char *d = dst + strlen(dst);
    while (n-- && *src) *d++ = *src++;
    *d = '\0';
    return dst;
}

/* ── comparação ──────────────────────────────────────────────────────────── */

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n-- && *s1 && (*s1 == *s2)) { s1++; s2++; }
    if (n == (size_t)-1) return 0;
    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

/* ── busca ───────────────────────────────────────────────────────────────── */

char *strchr(const char *s, int c) {
    char b = (char)c;
    while (*s) {
        if (*s == b) return (char *)s;
        s++;
    }
    return b == '\0' ? (char *)s : (char *)0;
}

char *strrchr(const char *s, int c) {
    char        b    = (char)c;
    const char *last = (char *)0;
    while (*s) {
        if (*s == b) last = s;
        s++;
    }
    return b == '\0' ? (char *)s : (char *)last;
}

/*
 * strstr: algoritmo naive O(n*m).
 * Para padrões longos um Knuth-Morris-Pratt valeria mais, mas
 * para uma libc bare-metal o naive é mais simples e auditável.
 */
char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    size_t nlen = strlen(needle);
    while (*haystack) {
        if (*haystack == *needle && memcmp(haystack, needle, nlen) == 0)
            return (char *)haystack;
        haystack++;
    }
    return (char *)0;
}

/* ── span / break ────────────────────────────────────────────────────────── */

size_t strspn(const char *s, const char *accept) {
    const char *p = s;
    while (*p) {
        const char *a = accept;
        while (*a && *a != *p) a++;
        if (!*a) break;
        p++;
    }
    return (size_t)(p - s);
}

size_t strcspn(const char *s, const char *reject) {
    const char *p = s;
    while (*p) {
        const char *r = reject;
        while (*r) {
            if (*r == *p) return (size_t)(p - s);
            r++;
        }
        p++;
    }
    return (size_t)(p - s);
}

char *strpbrk(const char *s, const char *accept) {
    s += strcspn(s, accept);
    return *s ? (char *)s : (char *)0;
}

/* ── tokenização ─────────────────────────────────────────────────────────── */

/*
 * strtok_r: versão reentrante (thread-safe quando há um saveptr por thread).
 * strtok usa um saveptr estático — não é reentrante.
 */
char *strtok_r(char *str, const char *delim, char **saveptr) {
    char *s = str ? str : *saveptr;

    /* pula delimitadores iniciais */
    s += strspn(s, delim);
    if (!*s) { *saveptr = s; return (char *)0; }

    /* encontra fim do token */
    char *end = s + strcspn(s, delim);
    if (*end) {
        *end     = '\0';
        *saveptr = end + 1;
    } else {
        *saveptr = end;
    }
    return s;
}

char *strtok(char *str, const char *delim) {
    static char *saved = (char *)0;
    return strtok_r(str, delim, &saved);
}

/* ── utilitários extras ───────────────────────────────────────────────────── */

char *strrev(char *s) {
    char *a = s;
    char *b = s + strlen(s) - 1;
    while (a < b) {
        char tmp = *a;
        *a++ = *b;
        *b-- = tmp;
    }
    return s;
}
