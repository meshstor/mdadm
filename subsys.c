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
	if (!path || !*path)
		return NULL;

	/* Strip leading "/dev/" if present. */
	const char *p = path;
	if (strncmp(p, "/dev/", 5) == 0)
		p += 5;
	if (!*p)
		return NULL;

	struct subsys *candidates[2] = { &subsys_md, &subsys_ms };
	for (int i = 0; i < 2; i++) {
		const struct subsys *s = candidates[i];
		size_t plen = strlen(s->devnm_prefix);

		/* Named-device dir: "<devnm_prefix>/<name>" */
		if (strncmp(p, s->devnm_prefix, plen) == 0 &&
		    p[plen] == '/' && p[plen + 1] != '\0')
			return s;

		/* Partitionable: "md_dN" (md only) */
		if (s->devnm_part_prefix) {
			size_t pplen = strlen(s->devnm_part_prefix);
			if (strncmp(p, s->devnm_part_prefix, pplen) == 0 &&
			    isdigit((unsigned char)p[pplen]))
				return s;
		}

		/* Numbered: "<devnm_prefix>N" */
		if (strncmp(p, s->devnm_prefix, plen) == 0 &&
		    isdigit((unsigned char)p[plen]))
			return s;
	}
	return NULL;
}

const struct subsys *subsys_for_devnm(const char *devnm)
{
	if (!devnm || !*devnm)
		return NULL;
	struct subsys *cs[2] = { &subsys_md, &subsys_ms };
	for (int i = 0; i < 2; i++) {
		struct subsys *s = cs[i];
		size_t plen = strlen(s->devnm_prefix);
		if (s->devnm_part_prefix) {
			size_t pplen = strlen(s->devnm_part_prefix);
			if (strncmp(devnm, s->devnm_part_prefix, pplen) == 0 &&
			    isdigit((unsigned char)devnm[pplen]))
				return s;
		}
		if (strncmp(devnm, s->devnm_prefix, plen) == 0 &&
		    isdigit((unsigned char)devnm[plen]))
			return s;
	}
	return NULL;
}

const struct subsys *subsys_for_major(unsigned int major)
{
	struct subsys *cs[2] = { &subsys_md, &subsys_ms };
	for (int i = 0; i < 2; i++) {
		struct subsys *s = cs[i];
		if (s->major > 0 && (unsigned int)s->major == major)
			return s;
		if (s->mdp_major > 0 && (unsigned int)s->mdp_major == major)
			return s;
	}
	return NULL;
}

const struct subsys *subsys_scan_argv(int argc, char **argv, bool *mixed_out)
{
	const struct subsys *found = NULL;
	if (mixed_out) *mixed_out = false;

	for (int i = 1; i < argc; i++) {
		const char *t = argv[i];
		if (!t || !*t)
			continue;
		if (t[0] == '-')
			continue;          /* option flag */
		if (strchr(t, '='))
			continue;          /* key=value option arg */
		const struct subsys *s = path_to_subsys(t);
		if (!s)
			continue;
		if (found && found != s) {
			if (mixed_out) *mixed_out = true;
			return NULL;
		}
		found = s;
	}
	return found;
}

static int read_file_to_buffer(const char *path, char *buf, size_t bufsz)
{
	FILE *f = fopen(path, "re");
	if (!f) return -1;
	size_t n = fread(buf, 1, bufsz - 1, f);
	fclose(f);
	buf[n] = '\0';
	return 0;
}

static const struct subsys *find_subsys_flag_in_argv(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		const char *t = argv[i];
		if (!t) continue;
		const char *val = NULL;
		if (strncmp(t, "--subsys=", 9) == 0) {
			val = t + 9;
		} else if (strcmp(t, "--subsys") == 0 && i + 1 < argc) {
			val = argv[i + 1];
		}
		if (!val) continue;
		if (strcmp(val, "md") == 0) return &subsys_md;
		if (strcmp(val, "ms") == 0) return &subsys_ms;
		if (strcmp(val, "auto") == 0) return NULL;
		fprintf(stderr,
			"mdadm: --subsys=%s is not a known subsystem (use md, ms, or auto)\n",
			val);
		exit(2);
	}
	return NULL;
}

void subsys_select(int argc, char **argv)
{
	/* Always discover majors so subsys_for_major works for both. */
	char buf[8192];
	if (read_file_to_buffer("/proc/devices", buf, sizeof(buf)) == 0) {
		int md_m = -1, mdp_m = -1, ms_m = -1;
		subsys_parse_proc_devices(buf, &md_m, &mdp_m, &ms_m);
		if (md_m  > 0) subsys_md.major     = md_m;
		if (mdp_m > 0) subsys_md.mdp_major = mdp_m;
		if (ms_m  > 0) subsys_ms.major     = ms_m;
	}

	const struct subsys *chosen = NULL;

	/* 1. Explicit --subsys flag. */
	chosen = find_subsys_flag_in_argv(argc, argv);

	/* 2. Path-based detection from argv. */
	if (!chosen) {
		bool mixed = false;
		chosen = subsys_scan_argv(argc, argv, &mixed);
		if (mixed) {
			fprintf(stderr,
				"mdadm: cannot operate on md and ms devices in the same invocation\n");
			exit(2);
		}
	}

	/* 3. Env var. */
	if (!chosen) {
		const char *e = getenv("MDADM_SUBSYS");
		if (e && strcmp(e, "auto") != 0) {
			if (strcmp(e, "md") == 0) chosen = &subsys_md;
			else if (strcmp(e, "ms") == 0) chosen = &subsys_ms;
			else {
				fprintf(stderr,
					"mdadm: MDADM_SUBSYS=%s is not a known subsystem\n", e);
				exit(2);
			}
		}
	}

	/* 4. Default: ms if /proc/msstat exists, else md.
	 *    Note: --scan operations against a config file that targets the
	 *    other subsystem require explicit --subsys= or MDADM_SUBSYS to
	 *    override. We don't pre-parse the config file here to keep
	 *    subsystem selection deterministic. */
	if (!chosen)
		chosen = subsys_available(&subsys_ms) ? &subsys_ms : &subsys_md;

	/* Verify availability of the chosen subsystem. */
	if (chosen == &subsys_ms && !subsys_available(&subsys_ms)) {
		fprintf(stderr,
			"mdadm: --subsys=ms requested but the 'ms' block device class is not registered.\n"
			"       Is the meshstor-md kernel module loaded?\n");
		exit(2);
	}

	current_subsys = chosen;
}

const char *dev_num_pref(void)     { return current_subsys->dev_prefix; }
size_t      dev_num_pref_len(void) { return strlen(current_subsys->dev_prefix); }
const char *dev_md_dir(void)       { return current_subsys->dev_dir; }
size_t      dev_md_dir_len(void)   { return strlen(current_subsys->dev_dir); }
