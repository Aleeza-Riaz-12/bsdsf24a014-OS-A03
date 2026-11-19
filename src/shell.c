#define _POSIX_C_SOURCE 200809L
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
int shell_loop(void) {
    char *line = NULL; size_t cap = 0;
    while (1) {
        if (isatty(STDIN_FILENO)) { fputs("myshell$ ", stdout); fflush(stdout); }
        if (getline(&line, &cap, stdin) < 0) break;
        line[strcspn(line,"\n")] = 0;
        if (!strcmp(line,"exit")) break;
        if (!strncmp(line,"cd",2) && (line[2]==0 || line[2]==' ')) {
            char *dir=line+2; while (*dir==' ') dir++;
            if (!*dir) dir=getenv("HOME");
            if (!dir || chdir(dir)) perror("cd");
            continue;
        }
        if (!strcmp(line,"help")) { puts("Built-ins: exit, cd, help, jobs"); continue; }
        if (!strcmp(line,"jobs")) { puts("Job control not yet implemented."); continue; }
        if (*line) execute_line(line);
    }
    free(line); return 0;
}
