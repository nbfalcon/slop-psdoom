#define _GNU_SOURCE
#include <linux/landlock.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i_landlock.h"
#include "m_argv.h"

#ifndef __NR_landlock_create_ruleset
#if defined(__x86_64__)
#define __NR_landlock_create_ruleset 444
#define __NR_landlock_add_rule 445
#define __NR_landlock_restrict_self 446
#endif
#endif

static int sys_landlock_create_ruleset(const struct landlock_ruleset_attr *attr, size_t size, __u32 flags)
{
    return (int)syscall(__NR_landlock_create_ruleset, attr, size, flags);
}

static int sys_landlock_add_rule(int ruleset_fd, enum landlock_rule_type rule_type, const void *rule_attr, __u32 flags)
{
    return (int)syscall(__NR_landlock_add_rule, ruleset_fd, rule_type, rule_attr, flags);
}

static int sys_landlock_restrict_self(int ruleset_fd, __u32 flags)
{
    return (int)syscall(__NR_landlock_restrict_self, ruleset_fd, flags);
}

static void landlock_add_path(int ruleset_fd, const char *path, __u64 allowed_access)
{
    if (!path || !path[0])
        return;

    int fd = open(path, O_PATH | O_CLOEXEC);
    if (fd < 0)
        return;

    struct landlock_path_beneath_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.parent_fd = fd;
    attr.allowed_access = allowed_access;

    sys_landlock_add_rule(ruleset_fd, LANDLOCK_RULE_PATH_BENEATH, &attr, 0);
    close(fd);
}

void I_InitLandlock(void)
{
    if (M_CheckParm("-nosandbox")) {
        printf("Landlock sandbox disabled by command line (-nosandbox).\n");
        return;
    }

    int abi = (int)syscall(__NR_landlock_create_ruleset, NULL, 0, LANDLOCK_CREATE_RULESET_VERSION);
    if (abi < 1) {
        printf("Landlock not supported by current kernel (ABI %d), skipping sandbox.\n", abi);
        return;
    }

    __u64 read_only = LANDLOCK_ACCESS_FS_READ_FILE | LANDLOCK_ACCESS_FS_READ_DIR;
    __u64 read_write = read_only | LANDLOCK_ACCESS_FS_WRITE_FILE |
                       LANDLOCK_ACCESS_FS_MAKE_CHAR | LANDLOCK_ACCESS_FS_MAKE_DIR |
                       LANDLOCK_ACCESS_FS_MAKE_REG | LANDLOCK_ACCESS_FS_MAKE_SOCK |
                       LANDLOCK_ACCESS_FS_MAKE_FIFO | LANDLOCK_ACCESS_FS_MAKE_SYM |
                       LANDLOCK_ACCESS_FS_REMOVE_DIR | LANDLOCK_ACCESS_FS_REMOVE_FILE;
    __u64 handled = read_write | LANDLOCK_ACCESS_FS_EXECUTE;

    struct landlock_ruleset_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.handled_access_fs = handled;

    int ruleset_fd = sys_landlock_create_ruleset(&attr, sizeof(attr), 0);
    if (ruleset_fd < 0) {
        fprintf(stderr, "Landlock: unable to create ruleset, sandbox not enabled.\n");
        return;
    }

    // Essential system directories for libraries, fonts, configurations, and devices
    landlock_add_path(ruleset_fd, "/usr", read_only);
    landlock_add_path(ruleset_fd, "/lib", read_only);
    landlock_add_path(ruleset_fd, "/lib64", read_only);
    landlock_add_path(ruleset_fd, "/etc", read_only);
    landlock_add_path(ruleset_fd, "/proc", read_only);
    landlock_add_path(ruleset_fd, "/sys", read_only);
    landlock_add_path(ruleset_fd, "/dev", read_only);

    // Audio/display socket runtime directory and temporary storage
    landlock_add_path(ruleset_fd, "/tmp", read_write);
    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (runtime_dir)
        landlock_add_path(ruleset_fd, runtime_dir, read_write);

    // User home directory (for ~/.psdoom saves, configuration)
    const char *home = getenv("HOME");
    if (home)
        landlock_add_path(ruleset_fd, home, read_write);

    // Current working directory and DOOMWADDIR
    landlock_add_path(ruleset_fd, ".", read_write);
    const char *doomwaddir = getenv("DOOMWADDIR");
    if (doomwaddir)
        landlock_add_path(ruleset_fd, doomwaddir, read_only);

    // Apply sandbox
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0) {
        fprintf(stderr, "Landlock: prctl(PR_SET_NO_NEW_PRIVS) failed.\n");
        close(ruleset_fd);
        return;
    }

    if (sys_landlock_restrict_self(ruleset_fd, 0) < 0) {
        fprintf(stderr, "Landlock: restrict_self failed.\n");
        close(ruleset_fd);
        return;
    }

    close(ruleset_fd);
    printf("Landlock sandbox enabled (ABI v%d): execution disallowed, host filesystem isolated.\n", abi);
}
