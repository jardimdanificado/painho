#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <stdint.h>

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

/* ── controle de fluxo ───────────────────────────────────────────────────── */
void abort(void);  /* importado do host via __host_abort */

/* ── conversão numérica ──────────────────────────────────────────────────── */
/* itoa: extensão — converte inteiro para string na base dada */
char *itoa(int value, char *str, int base);

#endif /* _STDLIB_H */
