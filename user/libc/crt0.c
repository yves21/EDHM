#include "include/libc.h"

extern int main(int argc, char **argv);

__attribute__((visibility("default")))
int user_entry(const syscall_table_t *table, int argc, char **argv) {
    __libc_init(table);
    return main(argc, argv);
}
