#include "../libc/include/libc.h"

int main(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        const char *word = argv[i];
        if (!word) {
            continue;
        }
        write(1, word, strlen(word));
        if (i + 1 < argc) {
            write(1, " ", 1);
        }
    }
    write(1, "\n", 1);
    return 0;
}
