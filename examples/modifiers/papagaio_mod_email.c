#include "../../src/papagaio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Modifier: $var$email
 * Validates a basic email format: local@domain.tld
 * Captures the entire matched address.
 */
static char *email_modifier(const char *match, const char *modifier,
                            size_t match_len, size_t mod_len, void *userdata)
{
    (void)modifier;
    (void)mod_len;
    (void)userdata;
    if (!match || match_len == 0) return NULL;

    const char *p = match;
    size_t len = match_len;

    /* Find @ */
    const char *at = (const char *)memchr(p, '@', len);
    if (!at) return NULL;
    size_t local_len = (size_t)(at - p);
    if (local_len == 0) return NULL;

    /* Validate local part */
    for (size_t i = 0; i < local_len; i++)
        if (!isalnum((unsigned char)p[i]) && p[i] != '.' && p[i] != '_' && p[i] != '-')
            return NULL;

    const char *domain = at + 1;
    size_t domain_len = len - local_len - 1;
    if (domain_len < 3) return NULL;

    /* Find last dot in domain */
    const char *dot = NULL;
    for (size_t i = 0; i < domain_len; i++)
        if (domain[i] == '.') dot = domain + i;
    if (!dot || (size_t)(dot - domain) == 0) return NULL;

    /* Validate domain chars */
    for (size_t i = 0; i < domain_len; i++)
        if (!isalnum((unsigned char)domain[i]) && domain[i] != '.' && domain[i] != '-')
            return NULL;

    /* TLD must be at least 2 chars */
    size_t tld_len = domain_len - (size_t)(dot - domain) - 1;
    if (tld_len < 2) return NULL;

    return strndup(match, match_len);
}

int papagaio_plugin_init(Papagaio *ctx)
{
    return papagaio_register_modifier(ctx, "email", email_modifier, NULL);
}
