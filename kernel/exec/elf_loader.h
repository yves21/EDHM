#ifndef KERNEL_EXEC_ELF_LOADER_H
#define KERNEL_EXEC_ELF_LOADER_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct elf_image {
    char path[PATH_MAX];
    uint16_t type;
    uint16_t machine;
    uint64_t entry;
    bool has_user_entry;
} elf_image_t;

int elf_loader_inspect(const char *path, elf_image_t *image);

#endif
