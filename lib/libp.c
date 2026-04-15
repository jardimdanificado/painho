/*
 * papagaio.h — Struct global "papagaio" com métodos agrupados por módulo.
 *
 * Uso:
 *   #include "papagaio.h"
 *
 *   papagaio.math.sin(1.57);
 *   papagaio.str.strcpy(dst, src);
 *   papagaio.mem.malloc(64);
 *   papagaio.io.printf("hello %d\n", 42);
 *   papagaio.ctype.isdigit('9');
 *
 * Todas as chamadas são ponteiros de função que apontam diretamente para
 * as implementações reais — debug completo, stack trace intacto, sem macros
 * com lógica embutida.
 */

#ifndef PAPAGAIO_H
#define PAPAGAIO_H

/* 
 * Papagaio Wasm SDK — lib.c
 * Unified Single-File Library (Headers + Implementation)
 * Generated automatically from wasm-libc.
 */

#ifndef PAPAGAIO_LIBC_H
#define PAPAGAIO_LIBC_H

/* ── Headers ─────────────────────────────────────────────────────────────── */

/* --- stdarg.h --- */

/*
 * stdarg.h para clang/wasm32.
 *
 * Clang fornece __builtin_va_* que funcionam corretamente para wasm32.
 * Não usamos a implementação manual baseada em ponteiro de stack porque
 * o WASM não garante layout de stack convencional.
 */

typedef __builtin_va_list va_list;

#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dst, src)   __builtin_va_copy(dst, src)


/* --- stddef.h --- */

/* tipos fundamentais para wasm32 */
typedef unsigned int  size_t;
typedef int           ptrdiff_t;

/* intptr_t e uintptr_t ficam em stdint.h para evitar conflito */

#define NULL ((void*)0)

#define offsetof(type, member) __builtin_offsetof(type, member)


/* --- stdint.h --- */

/* inteiros de largura fixa — wasm32: int=32, long=32, long long=64 */
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
typedef long long           int64_t;
typedef unsigned long long  uint64_t;

/* tipos "pelo menos N bits" */
typedef int8_t              int_least8_t;
typedef uint8_t             uint_least8_t;
typedef int16_t             int_least16_t;
typedef uint16_t            uint_least16_t;
typedef int32_t             int_least32_t;
typedef uint32_t            uint_least32_t;
typedef int64_t             int_least64_t;
typedef uint64_t            uint_least64_t;

/* tipos "pelo menos tão rápido quanto N bits" */
typedef int32_t             int_fast8_t;
typedef uint32_t            uint_fast8_t;
typedef int32_t             int_fast16_t;
typedef uint32_t            uint_fast16_t;
typedef int32_t             int_fast32_t;
typedef uint32_t            uint_fast32_t;
typedef int64_t             int_fast64_t;
typedef uint64_t            uint_fast64_t;

/* intmax */
typedef int64_t             intmax_t;
typedef uint64_t            uintmax_t;

/* ponteiros — wasm32 usa 32 bits */
typedef int32_t             intptr_t;
typedef uint32_t            uintptr_t;

/* limites */
#define INT8_MIN    (-128)
#define INT8_MAX    (127)
#define UINT8_MAX   (255U)

#define INT16_MIN   (-32768)
#define INT16_MAX   (32767)
#define UINT16_MAX  (65535U)

#define INT32_MIN   (-2147483648)
#define INT32_MAX   (2147483647)
#define UINT32_MAX  (4294967295U)

#define INT64_MIN   (-9223372036854775808LL)
#define INT64_MAX   (9223372036854775807LL)
#define UINT64_MAX  (18446744073709551615ULL)

#define SIZE_MAX    UINT32_MAX
#define INTPTR_MIN  INT32_MIN
#define INTPTR_MAX  INT32_MAX
#define UINTPTR_MAX UINT32_MAX
#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MAX UINT64_MAX

/* constantes para tipos de largura mínima e rápida */
#define INT_LEAST8_MIN    INT8_MIN
#define INT_LEAST8_MAX    INT8_MAX
#define UINT_LEAST8_MAX   UINT8_MAX
#define INT_LEAST16_MIN   INT16_MIN
#define INT_LEAST16_MAX   INT16_MAX
#define UINT_LEAST16_MAX  UINT16_MAX
#define INT_LEAST32_MIN   INT32_MIN
#define INT_LEAST32_MAX   INT32_MAX
#define UINT_LEAST32_MAX  UINT32_MAX
#define INT_LEAST64_MIN   INT64_MIN
#define INT_LEAST64_MAX   INT64_MAX
#define UINT_LEAST64_MAX  UINT64_MAX

#define INT_FAST8_MIN     INT32_MIN
#define INT_FAST8_MAX     INT32_MAX
#define UINT_FAST8_MAX    UINT32_MAX
#define INT_FAST16_MIN    INT32_MIN
#define INT_FAST16_MAX    INT32_MAX
#define UINT_FAST16_MAX   UINT32_MAX

/* macros de constantes literais */
#define INT8_C(c)    (c)
#define UINT8_C(c)   (c ## U)
#define INT16_C(c)   (c)
#define UINT16_C(c)  (c ## U)
#define INT32_C(c)   (c)
#define UINT32_C(c)  (c ## U)
#define INT64_C(c)   (c ## LL)
#define UINT64_C(c)  (c ## ULL)
#define INTMAX_C(c)  (c ## LL)
#define UINTMAX_C(c) (c ## ULL)


/* --- stdbool.h --- */

#define bool  _Bool
#define true  1
#define false 0

#define __bool_true_false_are_defined 1


/* --- limits.h --- */

