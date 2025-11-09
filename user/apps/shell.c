#include "../libc/include/libc.h"

#define SHELL_MAX_LINE 256
#define SHELL_MAX_ARGS 8

static void print_prompt(void) {
    write(1, "> ", 2);
}

static int read_line(char *buffer, size_t size) {
    size_t pos = 0;
    while (pos < size - 1) {
        char ch;
        ssize_t rc = read(0, &ch, 1);
        if (rc <= 0) {
            if (pos == 0) {
                return (int)rc;
            }
            break;
        }
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            break;
        }
        buffer[pos++] = ch;
    }
    buffer[pos] = '\0';
    return (int)pos;
}

static int tokenize(char *line, char *argv[], int max_args) {
    int count = 0;
    char *cursor = line;
    while (*cursor && count < max_args) {
        while (*cursor == ' ' || *cursor == '\t') {
            ++cursor;
        }
        if (*cursor == '\0') {
            break;
        }
        argv[count++] = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\t') {
            ++cursor;
        }
        if (*cursor == '\0') {
            break;
        }
        *cursor = '\0';
        ++cursor;
    }
    argv[count] = NULL;
    return count;
}

static void print_help(void) {
    puts("Built-in commands:");
    puts("  help - display this message");
    puts("  exit - leave the shell");
    puts("External commands are resolved from build/user/apps");
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    char line[SHELL_MAX_LINE];
    char *args[SHELL_MAX_ARGS + 1];

    while (1) {
        print_prompt();
        int len = read_line(line, sizeof(line));
        if (len < 0) {
            puts("exit");
            return 0;
        }
        if (len == 0) {
            continue;
        }

        int arg_count = tokenize(line, args, SHELL_MAX_ARGS);
        if (arg_count == 0) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            return 0;
        }
        if (strcmp(args[0], "help") == 0) {
            print_help();
            continue;
        }

        int rc = spawn(args[0], arg_count, args);
        if (rc < 0) {
            puts("Command failed");
        }
    }
}
