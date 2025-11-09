#include "include/syscalls.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "include/process.h"

ssize_t ksys_write(int fd, const void *buf, size_t count) {
    if (!buf) {
        return -EINVAL;
    }
    ssize_t written = write(fd, buf, count);
    return written < 0 ? -errno : written;
}

ssize_t ksys_read(int fd, void *buf, size_t count) {
    if (!buf) {
        return -EINVAL;
    }
    ssize_t received = read(fd, buf, count);
    return received < 0 ? -errno : received;
}

void ksys_exit(int status) {
    process_force_exit(status);
}

int ksys_spawn(const char *path, int argc, const char *const argv[]) {
    if (!path) {
        return -EINVAL;
    }
    return process_execute(path, argc, argv);
}

syscall_table_t kernel_syscalls = {
    .write = ksys_write,
    .read = ksys_read,
    .exit = ksys_exit,
    .spawn = ksys_spawn,
};
