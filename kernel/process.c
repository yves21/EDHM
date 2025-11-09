#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "include/process.h"

#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "exec/elf_loader.h"
#include "include/syscalls.h"

#define USER_APP_DIR "build/user/apps"

typedef int (*user_entry_fn)(const syscall_table_t *, int, char **);

typedef struct runtime_process {
    char path[PATH_MAX];
    void *dl_handle;
    jmp_buf env;
    int exit_code;
} runtime_process_t;

static runtime_process_t *current_process = NULL;

static int resolve_app_path(const char *path, char *resolved) {
    if (!path || !resolved) {
        return -EINVAL;
    }

    if (path[0] == '/' || strncmp(path, "./", 2) == 0) {
        if (realpath(path, resolved) == NULL) {
            return -errno;
        }
        return 0;
    }

    char candidate[PATH_MAX];
    const char *extension = ".so";
    bool has_ext = false;
    size_t len = strlen(path);
    if (len >= 3 && strcmp(path + len - 3, extension) == 0) {
        has_ext = true;
    }

    const char *search_paths[] = { USER_APP_DIR, "user/apps" };
    for (size_t i = 0; i < sizeof(search_paths) / sizeof(search_paths[0]); ++i) {
        if (has_ext) {
            snprintf(candidate, sizeof(candidate), "%s/%s", search_paths[i], path);
        } else {
            snprintf(candidate, sizeof(candidate), "%s/%s%s", search_paths[i], path, extension);
        }

        if (realpath(candidate, resolved) != NULL) {
            return 0;
        }
    }

    return -ENOENT;
}

static int invoke_entry(runtime_process_t *process, const char *path, int argc, const char *const argv[]) {
    elf_image_t image;
    int ret = elf_loader_inspect(path, &image);
    if (ret < 0) {
        fprintf(stderr, "[kernel] failed to inspect %s (%d)\n", path, ret);
        return ret;
    }

    if (!image.has_user_entry) {
        fprintf(stderr, "[kernel] %s lacks user_entry symbol\n", path);
        return -ENOEXEC;
    }

    void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "[kernel] dlopen failed: %s\n", dlerror());
        return -ENOEXEC;
    }

    process->dl_handle = handle;

    user_entry_fn entry = (user_entry_fn)dlsym(handle, "user_entry");
    if (!entry) {
        fprintf(stderr, "[kernel] user_entry not found via dlsym\n");
        dlclose(handle);
        process->dl_handle = NULL;
        return -ENOEXEC;
    }

    char **argv_copy = calloc((size_t)argc + 1, sizeof(char *));
    if (!argv_copy) {
        dlclose(handle);
        process->dl_handle = NULL;
        return -ENOMEM;
    }

    for (int i = 0; i < argc; ++i) {
        if (argv[i]) {
            argv_copy[i] = strdup(argv[i]);
        } else {
            argv_copy[i] = NULL;
        }
    }
    argv_copy[argc] = NULL;

    current_process = process;

    if (setjmp(process->env) == 0) {
        process->exit_code = entry(&kernel_syscalls, argc, argv_copy);
    }

    for (int i = 0; i < argc; ++i) {
        free(argv_copy[i]);
    }
    free(argv_copy);

    current_process = NULL;
    return process->exit_code;
}

int process_execute(const char *path, int argc, const char *const argv[]) {
    char resolved[PATH_MAX];
    int ret = resolve_app_path(path, resolved);
    if (ret < 0) {
        fprintf(stderr, "[kernel] unable to resolve %s (%d)\n", path, ret);
        return ret;
    }

    runtime_process_t process = {0};
    strncpy(process.path, resolved, sizeof(process.path) - 1);

    int exit_code = invoke_entry(&process, resolved, argc, argv);

    if (process.dl_handle) {
        dlclose(process.dl_handle);
        process.dl_handle = NULL;
    }

    return exit_code;
}

void process_force_exit(int status) {
    if (!current_process) {
        exit(status);
    }

    current_process->exit_code = status;
    longjmp(current_process->env, 1);
}
