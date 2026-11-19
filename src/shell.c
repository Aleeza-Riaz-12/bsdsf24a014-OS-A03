#define _POSIX_C_SOURCE 200809L
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int shell_loop(void) {
    char *line = NULL; size_t cap = 0;
    while (1) {
        if (isatty(STDIN_FILENO)) { fputs("myshell$ ", stdout); fflush(stdout); }
        if (getline(&line, &cap, stdin) < 0) break;
        line[strcspn(line,"\n")] = 0;
        if (!strcmp(line,"exit")) break;
        if (*line) execute_line(line);
    }
    free(line); return 0;
}
