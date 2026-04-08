#ifndef _LOCALE_H
#define _LOCALE_H

/*
 * locale.h — stubs para wasm bare-metal.
 *
 * Locale real requer tabelas Unicode enormes e complexidade que não faz
 * sentido num ambiente bare-metal. Implementamos apenas o suficiente
 * para que código que inclui locale.h compile e rode em "C" locale.
 *
 * Toda a libc opera em "C" locale: ASCII, ponto decimal = '.', etc.
 */

#define LC_ALL      0
#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_MONETARY 3
#define LC_NUMERIC  4
#define LC_TIME     5
#define LC_MESSAGES 6

struct lconv {
    char *decimal_point;      /* "."  */
    char *thousands_sep;      /* ""   */
    char *grouping;           /* ""   */
    char *int_curr_symbol;    /* ""   */
    char *currency_symbol;    /* ""   */
    char *mon_decimal_point;  /* ""   */
    char *mon_thousands_sep;  /* ""   */
    char *mon_grouping;       /* ""   */
    char *positive_sign;      /* ""   */
    char *negative_sign;      /* ""   */
    char  int_frac_digits;    /* 127  */
    char  frac_digits;        /* 127  */
    char  p_cs_precedes;      /* 127  */
    char  p_sep_by_space;     /* 127  */
    char  n_cs_precedes;      /* 127  */
    char  n_sep_by_space;     /* 127  */
    char  p_sign_posn;        /* 127  */
    char  n_sign_posn;        /* 127  */
};

/* sempre retorna "C" — único locale suportado */
char *setlocale(int category, const char *locale);

/* retorna estrutura com convenções do locale "C" */
struct lconv *localeconv(void);

#endif /* _LOCALE_H */
