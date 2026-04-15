/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "bitmap_parse.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(const char *label, int got, int want)
{
    if (got == want) {
        printf("  ok: %s\n", label);
    } else {
        printf("FAIL: %s (got %d, want %d)\n", label, got, want);
        failures++;
    }
}

int main(void)
{
    /* Real kernel output: llbitmap loaded, none currently selected. */
    check("[none] internal llbitmap -> llbitmap",
          bitmap_type_list_contains("[none] internal llbitmap \n", "llbitmap"), 1);
    check("[none] internal llbitmap -> internal",
          bitmap_type_list_contains("[none] internal llbitmap \n", "internal"), 1);
    check("[none] internal llbitmap -> cluster (absent)",
          bitmap_type_list_contains("[none] internal llbitmap \n", "cluster"), 0);

    /* internal currently selected, llbitmap still listed. */
    check("none [internal] llbitmap -> llbitmap",
          bitmap_type_list_contains("none [internal] llbitmap \n", "llbitmap"), 1);
    check("none [internal] llbitmap -> internal (bracketed)",
          bitmap_type_list_contains("none [internal] llbitmap \n", "internal"), 1);

    /* Old kernel: llbitmap submodule not registered. */
    check("[none] internal -> llbitmap (absent)",
          bitmap_type_list_contains("[none] internal \n", "llbitmap"), 0);
    check("[none] internal -> internal",
          bitmap_type_list_contains("[none] internal \n", "internal"), 1);

    /* Degenerate inputs. */
    check("empty -> llbitmap",
          bitmap_type_list_contains("", "llbitmap"), 0);
    check("NULL -> llbitmap",
          bitmap_type_list_contains(NULL, "llbitmap"), 0);
    check("[none] only -> none (bracketed)",
          bitmap_type_list_contains("[none] \n", "none"), 1);
    check("[none] only -> llbitmap (absent)",
          bitmap_type_list_contains("[none] \n", "llbitmap"), 0);

    /* No trailing newline, no trailing space. */
    check("no newline: [none] internal llbitmap -> llbitmap",
          bitmap_type_list_contains("[none] internal llbitmap", "llbitmap"), 1);

    /* Partial substring must not false-positive. */
    check("[none] llbitmap -> llbit (substring, absent)",
          bitmap_type_list_contains("[none] llbitmap \n", "llbit"), 0);

    if (failures) {
        printf("%d test(s) failed\n", failures);
        return 1;
    }
    printf("all tests passed\n");
    return 0;
}
