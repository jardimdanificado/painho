#include "../../src/papagaio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Modifier: $var$alphanum
 * Matches only alphanumeric characters (letters and digits).
 */
static char *alphanum_modifier(const char *match, const char *modifier,
                               size_t match_len, size_t mod_len, void *userdata)
{
    (void)modifier;
    (void)mod_len;
    (void)userdata;
    if (!match || match_len == 0) return NULL;
    for (size_t i = 0; i < match_len; i++)
        if (!isalnum((unsigned char)match[i])) return NULL;
    return strndup(match, match_len);
}

int papagaio_plugin_init(Papagaio *ctx)
{
    return papagaio_register_modifier(ctx, "alphanum", alphanum_modifier, NULL);
}
