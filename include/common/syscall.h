#ifndef COMMON_SYSCALL_H
#define COMMON_SYSCALL_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct syscall_table {
    ssize_t (*write)(int fd, const void *buf, size_t count);
    ssize_t (*read)(int fd, void *buf, size_t count);
    void (*exit)(int status);
    int (*spawn)(const char *path, int argc, const char *const argv[]);
} syscall_table_t;

#endif
