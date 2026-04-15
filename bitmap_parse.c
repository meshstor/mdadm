/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "bitmap_parse.h"

#include <string.h>

/*
 * Walks @buf once. For each whitespace-delimited token:
 *   - optionally skip a leading '['
 *   - compare up to the first ']' or end-of-token against @type
 * Returns 1 on first match, 0 at end.
 */
int bitmap_type_list_contains(const char *buf, const char *type)
{
    const char *p;
    size_t tlen;

    if (!buf || !type || !*type)
        return 0;

    tlen = strlen(type);
    p = buf;

    while (*p) {
        const char *tok;
        size_t toklen;

        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;
        if (!*p)
            break;

        if (*p == '[')
            p++;

        tok = p;
        while (*p && *p != ']' && *p != ' ' && *p != '\t'
               && *p != '\n' && *p != '\r')
            p++;
        toklen = (size_t)(p - tok);

        if (toklen == tlen && memcmp(tok, type, tlen) == 0)
            return 1;

        if (*p == ']')
            p++;
    }
    return 0;
}
