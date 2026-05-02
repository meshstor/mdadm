/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "subsys.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check_int(const char *label, int got, int want)
{
	if (got == want) {
		printf("  ok: %s\n", label);
	} else {
		printf("FAIL: %s (got %d, want %d)\n", label, got, want);
		failures++;
	}
}

static void test_proc_devices(void)
{
	int md = -1, mdp = -1, ms = -1;

	/* md and mdp present, ms absent (typical kernel without meshstor). */
	const char *case1 =
		"Character devices:\n"
		"  1 mem\n"
		"\n"
		"Block devices:\n"
		"  9 md\n"
		"254 mdp\n"
		"259 blkext\n";
	md = mdp = ms = -1;
	subsys_parse_proc_devices(case1, &md, &mdp, &ms);
	check_int("case1: md=9",   md,  9);
	check_int("case1: mdp=254", mdp, 254);
	check_int("case1: ms=-1 (absent)", ms, -1);

	/* All three present. */
	const char *case2 =
		"Block devices:\n"
		"  9 md\n"
		"254 mdp\n"
		"252 ms\n";
	md = mdp = ms = -1;
	subsys_parse_proc_devices(case2, &md, &mdp, &ms);
	check_int("case2: md=9",   md,  9);
	check_int("case2: mdp=254", mdp, 254);
	check_int("case2: ms=252",  ms,  252);

	/* ms only (hypothetical). */
	const char *case3 = "Block devices:\n252 ms\n";
	md = mdp = ms = -1;
	subsys_parse_proc_devices(case3, &md, &mdp, &ms);
	check_int("case3: md=-1",  md,  -1);
	check_int("case3: mdp=-1", mdp, -1);
	check_int("case3: ms=252", ms,  252);

	/* Empty / malformed - all stay -1. */
	md = mdp = ms = -1;
	subsys_parse_proc_devices("", &md, &mdp, &ms);
	check_int("empty: md=-1",  md,  -1);
	check_int("empty: mdp=-1", mdp, -1);
	check_int("empty: ms=-1",  ms,  -1);

	/* Substring guard - "mds" should NOT match "ms". */
	const char *case5 = "Block devices:\n200 mds\n201 msx\n";
	md = mdp = ms = -1;
	subsys_parse_proc_devices(case5, &md, &mdp, &ms);
	check_int("substring guard: ms=-1", ms, -1);
}

static void test_path_to_subsys(void)
{
	struct {
		const char *path;
		const char *want;  /* "md", "ms", or NULL for no match */
	} cases[] = {
		/* md cases */
		{"/dev/md0",          "md"},
		{"/dev/md127",        "md"},
		{"/dev/md_d0",        "md"},
		{"/dev/md/home",      "md"},
		{"/dev/md/0",         "md"},
		{"md0",               "md"},
		{"md_d0",             "md"},
		/* ms cases */
		{"/dev/ms0",          "ms"},
		{"/dev/ms42",         "ms"},
		{"/dev/ms/home",      "ms"},
		{"ms0",               "ms"},
		/* near-misses */
		{"/dev/sda",          NULL},
		{"/dev/mapper/foo",   NULL},
		{"/dev/msftp",        NULL},
		{"/dev/mdadm-extra",  NULL},
		{"/dev/md",           NULL},
		{"/dev/ms",           NULL},
		{"/dev/mdfoo",        NULL},
		{"",                  NULL},
		{"/dev/",             NULL},
	};
	for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
		const struct subsys *got = path_to_subsys(cases[i].path);
		const char *got_name = got ? got->name : NULL;
		bool match;
		if (got_name == NULL && cases[i].want == NULL)
			match = true;
		else if (got_name == NULL || cases[i].want == NULL)
			match = false;
		else
			match = strcmp(got_name, cases[i].want) == 0;
		if (match) {
			printf("  ok: path_to_subsys(%s) -> %s\n",
			       cases[i].path, got_name ? got_name : "(null)");
		} else {
			printf("FAIL: path_to_subsys(%s) got %s want %s\n",
			       cases[i].path,
			       got_name ? got_name : "(null)",
			       cases[i].want ? cases[i].want : "(null)");
			failures++;
		}
	}
}

int main(void)
{
	test_proc_devices();
	test_path_to_subsys();

	if (failures) {
		printf("%d test(s) failed\n", failures);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
