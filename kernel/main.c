#include <stdio.h>
#include <string.h>

#include "include/process.h"

static int run_default_shell(void) {
    const char *argv[] = {"shell", NULL};
    return process_execute("shell", 1, argv);
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        printf("[kernel] launching default shell...\n");
        int rc = run_default_shell();
        printf("[kernel] shell exited with status %d\n", rc);
        return rc;
    }

    const char *path = argv[1];
    const char *const *prog_argv = (const char *const *)&argv[1];
    int prog_argc = argc - 1;

    int rc = process_execute(path, prog_argc, prog_argv);
    if (rc < 0) {
        fprintf(stderr, "[kernel] failed to execute %s (rc=%d)\n", path, rc);
    }
    return rc;
}
