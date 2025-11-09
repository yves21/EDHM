#ifndef KERNEL_SYSCALLS_H
#define KERNEL_SYSCALLS_H

#include <stddef.h>
#include <sys/types.h>

#include "common/syscall.h"

ssize_t ksys_write(int fd, const void *buf, size_t count);
ssize_t ksys_read(int fd, void *buf, size_t count);
void ksys_exit(int status);
int ksys_spawn(const char *path, int argc, const char *const argv[]);

extern syscall_table_t kernel_syscalls;

#endif
