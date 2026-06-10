#include "../../src/papagaio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Modifier: $var$len{MIN{,MAX}}
 * Validates that the captured text length is within [MIN, MAX].
 * Pass MIN and MAX as part of the modifier name separated by underscores.
 *
 * Examples:
 *   $pattern{$p$len_3_8}{[$p]}   — length between 3 and 8
 *   $pattern{$p$len_5}{[$p]}     — exact length 5
 *
 * Note: modifier names allow alphanumeric + underscore, so
 * "len_3_8" is a valid modifier name.
 */
static char *len_modifier(const char *match, const char *modifier,
                          size_t match_len, size_t mod_len, void *userdata)
{
    (void)userdata;
    if (!match) return NULL;

    /* Parse modifier name: "len" or "len_MIN" or "len_MIN_MAX" */
    int min = 0, max = -1;
    int parsed = 0;

    /* Skip "len" prefix */
    const char *p = modifier;
    size_t remaining = mod_len;
    if (remaining >= 3 && memcmp(p, "len", 3) == 0) {
        p += 3;
        remaining -= 3;
    }

    if (remaining > 0 && *p == '_') {
        p++; remaining--;
        /* Parse min */
        min = 0;
        while (remaining > 0 && *p >= '0' && *p <= '9') {
            min = min * 10 + (*p - '0');
            p++; remaining--; parsed = 1;
        }
        if (remaining > 0 && *p == '_') {
            p++; remaining--;
            max = 0;
            while (remaining > 0 && *p >= '0' && *p <= '9') {
                max = max * 10 + (*p - '0');
                p++; remaining--; parsed = 1;
            }
        } else {
            max = min;
        }
    }

    if (!parsed) {
        min = 0;
        max = -1;
    }

    size_t len = match_len;
    if (len < (size_t)min) return NULL;
    if (max >= 0 && (int)len > max) return NULL;
    return strndup(match, match_len);
}

int papagaio_plugin_init(Papagaio *ctx)
{
    return papagaio_register_modifier(ctx, "len", len_modifier, NULL);
}
