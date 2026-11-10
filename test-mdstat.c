/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * test-mdstat.c - mdstat_read() must accept the array lines of the
 * active subsystem: "msN" / "ms_<name>" under ms, "mdN" / "md_dN" /
 * "md_<name>" under md.
 *
 * mdstat_read() opens current_subsys->proc_stat, so each case points a
 * private copy of the descriptor at a fixture file that mimics the
 * kernel's /proc/mdstat layout (continuation lines are indented).
 */
#include "mdadm.h"
#include "mdstat.h"
#include "subsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* pr_err() and friends print this; mdadm.c normally defines it. */
const char Name[] = "test-mdstat";

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

static void check_str(const char *label, const char *got, const char *want)
{
	if (got && strcmp(got, want) == 0) {
		printf("  ok: %s\n", label);
	} else {
		printf("FAIL: %s (got \"%s\", want \"%s\")\n", label,
		       got ? got : "(null)", want);
		failures++;
	}
}

static int count(struct mdstat_ent *e)
{
	int n = 0;

	for (; e; e = e->next)
		n++;
	return n;
}

static struct mdstat_ent *read_fixture(const struct subsys *base,
				       const char *content, int start)
{
	char path[] = "/tmp/test-mdstat.XXXXXX";
	const struct subsys *saved = current_subsys;
	struct subsys s = *base;
	struct mdstat_ent *ents;
	ssize_t len = strlen(content);
	int fd = mkstemp(path);

	if (fd < 0 || write(fd, content, len) != len) {
		perror("test-mdstat: cannot write fixture");
		exit(1);
	}
	close(fd);

	s.proc_stat = path;
	current_subsys = &s;
	ents = mdstat_read(0, start);
	current_subsys = saved;

	unlink(path);
	return ents;
}

/* /proc/msstat with a numbered and a named array. */
static void test_ms_arrays(void)
{
	static const char fixture[] =
		"Personalities : [raid1] [raid10] \n"
		"ms0 : active raid1 loop1[1] loop0[0]\n"
		"      102336 blocks super 1.2 [2/2] [UU]\n"
		"      \n"
		"ms_foo : inactive loop3[1](S) loop2[0](S)\n"
		"      204672 blocks super 1.2\n"
		"       \n"
		"unused devices: <none>\n";
	struct mdstat_ent *ents = read_fixture(&subsys_ms, fixture, 0);

	check_int("ms: two arrays parsed", count(ents), 2);
	if (count(ents) == 2) {
		check_str("ms: ms0 devnm", ents->devnm, "ms0");
		check_int("ms: ms0 active", ents->active, 1);
		check_str("ms: ms0 level", ents->level, "raid1");
		check_int("ms: ms0 devcnt", ents->devcnt, 2);
		check_int("ms: ms0 raid_disks", ents->raid_disks, 2);
		check_str("ms: ms0 metadata", ents->metadata_version, "1.2");
		check_str("ms: ms_foo devnm", ents->next->devnm, "ms_foo");
		check_int("ms: ms_foo inactive", ents->next->active, 0);
	}
	free_mdstat(ents);
}

/* A stacked ms array: with start=1 the component must come before the
 * composite, which needs the member-name prefix check to see "ms0[0]". */
static void test_ms_nested_order(void)
{
	static const char fixture[] =
		"Personalities : [raid0] [raid1] \n"
		"ms0 : active raid1 loop1[1] loop0[0]\n"
		"      102336 blocks super 1.2 [2/2] [UU]\n"
		"      \n"
		"ms1 : active raid0 ms0[0] loop2[1]\n"
		"      204672 blocks super 1.2 512k chunks\n"
		"      \n"
		"unused devices: <none>\n";
	struct mdstat_ent *ents = read_fixture(&subsys_ms, fixture, 1);

	check_int("ms nested: two arrays parsed", count(ents), 2);
	if (count(ents) == 2) {
		check_str("ms nested: component first", ents->devnm, "ms0");
		check_str("ms nested: composite second", ents->next->devnm, "ms1");
	}
	free_mdstat(ents);
}

/* Stock md must keep working: numbered and partitionable names. */
static void test_md_arrays(void)
{
	static const char fixture[] =
		"Personalities : [raid1] \n"
		"md0 : active raid1 loop1[1] loop0[0]\n"
		"      102336 blocks super 1.2 [2/2] [UU]\n"
		"      \n"
		"md_d0 : active raid1 loop3[1] loop2[0]\n"
		"      102336 blocks super 1.2 [2/2] [UU]\n"
		"      \n"
		"unused devices: <none>\n";
	struct mdstat_ent *ents = read_fixture(&subsys_md, fixture, 0);

	check_int("md: two arrays parsed", count(ents), 2);
	if (count(ents) == 2) {
		check_str("md: md0 devnm", ents->devnm, "md0");
		check_str("md: md_d0 devnm", ents->next->devnm, "md_d0");
		check_int("md: md0 devcnt", ents->devcnt, 2);
	}
	free_mdstat(ents);
}

int main(void)
{
	test_ms_arrays();
	test_ms_nested_order();
	test_md_arrays();

	if (failures) {
		printf("%d test(s) failed\n", failures);
		return 1;
	}
	printf("all mdstat tests passed\n");
	return 0;
}
