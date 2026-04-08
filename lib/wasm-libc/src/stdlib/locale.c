#include <locale.h>

/* lconv estático com convenções do locale "C" */
static struct lconv _c_locale = {
    .decimal_point     = ".",
    .thousands_sep     = "",
    .grouping          = "",
    .int_curr_symbol   = "",
    .currency_symbol   = "",
    .mon_decimal_point = "",
    .mon_thousands_sep = "",
    .mon_grouping      = "",
    .positive_sign     = "",
    .negative_sign     = "",
    .int_frac_digits   = 127,
    .frac_digits       = 127,
    .p_cs_precedes     = 127,
    .p_sep_by_space    = 127,
    .n_cs_precedes     = 127,
    .n_sep_by_space    = 127,
    .p_sign_posn       = 127,
    .n_sign_posn       = 127,
};

char *setlocale(int category, const char *locale) {
    (void)category;
    /* aceita NULL (query) e "C" / "POSIX"; rejeita todo o resto */
    if (locale == (char *)0 || locale[0] == '\0') return "C";
    if (locale[0] == 'C' && locale[1] == '\0')    return "C";
    if (locale[0] == 'P' && locale[1] == 'O' &&
        locale[2] == 'S' && locale[3] == 'I' &&
        locale[4] == 'X' && locale[5] == '\0')    return "C";
    return (char *)0;   /* locale não suportado */
}

struct lconv *localeconv(void) {
    return &_c_locale;
}
