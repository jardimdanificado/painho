#include "../../src/papagaio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Modifier: $var$alpha
 * Matches only if the captured text contains exclusively alphabetic characters.
 * Transforms to uppercase.
 */
static char *alpha_modifier(const char *match, const char *modifier,
                            size_t match_len, size_t mod_len, void *userdata)
{
    (void)modifier;
    (void)mod_len;
    (void)userdata;
    if (!match || match_len == 0) return NULL;
    for (size_t i = 0; i < match_len; i++)
        if (!isalpha((unsigned char)match[i])) return NULL;
    char *res = (char *)malloc(match_len + 1);
    for (size_t i = 0; i < match_len; i++) res[i] = toupper((unsigned char)match[i]);
    res[match_len] = '\0';
    return res;
}

int papagaio_plugin_init(Papagaio *ctx)
{
    return papagaio_register_modifier(ctx, "alpha", alpha_modifier, NULL);
}
