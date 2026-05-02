/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "subsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

struct subsys subsys_md = {
	.name              = "md",
	.dev_prefix        = "/dev/md",
	.dev_dir           = "/dev/md/",
	.proc_stat         = "/proc/mdstat",
	.sysfs_subdir      = "md",
	.devnm_prefix      = "md",
	.devnm_part_prefix = "md_d",
	.major             = 9,    /* MD_MAJOR; verified against /proc/devices at startup */
	.mdp_major         = -1,   /* filled at startup */
};

struct subsys subsys_ms = {
	.name              = "ms",
	.dev_prefix        = "/dev/ms",
	.dev_dir           = "/dev/ms/",
	.proc_stat         = "/proc/msstat",
	.sysfs_subdir      = "ms",
	.devnm_prefix      = "ms",
	.devnm_part_prefix = NULL,
	.major             = -1,   /* filled at startup */
	.mdp_major         = -1,   /* ms has no mdp analog */
};

const struct subsys *current_subsys = &subsys_md;

bool subsys_available(const struct subsys *s)
{
	struct stat st;
	return stat(s->proc_stat, &st) == 0;
}

/* Function bodies for everything else are added in subsequent tasks. */

int subsys_parse_proc_devices(const char *content, int *md_major,
			      int *mdp_major, int *ms_major)
{
	if (!content)
		return -1;

	bool in_block = false;
	const char *p = content;
	while (*p) {
		const char *eol = strchr(p, '\n');
		size_t len = eol ? (size_t)(eol - p) : strlen(p);

		if (len >= 13 && strncmp(p, "Block devices", 13) == 0) {
			in_block = true;
		} else if (len >= 17 && strncmp(p, "Character devices", 17) == 0) {
			in_block = false;
		} else if (in_block && len > 0) {
			/* Format: "  9 md", "254 mdp", "252 ms". */
			const char *s = p;
			while (s < p + len && isspace((unsigned char)*s)) s++;
			if (s < p + len && isdigit((unsigned char)*s)) {
				int major = 0;
				while (s < p + len && isdigit((unsigned char)*s)) {
					major = major * 10 + (*s - '0');
					s++;
				}
				while (s < p + len && isspace((unsigned char)*s)) s++;
				size_t name_len = (p + len) - s;
				/* Trim trailing whitespace from name. */
				while (name_len > 0 &&
				       isspace((unsigned char)s[name_len - 1]))
					name_len--;
				if (name_len == 2 && strncmp(s, "md", 2) == 0) {
					if (md_major) *md_major = major;
				} else if (name_len == 3 && strncmp(s, "mdp", 3) == 0) {
					if (mdp_major) *mdp_major = major;
				} else if (name_len == 2 && strncmp(s, "ms", 2) == 0) {
					if (ms_major) *ms_major = major;
				}
			}
		}

		if (!eol) break;
		p = eol + 1;
	}
	return 0;
}

const struct subsys *path_to_subsys(const char *path)
{
	(void)path;
	return NULL; /* Task 4 */
}

const struct subsys *subsys_for_devnm(const char *devnm)
{
	(void)devnm;
	return NULL; /* Task 5 */
}

const struct subsys *subsys_for_major(unsigned int major)
{
	(void)major;
	return NULL; /* Task 5 */
}

const struct subsys *subsys_scan_argv(int argc, char **argv, bool *mixed_out)
{
	(void)argc; (void)argv;
	if (mixed_out) *mixed_out = false;
	return NULL; /* Task 6 */
}

void subsys_select(int argc, char **argv)
{
	(void)argc; (void)argv;
	/* Task 6 */
}

const char *dev_num_pref(void)     { return current_subsys->dev_prefix; }
size_t      dev_num_pref_len(void) { return strlen(current_subsys->dev_prefix); }
const char *dev_md_dir(void)       { return current_subsys->dev_dir; }
size_t      dev_md_dir_len(void)   { return strlen(current_subsys->dev_dir); }