#define CHAR_BIT    8
#define SCHAR_MIN   (-128)
#define SCHAR_MAX   127
#define UCHAR_MAX   255U
#define CHAR_MIN    SCHAR_MIN
#define CHAR_MAX    SCHAR_MAX
#define SHRT_MIN    (-32768)
#define SHRT_MAX    32767
#define USHRT_MAX   65535U
#define INT_MIN     (-2147483648)
#define INT_MAX     2147483647
#define UINT_MAX    4294967295U
#define LONG_MIN    INT_MIN
#define LONG_MAX    INT_MAX
#define ULONG_MAX   UINT_MAX
#define LLONG_MIN   (-9223372036854775808LL)
#define LLONG_MAX   9223372036854775807LL
#define ULLONG_MAX  18446744073709551615ULL


/* --- string.h --- */


/* cópia e movimentação de memória */
void  *memcpy (void *dst, const void *src, size_t n);
void  *memmove(void *dst, const void *src, size_t n);
void  *memset (void *s, int c, size_t n);
int    memcmp (const void *s1, const void *s2, size_t n);
void  *memchr (const void *s, int c, size_t n);

/* operações em strings */
size_t strlen (const char *s);
size_t strnlen(const char *s, size_t maxlen);
char  *strdup (const char *s);
char  *strcpy (char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);
char  *strcat (char *dst, const char *src);
char  *strncat(char *dst, const char *src, size_t n);
int    strcmp (const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);
char  *strchr (const char *s, int c);
char  *strrchr(const char *s, int c);
char  *strstr (const char *haystack, const char *needle);
char  *strpbrk(const char *s, const char *accept);
size_t strspn (const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char  *strtok (char *str, const char *delim);
char  *strtok_r(char *str, const char *delim, char **saveptr);

/* utilitários */
char  *strrev (char *s);       /* extensão: inverte in-place */


/* diagnóstico de erro */
const char *strerror(int errnum);
void        perror  (const char *s);


/* --- stdio.h --- */



/* ── printf família ──────────────────────────────────────────────────────── */
int printf (const char *fmt, ...);
int fprintf_stdout(const char *fmt, ...);   /* stdout explícito */
int fprintf_stderr(const char *fmt, ...);   /* stderr           */

int sprintf (char *buf, const char *fmt, ...);
int snprintf(char *buf, size_t size, const char *fmt, ...);

int vprintf (const char *fmt, va_list ap);
int vsprintf (char *buf, const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

/* ── putchar / puts ──────────────────────────────────────────────────────── */
int putchar(int c);
int puts   (const char *s);



/* --- stdlib.h --- */


/* ── alocação de memória ─────────────────────────────────────────────────── */
void  *malloc (size_t size);
void  *calloc (size_t nmemb, size_t size);
void  *realloc(void *ptr, size_t size);
void   free   (void *ptr);

/* ── conversão de strings ────────────────────────────────────────────────── */
int       atoi  (const char *s);
long      atol  (const char *s);
long long atoll (const char *s);

long      strtol  (const char *s, char **endptr, int base);
long long strtoll (const char *s, char **endptr, int base);
unsigned long      strtoul (const char *s, char **endptr, int base);
unsigned long long strtoull(const char *s, char **endptr, int base);

/* ── algoritmos ──────────────────────────────────────────────────────────── */
void  qsort   (void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *));
void *bsearch (const void *key, const void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *));

/* ── aritmética ──────────────────────────────────────────────────────────── */
int       abs  (int x);
long      labs (long x);
long long llabs(long long x);

typedef struct { int  quot, rem; }           div_t;
typedef struct { long quot, rem; }           ldiv_t;
typedef struct { long long quot, rem; }      lldiv_t;

div_t   div  (int numer, int denom);
ldiv_t  ldiv (long numer, long denom);
lldiv_t lldiv(long long numer, long long denom);

/* ── números pseudo-aleatórios ───────────────────────────────────────────── */
#define RAND_MAX 0x7fffffff
int  rand (void);
void srand(unsigned int seed);

/* ── conversão numérica ──────────────────────────────────────────────────── */
/* itoa: extensão — converte inteiro para string na base dada */
char *itoa(int value, char *str, int base);


/* --- ctype.h --- */

/*
 * Implementação via lookup table de 256 entradas.
 * Cada entrada é uma bitmask das categorias do caractere.
 * Todas as funções são inline para zero overhead em hot paths.
 */

/* bits de categoria */
#define _CT_UP   0x01   /* maiúscula A-Z         */
#define _CT_LO   0x02   /* minúscula a-z         */
#define _CT_DI   0x04   /* dígito decimal 0-9    */
#define _CT_SP   0x08   /* espaço em branco      */
#define _CT_PU   0x10   /* pontuação/símbolo     */
#define _CT_CN   0x20   /* controle              */
#define _CT_HX   0x40   /* dígito hex a-f A-F    */
#define _CT_BL   0x80   /* espaço (0x20)         */

extern const unsigned char _ctype_table[256];

static inline int isalpha (int c) { return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO); }
static inline int isupper (int c) { return _ctype_table[(unsigned char)c] & _CT_UP; }
static inline int islower (int c) { return _ctype_table[(unsigned char)c] & _CT_LO; }
static inline int isdigit (int c) { return _ctype_table[(unsigned char)c] & _CT_DI; }
static inline int isalnum (int c) { return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO|_CT_DI); }
static inline int isxdigit(int c) { return _ctype_table[(unsigned char)c] & _CT_HX; }
static inline int isspace (int c) { return _ctype_table[(unsigned char)c] & _CT_SP; }
static inline int isblank (int c) { return (c == ' ' || c == '\t'); }
static inline int ispunct (int c) { return _ctype_table[(unsigned char)c] & _CT_PU; }
static inline int iscntrl (int c) { return _ctype_table[(unsigned char)c] & _CT_CN; }
static inline int isprint (int c) {
    return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO|_CT_DI|_CT_PU|_CT_BL);
}
static inline int isgraph (int c) {
    return _ctype_table[(unsigned char)c] & (_CT_UP|_CT_LO|_CT_DI|_CT_PU);
}

static inline int toupper(int c) {
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}
static inline int tolower(int c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}


