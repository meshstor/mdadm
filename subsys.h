/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * subsys.h - md/ms subsystem descriptor and selection.
 *
 * mdadm manages two parallel kernel RAID subsystems:
 *   - md: kernel built-in, /dev/md*, /proc/mdstat, /sys/block/mdX/md/, major 9
 *   - ms: meshstor parallel subsystem, /dev/ms*, /proc/msstat,
 *         /sys/block/msX/ms/, dynamic major
 *
 * One mdadm invocation operates on exactly one subsystem, chosen by
 * subsys_select() at startup. See docs/superpowers/specs/2026-05-02-
 * mdadm-ms-subsystem-design.md.
 */
#ifndef MDADM_SUBSYS_H
#define MDADM_SUBSYS_H

#include <stdbool.h>
#include <stddef.h>

struct subsys {
	const char *name;              /* "md" / "ms" */
	const char *dev_prefix;        /* "/dev/md" / "/dev/ms" */
	const char *dev_dir;           /* "/dev/md/" / "/dev/ms/" */
	const char *proc_stat;         /* "/proc/mdstat" / "/proc/msstat" */
	const char *sysfs_subdir;      /* "md" / "ms" */
	const char *devnm_prefix;      /* "md" / "ms" */
	const char *devnm_part_prefix; /* "md_d" or NULL (ms has no mdp analog) */
	int         major;             /* 9 for md, dynamic for ms (filled at startup) */
	int         mdp_major;         /* dynamic for md, -1 for ms */
};

extern struct subsys        subsys_md;
extern struct subsys        subsys_ms;
extern const struct subsys *current_subsys;

/* Called once from main() before option parsing. Aborts on conflicts
 * (mixed-subsystem invocation, --subsys=ms when ms unavailable). */
void subsys_select(int argc, char **argv);

/* Disambiguators - work against both subsystems unconditionally,
 * regardless of current_subsys. */
const struct subsys *subsys_for_devnm(const char *devnm);
const struct subsys *subsys_for_major(unsigned int major);
const struct subsys *path_to_subsys(const char *path);
bool                 subsys_available(const struct subsys *s);

/* Convenience accessors (replace removed DEV_NUM_PREF/DEV_MD_DIR macros). */
const char *dev_num_pref(void);
size_t      dev_num_pref_len(void);
const char *dev_md_dir(void);
size_t      dev_md_dir_len(void);

/* Test seams - exposed for test-subsys.c. Not part of the public API
 * for the rest of mdadm. */
int  subsys_parse_proc_devices(const char *content, int *md_major,
			       int *mdp_major, int *ms_major);
const struct subsys *subsys_scan_argv(int argc, char **argv,
				      bool *mixed_out);

#endif /* MDADM_SUBSYS_H */
