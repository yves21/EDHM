#include "include/libc.h"

static const syscall_table_t *syscalls = NULL;

void __libc_init(const syscall_table_t *table) {
    syscalls = table;
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (!syscalls || !syscalls->write) {
        return -1;
    }
    return syscalls->write(fd, buf, count);
}

ssize_t read(int fd, void *buf, size_t count) {
    if (!syscalls || !syscalls->read) {
        return -1;
    }
    return syscalls->read(fd, buf, count);
}

void exit(int status) {
    if (syscalls && syscalls->exit) {
        syscalls->exit(status);
    }
    while (1) {
    }
}

int spawn(const char *path, int argc, char *const argv[]) {
    if (!syscalls || !syscalls->spawn) {
        return -1;
    }
    return syscalls->spawn(path, argc, (const char *const *)argv);
}

int puts(const char *str) {
    size_t len = strlen(str);
    ssize_t written = write(1, str, len);
    if (written < 0) {
        return -1;
    }
    if (write(1, "\n", 1) < 0) {
        return -1;
    }
    return 0;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }
    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        ++a;
        ++b;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    size_t i = 0;
    for (; i < n; ++i) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca != cb) {
            return ca - cb;
        }
        if (ca == '\0') {
            return 0;
        }
    }
    return 0;
}

char *strcpy(char *dst, const char *src) {
    char *orig = dst;
    while ((*dst++ = *src++) != '\0') {
    }
    return orig;
}

char *strcat(char *dst, const char *src) {
    char *orig = dst;
    while (*dst) {
        ++dst;
    }
    while ((*dst++ = *src++) != '\0') {
    }
    return orig;
}

char *strchr(const char *s, int c) {
    char ch = (char)c;
    while (*s) {
        if (*s == ch) {
            return (char *)s;
        }
        ++s;
    }
    if (ch == '\0') {
        return (char *)s;
    }
    return NULL;
}

void *memset(void *dest, int value, size_t count) {
    unsigned char *ptr = (unsigned char *)dest;
    for (size_t i = 0; i < count; ++i) {
        ptr[i] = (unsigned char)value;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < count; ++i) {
        d[i] = s[i];
    }
    return dest;
}