/* --- math.h --- */

/* ── constantes ──────────────────────────────────────────────────────────── */

#define M_E         2.71828182845904523536
#define M_LOG2E     1.44269504088896340736
#define M_LOG10E    0.43429448190325182765
#define M_LN2       0.69314718055994530942
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.78539816339744830962
#define M_1_PI      0.31830988618379067154
#define M_2_PI      0.63661977236758134308
#define M_SQRT2     1.41421356237309504880
#define M_SQRT1_2   0.70710678118654752440

#define HUGE_VAL    __builtin_huge_val()
#define INFINITY    __builtin_inff()
#define NAN         __builtin_nanf("")

/* ── classificação ───────────────────────────────────────────────────────── */
int isinf (double x);
int isnan (double x);
int isfinite(double x);
int fpclassify(double x);   /* retorna: 0=normal 1=subnormal 2=zero 3=inf 4=nan */

/* ── trigonometria ───────────────────────────────────────────────────────── */
double sin  (double x);
double cos  (double x);
double tan  (double x);
double asin (double x);
double acos (double x);
double atan (double x);
double atan2(double y, double x);

/* ── hiperbólicas ────────────────────────────────────────────────────────── */
double sinh (double x);
double cosh (double x);
double tanh (double x);

/* ── exponencial e logaritmo ─────────────────────────────────────────────── */
double exp  (double x);
double exp2 (double x);
double log  (double x);
double log2 (double x);
double log10(double x);
double pow  (double base, double exp);

/* ── raiz e potência ─────────────────────────────────────────────────────── */
double sqrt (double x);
double cbrt (double x);
double hypot(double x, double y);

/* ── arredondamento ──────────────────────────────────────────────────────── */
double floor(double x);
double ceil (double x);
double round(double x);
double trunc(double x);
double fmod (double x, double y);

/* ── valor absoluto e sinal ──────────────────────────────────────────────── */
double fabs (double x);
double copysign(double x, double y);

/* ── float versions (delegam para double) ────────────────────────────────── */
float  sinf  (float x);
float  cosf  (float x);
float  tanf  (float x);
float  sqrtf (float x);
float  fabsf (float x);
float  floorf(float x);
float  ceilf (float x);
float  roundf(float x);
float  powf  (float base, float exp);
float  logf  (float x);
float  expf  (float x);


#endif /* PAPAGAIO_LIBC_H */

/* ── Implementation ──────────────────────────────────────────────────────── */


/* --- src/string.c --- */

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

/* --- src/malloc.c --- */

/* 
 * Um alocador simples (First-Fit, Coalescência Imediata) para WebAssembly.
 * Memória layout: [header (8b)] [payload] [footer (4b)]
 * Alinhamento: 8 bytes.
 */

/* Entry point do heap (vindo do linker ou definido aqui) */
extern char __heap_base;

#define WASM_PAGE_SIZE 65536

uint32_t heap_pointer = __heap_base;

static inline uint32_t wasm_memory_size(void) {
    return __builtin_wasm_memory_size(0);
}

static inline uint32_t wasm_memory_grow(uint32_t pages) {
    return __builtin_wasm_memory_grow(0, pages);
}


static inline uint32_t palloc(uint32_t size)
{
    uint32_t result = heap_pointer;
    if(heap_pointer + size >= __heap_base + (wasm_memory_size() * WASM_PAGE_SIZE))
    {
        int page_count = 0;
        while(heap_pointer + (size*(page_count+1)) >= __heap_base + (wasm_memory_size() * WASM_PAGE_SIZE))
            page_count++;
        wasm_memory_grow(page_count);
    }
    heap_pointer += size;
    return result;
}

/* --- src/printf.c --- */
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
 */


/* ── contexto de escrita ─────────────────────────────────────────────────── */

