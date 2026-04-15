/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef BITMAP_PARSE_H
#define BITMAP_PARSE_H

/*
 * bitmap_type_list_contains - does @buf list @type as a supported
 * bitmap submodule?
 *
 * @buf is the raw content of /sys/block/mdX/md/bitmap_type as produced
 * by the kernel's bitmap_type_show(), e.g.:
 *
 *     "[none] internal llbitmap \n"
 *
 * Tokens are whitespace-separated. The currently-selected token is
 * wrapped in [brackets]. Returns 1 iff any token (with brackets
 * stripped) equals @type. Safe to call with a buf that is NULL, empty,
 * or lacks a trailing newline.
 */
int bitmap_type_list_contains(const char *buf, const char *type);

#endif /* BITMAP_PARSE_H */
