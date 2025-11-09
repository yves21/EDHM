#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include <stddef.h>

#include "common/syscall.h"

int process_execute(const char *path, int argc, const char *const argv[]);
void process_force_exit(int status);

#endif