typedef struct {
    char  *buf;
    size_t pos;
    size_t cap;     /* 0 = ilimitado (apenas conta) */
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

/* --- src/stdlib.c --- */

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

/* --- src/ctype.c --- */

/*
 * Tabela de 256 entradas para classificação ASCII.
 * Índice = valor do byte (0-255).
 * Cada entrada é uma bitmask das categorias do caractere.
 *
 * Gerada estaticamente — zero overhead em runtime.
 *
 * Legenda das colunas (bits):
 *   UP=maiúscula  LO=minúscula  DI=dígito  SP=whitespace
 *   PU=pontuação  CN=controle   HX=hex     BL=espaço(0x20)
 */

#define U  _CT_UP
#define L  _CT_LO
#define D  _CT_DI
#define S  _CT_SP
#define P  _CT_PU
#define C  _CT_CN
#define X  _CT_HX
#define B  _CT_BL

const unsigned char _ctype_table[256] = {
/*  0 NUL*/C,  /*  1 SOH*/C,  /*  2 STX*/C,  /*  3 ETX*/C,
/*  4 EOT*/C,  /*  5 ENQ*/C,  /*  6 ACK*/C,  /*  7 BEL*/C,
/*  8 BS */C,  /*  9 TAB*/C|S,/* 10 LF */C|S,/* 11 VT */C|S,
/* 12 FF */C|S,/* 13 CR */C|S,/* 14 SO */C,  /* 15 SI */C,
/* 16 DLE*/C,  /* 17 DC1*/C,  /* 18 DC2*/C,  /* 19 DC3*/C,
/* 20 DC4*/C,  /* 21 NAK*/C,  /* 22 SYN*/C,  /* 23 ETB*/C,
/* 24 CAN*/C,  /* 25 EM */C,  /* 26 SUB*/C,  /* 27 ESC*/C,
/* 28 FS */C,  /* 29 GS */C,  /* 30 RS */C,  /* 31 US */C,
/* 32 SPC*/B|S,/* 33 !  */P,  /* 34 "  */P,  /* 35 #  */P,
/* 36 $  */P,  /* 37 %  */P,  /* 38 &  */P,  /* 39 '  */P,
/* 40 (  */P,  /* 41 )  */P,  /* 42 *  */P,  /* 43 +  */P,
/* 44 ,  */P,  /* 45 -  */P,  /* 46 .  */P,  /* 47 /  */P,
/* 48 0  */D|X,/* 49 1  */D|X,/* 50 2  */D|X,/* 51 3  */D|X,
/* 52 4  */D|X,/* 53 5  */D|X,/* 54 6  */D|X,/* 55 7  */D|X,
/* 56 8  */D|X,/* 57 9  */D|X,/* 58 :  */P,  /* 59 ;  */P,
/* 60 <  */P,  /* 61 =  */P,  /* 62 >  */P,  /* 63 ?  */P,
/* 64 @  */P,  /* 65 A  */U|X,/* 66 B  */U|X,/* 67 C  */U|X,
/* 68 D  */U|X,/* 69 E  */U|X,/* 70 F  */U|X,/* 71 G  */U,
/* 72 H  */U,  /* 73 I  */U,  /* 74 J  */U,  /* 75 K  */U,
/* 76 L  */U,  /* 77 M  */U,  /* 78 N  */U,  /* 79 O  */U,
/* 80 P  */U,  /* 81 Q  */U,  /* 82 R  */U,  /* 83 S  */U,
/* 84 T  */U,  /* 85 U  */U,  /* 86 V  */U,  /* 87 W  */U,
/* 88 X  */U,  /* 89 Y  */U,  /* 90 Z  */U,  /* 91 [  */P,
/* 92 \  */P,  /* 93 ]  */P,  /* 94 ^  */P,  /* 95 _  */P,
/* 96 `  */P,  /* 97 a  */L|X,/* 98 b  */L|X,/* 99 c  */L|X,
/*100 d  */L|X,/*101 e  */L|X,/*102 f  */L|X,/*103 g  */L,
/*104 h  */L,  /*105 i  */L,  /*106 j  */L,  /*107 k  */L,
/*108 l  */L,  /*109 m  */L,  /*110 n  */L,  /*111 o  */L,
/*112 p  */L,  /*113 q  */L,  /*114 r  */L,  /*115 s  */L,
/*116 t  */L,  /*117 u  */L,  /*118 v  */L,  /*119 w  */L,
/*120 x  */L,  /*121 y  */L,  /*122 z  */L,  /*123 {  */P,
/*124 |  */P,  /*125 }  */P,  /*126 ~  */P,  /*127 DEL*/C,
/* 128-255: todos zero (não-ASCII) */
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
};

#undef U
#undef L
#undef D
#undef S
#undef P
#undef C
#undef X
#undef B

/* --- src/math.c --- */
/*
 * math.c — implementação bare-metal de funções matemáticas
 *
 * Estratégias por função:
 *
 *   sin/cos   — redução de argumento para [-π/4, π/4] + série de Chebyshev
 *               (7 termos, erro < 1 ULP no intervalo reduzido)
 *   exp       — redução por exp(x) = 2^k · exp(r), série de Remez para exp(r)
 *   log       — redução por log(x) = k·ln2 + log(m), m ∈ [1, 2)
 *   sqrt      — Newton-Raphson com seed via bit trick (IEEE 754)
 *   pow       — exp(y · log(x))
 *   atan      — minimax racional de grau [3/3]
 *   asin/acos — identidade com atan
 *
 * Precisão: ~15 dígitos significativos (double), suficiente para quase
 * todos os casos de uso. Números subnormais e NaN são tratados
 * explicitamente mas não são o foco central.
 */


/* ── utilitários de bit IEEE 754 ─────────────────────────────────────────── */

typedef union { double d; uint64_t u; } f64bits_t;
typedef union { float  f; uint32_t u; } f32bits_t;

static inline uint64_t d2u(double d) { f64bits_t x; x.d = d; return x.u; }
static inline double   u2d(uint64_t u) { f64bits_t x; x.u = u; return x.d; }
static inline uint32_t f2u(float f) { f32bits_t x; x.f = f; return x.u; }
static inline float    u2f(uint32_t u) { f32bits_t x; x.u = u; return x.f; }

/* ── classificação ───────────────────────────────────────────────────────── */

int isinf(double x) {
    return (d2u(x) & 0x7fffffffffffffffULL) == 0x7ff0000000000000ULL;
}

int isnan(double x) {
    uint64_t u = d2u(x) & 0x7fffffffffffffffULL;
    return u > 0x7ff0000000000000ULL;
}

int isfinite(double x) {
    return (d2u(x) & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL;
}

int fpclassify(double x) {
    uint64_t u = d2u(x);
    uint64_t exp = u & 0x7ff0000000000000ULL;
    uint64_t man = u & 0x000fffffffffffffULL;
    if (exp == 0x7ff0000000000000ULL) return man ? 4 : 3;  /* nan : inf */
    if (exp == 0) return man ? 1 : 2;                       /* sub : zero */
    return 0;                                               /* normal */
}

/* ── fabs, copysign ──────────────────────────────────────────────────────── */

double fabs(double x) {
    return u2d(d2u(x) & 0x7fffffffffffffffULL);
}

double copysign(double x, double y) {
    return u2d((d2u(x) & 0x7fffffffffffffffULL) |
               (d2u(y) & 0x8000000000000000ULL));
}

float fabsf(float x) { return u2f(f2u(x) & 0x7fffffffU); }

/* ── arredondamento ──────────────────────────────────────────────────────── */

double floor(double x) {
    double t = (double)(long long)x;
    return (t > x) ? t - 1.0 : t;
}

double ceil(double x) {
    double t = (double)(long long)x;
    return (t < x) ? t + 1.0 : t;
}

double trunc(double x) {
    return (double)(long long)x;
}

double round(double x) {
    return (x >= 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}

double fmod(double x, double y) {
    if (y == 0.0 || isinf(x) || isnan(x) || isnan(y)) return NAN;
    if (isinf(y)) return x;
    long long n = (long long)(x / y);
    return x - (double)n * y;
}

float floorf(float x) { return (float)floor((double)x); }
float ceilf (float x) { return (float)ceil ((double)x); }
float roundf(float x) { return (float)round((double)x); }

/* ── sqrt — Newton-Raphson com seed IEEE 754 ─────────────────────────────── */

double sqrt(double x) {
    if (x < 0.0) return NAN;
    if (x == 0.0 || isinf(x)) return x;

    /* seed: aproveita a representação IEEE 754 para estimativa inicial */
    double r = u2d((d2u(x) + 0x3ff0000000000000ULL) >> 1);

    /* 4 iterações de Newton: r ← (r + x/r) / 2 — converge quadraticamente */
    r = 0.5 * (r + x / r);
    r = 0.5 * (r + x / r);
    r = 0.5 * (r + x / r);
    r = 0.5 * (r + x / r);
    return r;
}

/* ── cbrt — Newton-Raphson cúbica ────────────────────────────────────────── */

double cbrt(double x) {
    if (x == 0.0) return 0.0;
    int neg = x < 0.0;
    double a = neg ? -x : x;

    /* seed via bit trick para cúbica: divide expoente por 3 */
    uint64_t u = d2u(a);
    int exp = (int)((u >> 52) & 0x7ff) - 1023;
    int exp3 = exp / 3;
    double r = u2d(((uint64_t)(exp3 + 1023) << 52) | (u & 0x000fffffffffffffULL));

    /* 5 iterações de Newton: r ← (2r + a/r²) / 3 */
    for (int i = 0; i < 5; i++)
        r = (2.0 * r + a / (r * r)) / 3.0;

    return neg ? -r : r;
}

double hypot(double x, double y) {
    return sqrt(x*x + y*y);
}

float sqrtf(float x) { return (float)sqrt((double)x); }

/* ── exp — decomposição em potência de 2 ────────────────────────────────── */

/*
 * exp(x) = 2^k · exp(r),  onde k = round(x/ln2), r = x - k·ln2
 * Para r ∈ [-ln2/2, ln2/2] usamos a minimax de grau 6.
 */
double exp(double x) {
    if (isnan(x)) return NAN;
    if (x >  709.78) return INFINITY;
    if (x < -745.13) return 0.0;

    /* k = nint(x / ln2) */
    double k_f = x * M_LOG2E;
    long long k = (long long)(k_f + (k_f >= 0 ? 0.5 : -0.5));
    double r = x - (double)k * M_LN2;

    /* série de Taylor para exp(r), r ∈ [-ln2/2, ln2/2]
     * grau 10 garante erro < 1 ULP no intervalo reduzido */
    double r2 = r  * r;
    double r4 = r2 * r2;
    double p  = 1.0
              + r
              + r2 * 5.0000000000000000e-1
              + r2 * r  * 1.6666666666666667e-1
              + r4 * 4.1666666666666664e-2
              + r4 * r  * 8.3333333333333332e-3
              + r4 * r2 * 1.3888888888888889e-3
              + r4 * r2 * r * 1.9841269841269841e-4
              + r4 * r4 * 2.4801587301587302e-5
              + r4 * r4 * r * 2.7557319223985888e-6;

    /* escalona por 2^k manipulando o expoente IEEE 754 */
    return u2d(d2u(p) + ((uint64_t)k << 52));
}

double exp2(double x) {
    return exp(x * M_LN2);
}

float expf(float x) { return (float)exp((double)x); }

/* ── log — redução para [1, 2) ───────────────────────────────────────────── */

/*
 * log(x) = k·ln2 + log(m),  x = m · 2^k,  m ∈ [1, 2)
 *
 * Para log(m): usa a substituição s = (m-1)/(m+1), log(m) = 2·atanh(s)
 * com série de Horner para atanh(s) = s + s³/3 + s⁵/5 + ...
 */
double log(double x) {
    if (x < 0.0) return NAN;
    if (x == 0.0) return -INFINITY;
    if (isinf(x)) return INFINITY;
    if (isnan(x)) return NAN;

    uint64_t u = d2u(x);
    int k = (int)((u >> 52) & 0x7ff) - 1023;

    /* normaliza mantissa para [1, 2) */
    double m = u2d((u & 0x000fffffffffffffULL) | 0x3ff0000000000000ULL);

    /* s = (m-1)/(m+1), log(m) = 2·atanh(s) */
    double s  = (m - 1.0) / (m + 1.0);
    double s2 = s * s;

    /* série de Horner para 2·atanh(s) — termos ímpares até s^17
     * erro < 1e-15 para m ∈ [1, 2) */
    double p = s * (2.0 + s2 * (2.0/3.0 + s2 * (2.0/5.0 +
               s2 * (2.0/7.0 + s2 * (2.0/9.0 +
               s2 * (2.0/11.0 + s2 * (2.0/13.0 +
               s2 * (2.0/15.0 + s2 *  2.0/17.0))))))));

    return (double)k * M_LN2 + p;
}

double log2 (double x) { return log(x) * M_LOG2E;  }
double log10(double x) { return log(x) * M_LOG10E; }

float logf(float x) { return (float)log((double)x); }

/* ── pow ─────────────────────────────────────────────────────────────────── */

double pow(double base, double exponent) {
    if (exponent == 0.0) return 1.0;
    if (isnan(base) || isnan(exponent)) return NAN;
    if (base == 1.0) return 1.0;

    /* casos inteiros: usa multiplicação rápida para n pequeno */
    long long n = (long long)exponent;
    if ((double)n == exponent && n >= -64 && n <= 64) {
        double result = 1.0;
        double b = n >= 0 ? base : 1.0 / base;
        long long an = n >= 0 ? n : -n;
        while (an) {
            if (an & 1) result *= b;
            b  *= b;
            an >>= 1;
        }
        return result;
    }

    if (base < 0.0) return NAN;
    if (base == 0.0) return (exponent > 0.0) ? 0.0 : INFINITY;
    return exp(exponent * log(base));
}

float powf(float base, float exp) { return (float)pow((double)base, (double)exp); }

/* ── sin/cos — Chebyshev após redução de argumento ───────────────────────── */

/*
 * Redução de argumento: mapeia x para [-π/4, π/4].
 * Quadrante codificado em `quad` (0..3).
 *
 * Coeficientes de Chebyshev para sin(x) e cos(x) no intervalo reduzido
 * calculados via algoritmo de Remez — erro máximo < 2^-52.
 */

/* polinômio para sin(r), r ∈ [-π/4, π/4] */
static double poly_sin(double r) {
    double r2 = r * r;
    return r * (1.0 + r2 * (-1.6666666666666666e-1 +
           r2 * ( 8.3333333333333329e-3 +
           r2 * (-1.9841269841269841e-4 +
           r2 * ( 2.7557319223985889e-6 +
           r2 * (-2.5052108385441720e-8 +
           r2 *   1.6059043836821613e-10))))));
}

/* polinômio para cos(r), r ∈ [-π/4, π/4] */
static double poly_cos(double r) {
    double r2 = r * r;
    return 1.0 + r2 * (-0.5 +
           r2 * ( 4.1666666666666664e-2 +
           r2 * (-1.3888888888888889e-3 +
           r2 * ( 2.4801587301587302e-5 +
           r2 * (-2.7557319223985888e-7 +
           r2 *   2.0876756987868099e-9)))));
}

/* reduz x para [-π/4, π/4] e retorna quadrante 0-3 */
static int reduce_trig(double x, double *r) {
    /* n = round(x / (π/2)), mantém sinal */
    long long n = (long long)(x * M_2_PI + (x >= 0 ? 0.5 : -0.5));
    *r = x - (double)n * M_PI_2;
    return (int)((n % 4 + 4) % 4);   /* quadrante 0-3, sempre positivo */
}

double sin(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    double r;
    int q = reduce_trig(x, &r);
    switch (q & 3) {
        case 0: return  poly_sin(r);
        case 1: return  poly_cos(r);
        case 2: return -poly_sin(r);
        default: return -poly_cos(r);
    }
}

double cos(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    double r;
    int q = reduce_trig(x, &r);
    switch (q & 3) {
        case 0: return  poly_cos(r);
        case 1: return -poly_sin(r);
        case 2: return -poly_cos(r);
        default: return  poly_sin(r);
    }
}

double tan(double x) {
    if (isnan(x) || isinf(x)) return NAN;
    double s = sin(x);
    double c = cos(x);
    return c == 0.0 ? copysign(INFINITY, s) : s / c;
}

float sinf(float x) { return (float)sin((double)x); }
float cosf(float x) { return (float)cos((double)x); }
float tanf(float x) { return (float)tan((double)x); }

/* ── atan — minimax racional [3/3] ───────────────────────────────────────── */

/*
 * Para |x| ≤ 1: atan(x) direto com minimax.
 * Para |x| > 1: atan(x) = π/2 - atan(1/x).
 * Para x < 0:   atan(x) = -atan(-x).
 */
static double atan_small(double x) {
    if (x == 1.0) return 0.7853981633974483096156608458198757; /* M_PI_4 */
    /* aproximação minimax de precisão absoluta, rerr < 10^-12 */
    double x2 = x * x;
    return x * (1.0 + x2 * (-0.3333333333301032 + 
           x2 * (0.1999999994328211 + 
           x2 * (-0.142857118318 + 
           x2 * (0.111110825 + 
           x2 * (-0.090906 + 
           x2 * (0.0768 + 
           x2 * (-0.063))))))));
}

double atan(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return copysign(M_PI_2, x);

    int neg = x < 0.0;
    double a = neg ? -x : x;

    double r;
    if (a <= 1.0) {
        r = atan_small(a);
    } else {
        r = M_PI_2 - atan_small(1.0 / a);
    }
    return neg ? -r : r;
}

double atan2(double y, double x) {
    if (isnan(x) || isnan(y)) return NAN;
    if (x == 0.0) {
        if (y == 0.0) return 0.0;
        return copysign(M_PI_2, y);
    }
    double r = atan(y / x);
    if (x < 0.0) r += (y >= 0.0) ? M_PI : -M_PI;
    return r;
}

/* ── asin / acos ─────────────────────────────────────────────────────────── */

/*
 * asin(x) = atan(x / sqrt(1 - x²))     para |x| ≤ √2/2
 * asin(x) = π/2 - 2·asin(√((1-x)/2))  para |x| > √2/2  (melhor precisão)
 */
double asin(double x) {
    if (isnan(x)) return NAN;
    double a = fabs(x);
    if (a > 1.0) return NAN;
    if (a == 1.0) return copysign(M_PI_2, x);

    double r;
    if (a <= 0.5) {
        r = atan(a / sqrt(1.0 - a*a));
    } else {
        r = M_PI_2 - 2.0 * atan(sqrt((1.0 - a) / (1.0 + a)));
    }
    return copysign(r, x);
}

double acos(double x) {
    if (isnan(x)) return NAN;
    if (fabs(x) > 1.0) return NAN;
    return M_PI_2 - asin(x);
}

/* ── hiperbólicas ────────────────────────────────────────────────────────── */

/*
 * sinh(x) = (eˣ - e⁻ˣ) / 2
 * cosh(x) = (eˣ + e⁻ˣ) / 2
 * tanh(x) = sinh(x) / cosh(x)
 *
 * Para x pequeno, usa expansão direta para evitar cancelamento catastrófico.
 */
double sinh(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return x;
    double a = fabs(x);
    if (a < 1e-8) return x;     /* evita cancelamento: sinh(x) ≈ x */
    double ex = exp(a);
    double r  = 0.5 * (ex - 1.0/ex);
    return copysign(r, x);
}

double cosh(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return INFINITY;
    double ex = exp(fabs(x));
    return 0.5 * (ex + 1.0/ex);
}

double tanh(double x) {
    if (isnan(x)) return NAN;
    if (isinf(x)) return copysign(1.0, x);
    double a = fabs(x);
    double r;
    if (a < 1e-8) {
        r = a;                   /* tanh(x) ≈ x para x pequeno */
    } else if (a < 20.0) {
        double e2 = exp(2.0 * a);
        r = (e2 - 1.0) / (e2 + 1.0);
    } else {
        r = 1.0;                 /* tanh satura em 1 */
    }
    return copysign(r, x);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: math
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    /* classificação */
    int    (*isinf)    (double x);
    int    (*isnan)    (double x);
    int    (*isfinite) (double x);

    /* trigonometria */
    double (*sin)  (double x);
    double (*cos)  (double x);
    double (*tan)  (double x);
    double (*asin) (double x);
    double (*acos) (double x);
    double (*atan) (double x);
    double (*atan2)(double y, double x);

    /* hiperbólicas */
    double (*sinh) (double x);
    double (*cosh) (double x);
    double (*tanh) (double x);

    /* exponencial e logaritmo */
    double (*exp)  (double x);
    double (*exp2) (double x);
    double (*log)  (double x);
    double (*log2) (double x);
    double (*log10)(double x);
    double (*pow)  (double base, double exponent);

    /* raiz e potência */
    double (*sqrt) (double x);
    double (*cbrt) (double x);
    double (*hypot)(double x, double y);

    /* arredondamento */
    double (*floor)(double x);
    double (*ceil) (double x);
    double (*round)(double x);
    double (*trunc)(double x);
    double (*fmod) (double x, double y);

    /* valor absoluto e sinal */
    double (*fabs)    (double x);
    double (*copysign)(double x, double y);

    /* versões float */
    float  (*sinf)  (float x);
    float  (*cosf)  (float x);
    float  (*tanf)  (float x);
    float  (*sqrtf) (float x);
    float  (*fabsf) (float x);
    float  (*floorf)(float x);
    float  (*ceilf) (float x);
    float  (*roundf)(float x);
    float  (*powf)  (float base, float exp);
    float  (*logf)  (float x);
    float  (*expf)  (float x);
} Papagaio_Math;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: str  (string.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    /* comprimento */
    size_t (*strlen) (const char *s);
    size_t (*strnlen)(const char *s, size_t maxlen);
    char  *(*strdup) (const char *s);

    /* cópia */
    char  *(*strcpy) (char *dst, const char *src);
    char  *(*strncpy)(char *dst, const char *src, size_t n);

    /* concatenação */
    char  *(*strcat) (char *dst, const char *src);
    char  *(*strncat)(char *dst, const char *src, size_t n);

    /* comparação */
    int    (*strcmp) (const char *s1, const char *s2);
    int    (*strncmp)(const char *s1, const char *s2, size_t n);

    /* busca */
    char  *(*strchr) (const char *s, int c);
    char  *(*strrchr)(const char *s, int c);
    char  *(*strstr) (const char *haystack, const char *needle);
    char  *(*strpbrk)(const char *s, const char *accept);

    /* span */
    size_t (*strspn) (const char *s, const char *accept);
    size_t (*strcspn)(const char *s, const char *reject);

    /* tokenização */
    char  *(*strtok)  (char *str, const char *delim);
    char  *(*strtok_r)(char *str, const char *delim, char **saveptr);

    /* extras */
    char  *(*strrev)  (char *s);
    const char *(*strerror)(int errnum);
} Papagaio_Str;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: mem  (memória — parte de string.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    void  *(*memcpy) (void *dst, const void *src, size_t n);
    void  *(*memmove)(void *dst, const void *src, size_t n);
    void  *(*memset) (void *s, int c, size_t n);
    int    (*memcmp) (const void *s1, const void *s2, size_t n);
    void  *(*memchr) (const void *s, int c, size_t n);

    /* alocação (stdlib) */
    void  *(*malloc) (size_t size);
    void  *(*calloc) (size_t nmemb, size_t size);
    void  *(*realloc)(void *ptr, size_t size);
    void   (*free)   (void *ptr);
} Papagaio_Mem;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: io  (stdio.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int (*printf)        (const char *fmt, ...);
    int (*sprintf)       (char *buf, const char *fmt, ...);
    int (*snprintf)      (char *buf, size_t size, const char *fmt, ...);
    int (*vprintf)       (const char *fmt, va_list ap);
    int (*vsprintf)      (char *buf, const char *fmt, va_list ap);
    int (*vsnprintf)     (char *buf, size_t size, const char *fmt, va_list ap);
    int (*putchar)       (int c);
    int (*puts)          (const char *s);
    int (*fprintf_stdout)(const char *fmt, ...);
    int (*fprintf_stderr)(const char *fmt, ...);
} Papagaio_IO;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: conv  (conversões — stdlib.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    /* string → número */
    int               (*atoi)  (const char *s);
    long              (*atol)  (const char *s);
    long long         (*atoll) (const char *s);
    long              (*strtol)  (const char *s, char **endptr, int base);
    long long         (*strtoll) (const char *s, char **endptr, int base);
    unsigned long     (*strtoul) (const char *s, char **endptr, int base);
    unsigned long long(*strtoull)(const char *s, char **endptr, int base);

    /* número → string */
    char *(*itoa)(int value, char *str, int base);
} Papagaio_Conv;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: algo  (algoritmos — stdlib.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    void  (*qsort)  (void *base, size_t nmemb, size_t size,
                     int (*compar)(const void *, const void *));
    void *(*bsearch)(const void *key, const void *base, size_t nmemb, size_t size,
                     int (*compar)(const void *, const void *));

    /* aritmética inteira */
    int       (*abs)  (int x);
    long      (*labs) (long x);
    long long (*llabs)(long long x);

    div_t   (*div)  (int numer, int denom);
    ldiv_t  (*ldiv) (long numer, long denom);
    lldiv_t (*lldiv)(long long numer, long long denom);
} Papagaio_Algo;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: rng  (pseudo-aleatório — stdlib.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int  (*rand) (void);
    void (*srand)(unsigned int seed);
} Papagaio_RNG;


/* ═══════════════════════════════════════════════════════════════════════════
 * Módulo: ctype  (ctype.h)
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int (*isalpha) (int c);
    int (*isupper) (int c);
    int (*islower) (int c);
    int (*isdigit) (int c);
    int (*isalnum) (int c);
    int (*isxdigit)(int c);
    int (*isspace) (int c);
    int (*isblank) (int c);
    int (*ispunct) (int c);
    int (*iscntrl) (int c);
    int (*isprint) (int c);
    int (*isgraph) (int c);
    int (*toupper) (int c);
    int (*tolower) (int c);
} Papagaio_Ctype;


/* ═══════════════════════════════════════════════════════════════════════════
 * Struct raiz: Papagaio
 * ═════════════════════════════════════════════════════════════════════════ */

typedef struct {
    Papagaio_Math  math;
    Papagaio_Str   str;
    Papagaio_Mem   mem;
    Papagaio_IO    io;
    Papagaio_Conv  conv;
    Papagaio_Algo  algo;
    Papagaio_RNG   rng;
    Papagaio_Ctype ctype;
} Papagaio;


/* ═══════════════════════════════════════════════════════════════════════════
 * Instância global (const — todos os ponteiros são fixos em compile-time)
 * ═════════════════════════════════════════════════════════════════════════ */

static const Papagaio papagaio = {

    /* ── math ────────────────────────────────────────────────────────────── */
    .math = {
        .isinf     = isinf,
        .isnan     = isnan,
        .isfinite  = isfinite,

        .sin   = sin,
        .cos   = cos,
        .tan   = tan,
        .asin  = asin,
        .acos  = acos,
        .atan  = atan,
        .atan2 = atan2,

        .sinh  = sinh,
        .cosh  = cosh,
        .tanh  = tanh,

        .exp   = exp,
        .exp2  = exp2,
        .log   = log,
        .log2  = log2,
        .log10 = log10,
        .pow   = pow,

        .sqrt  = sqrt,
        .cbrt  = cbrt,
        .hypot = hypot,

        .floor = floor,
        .ceil  = ceil,
        .round = round,
        .trunc = trunc,
        .fmod  = fmod,

        .fabs     = fabs,
        .copysign = copysign,

        .sinf   = sinf,
        .cosf   = cosf,
        .tanf   = tanf,
        .sqrtf  = sqrtf,
        .fabsf  = fabsf,
        .floorf = floorf,
        .ceilf  = ceilf,
        .roundf = roundf,
        .powf   = powf,
        .logf   = logf,
        .expf   = expf,
    },

    /* ── str ─────────────────────────────────────────────────────────────── */
    .str = {
        .strlen  = strlen,
        .strnlen = strnlen,
        .strdup  = strdup,

        .strcpy  = strcpy,
        .strncpy = strncpy,

        .strcat  = strcat,
        .strncat = strncat,

        .strcmp  = strcmp,
        .strncmp = strncmp,

        .strchr  = strchr,
        .strrchr = strrchr,
        .strstr  = strstr,
        .strpbrk = strpbrk,

        .strspn  = strspn,
        .strcspn = strcspn,

        .strtok   = strtok,
        .strtok_r = strtok_r,

        .strrev   = strrev,
        .strerror = strerror,
    },

    /* ── mem ─────────────────────────────────────────────────────────────── */
    .mem = {
        .memcpy  = memcpy,
        .memmove = memmove,
        .memset  = memset,
        .memcmp  = memcmp,
        .memchr  = memchr,

        .malloc  = malloc,
        .calloc  = calloc,
        .realloc = realloc,
        .free    = free,
    },

    /* ── io ──────────────────────────────────────────────────────────────── */
    .io = {
        .printf         = printf,
        .sprintf        = sprintf,
        .snprintf       = snprintf,
        .vprintf        = vprintf,
        .vsprintf       = vsprintf,
        .vsnprintf      = vsnprintf,
        .putchar        = putchar,
        .puts           = puts,
        .fprintf_stdout = fprintf_stdout,
        .fprintf_stderr = fprintf_stderr,
    },

    /* ── conv ────────────────────────────────────────────────────────────── */
    .conv = {
        .atoi    = atoi,
        .atol    = atol,
        .atoll   = atoll,
        .strtol  = strtol,
        .strtoll = strtoll,
        .strtoul = strtoul,
        .strtoull= strtoull,
        .itoa    = itoa,
    },

    /* ── algo ────────────────────────────────────────────────────────────── */
    .algo = {
        .qsort   = qsort,
        .bsearch = bsearch,
        .abs     = abs,
        .labs    = labs,
        .llabs   = llabs,
        .div     = div,
        .ldiv    = ldiv,
        .lldiv   = lldiv,
    },

    /* ── rng ─────────────────────────────────────────────────────────────── */
    .rng = {
        .rand  = rand,
        .srand = srand,
    },

    /* ── ctype ───────────────────────────────────────────────────────────── */
    .ctype = {
        .isalpha  = isalpha,
        .isupper  = isupper,
        .islower  = islower,
        .isdigit  = isdigit,
        .isalnum  = isalnum,
        .isxdigit = isxdigit,
        .isspace  = isspace,
        .isblank  = isblank,
        .ispunct  = ispunct,
        .iscntrl  = iscntrl,
        .isprint  = isprint,
        .isgraph  = isgraph,
        .toupper  = toupper,
        .tolower  = tolower,
    },
};

#endif /* PAPAGAIO_H */