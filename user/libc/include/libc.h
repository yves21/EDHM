#ifndef USER_LIBC_H
#define USER_LIBC_H

#include <stddef.h>
#include <sys/types.h>

#include "common/syscall.h"

void __libc_init(const syscall_table_t *table);

ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
void exit(int status) __attribute__((noreturn));
int spawn(const char *path, int argc, char *const argv[]);
int puts(const char *str);

size_t strlen(const char *str);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);
char *strchr(const char *s, int c);
void *memset(void *dest, int value, size_t count);
void *memcpy(void *dest, const void *src, size_t count);

#endif
