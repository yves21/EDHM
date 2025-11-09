#include "elf_loader.h"

#include <elf.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool symbol_table_has_entry(FILE *file, const Elf64_Shdr *sections, size_t count, size_t index, const char *target) {
    if (index >= count) {
        return false;
    }

    const Elf64_Shdr *sym_hdr = &sections[index];
    if (sym_hdr->sh_entsize == 0 || sym_hdr->sh_size == 0) {
        return false;
    }

    size_t entry_count = sym_hdr->sh_size / sym_hdr->sh_entsize;

    if (sym_hdr->sh_link >= count) {
        return false;
    }

    const Elf64_Shdr *str_hdr = &sections[sym_hdr->sh_link];
    if (str_hdr->sh_size == 0) {
        return false;
    }

    char *strings = malloc(str_hdr->sh_size);
    if (!strings) {
        return false;
    }

    if (fseek(file, str_hdr->sh_offset, SEEK_SET) != 0) {
        free(strings);
        return false;
    }

    if (fread(strings, 1, str_hdr->sh_size, file) != str_hdr->sh_size) {
        free(strings);
        return false;
    }

    if (fseek(file, sym_hdr->sh_offset, SEEK_SET) != 0) {
        free(strings);
        return false;
    }

    bool found = false;
    for (size_t i = 0; i < entry_count; ++i) {
        Elf64_Sym sym;
        if (fread(&sym, 1, sizeof(sym), file) != sizeof(sym)) {
            found = false;
            break;
        }
        if (sym.st_name >= str_hdr->sh_size) {
            continue;
        }
        const char *name = &strings[sym.st_name];
        if (name && strcmp(name, target) == 0) {
            found = true;
            break;
        }
    }

    free(strings);
    return found;
}

int elf_loader_inspect(const char *path, elf_image_t *image) {
    if (!path || !image) {
        return -EINVAL;
    }

    FILE *file = fopen(path, "rb");
    if (!file) {
        return -errno;
    }

    Elf64_Ehdr header;
    if (fread(&header, 1, sizeof(header), file) != sizeof(header)) {
        fclose(file);
        return -EIO;
    }

    if (memcmp(header.e_ident, ELFMAG, SELFMAG) != 0) {
        fclose(file);
        return -ENOEXEC;
    }

    if (header.e_ident[EI_CLASS] != ELFCLASS64 || header.e_ident[EI_DATA] != ELFDATA2LSB) {
        fclose(file);
        return -ENOEXEC;
    }

    if (header.e_machine != EM_X86_64) {
        fclose(file);
        return -ENOEXEC;
    }

    if (header.e_type != ET_DYN && header.e_type != ET_EXEC) {
        fclose(file);
        return -ENOEXEC;
    }

    strncpy(image->path, path, sizeof(image->path) - 1);
    image->path[sizeof(image->path) - 1] = '\0';
    image->type = header.e_type;
    image->machine = header.e_machine;
    image->entry = header.e_entry;
    image->has_user_entry = false;

    if (header.e_shoff == 0 || header.e_shentsize == 0 || header.e_shnum == 0) {
        fclose(file);
        return 0;
    }

    if (fseek(file, header.e_shoff, SEEK_SET) != 0) {
        fclose(file);
        return -EIO;
    }

    size_t section_bytes = header.e_shentsize * header.e_shnum;
    Elf64_Shdr *sections = malloc(section_bytes);
    if (!sections) {
        fclose(file);
        return -ENOMEM;
    }

    if (fread(sections, header.e_shentsize, header.e_shnum, file) != header.e_shnum) {
        free(sections);
        fclose(file);
        return -EIO;
    }

    for (size_t i = 0; i < header.e_shnum; ++i) {
        if (sections[i].sh_type == SHT_SYMTAB || sections[i].sh_type == SHT_DYNSYM) {
            if (symbol_table_has_entry(file, sections, header.e_shnum, i, "user_entry")) {
                image->has_user_entry = true;
                break;
            }
        }
    }

    free(sections);
    fclose(file);
    return 0;
}
