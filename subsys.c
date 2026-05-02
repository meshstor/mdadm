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
	(void)content; (void)md_major; (void)mdp_major; (void)ms_major;
	return -1; /* Task 3 */
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
