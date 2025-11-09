#include "../libc/include/libc.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    char buffer[128];
    while (1) {
        ssize_t count = read(0, buffer, sizeof(buffer));
        if (count <= 0) {
            break;
        }
        write(1, buffer, (size_t)count);
    }
    return 0;
}
