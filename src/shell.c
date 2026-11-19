#define _POSIX_C_SOURCE 200809L
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define HISTORY_SIZE 100
static char *history[HISTORY_SIZE];
static int count=0;
static void add_cmd(const char *cmd) {
    if (count==HISTORY_SIZE) { free(history[0]); memmove(history,history+1,(HISTORY_SIZE-1)*sizeof(char*)); count--; }
    history[count++]=strdup(cmd);
}
int shell_loop(void) {
    char *line = NULL; size_t cap = 0;
    while (1) {
        if (isatty(STDIN_FILENO)) { fputs("myshell$ ", stdout); fflush(stdout); }
        if (getline(&line, &cap, stdin) < 0) break;
        line[strcspn(line,"\n")] = 0;
        if (line[0]=='!' && line[1]) {
            char *end; long n=strtol(line+1,&end,10);
            if (*end || n<1 || n>count) {fprintf(stderr,"history: invalid event\n");continue;}
            char *expanded=strdup(history[n-1]);
            if (!expanded) continue;
            free(line); line=expanded;cap=strlen(line)+1;
            if (isatty(STDIN_FILENO)) puts(line);
        }
        if (*line) add_cmd(line);
        if (!strcmp(line,"history")) {
            for(int i=0;i<count;i++) printf("%d %s\n",i+1,history[i]);
            continue;
        }
        if (!strcmp(line,"exit")) break;
        if (!strncmp(line,"cd",2) && (line[2]==0 || line[2]==' ')) {
            char *dir=line+2; while (*dir==' ') dir++;
            if (!*dir) dir=getenv("HOME");
            if (!dir || chdir(dir)) perror("cd");
            continue;
        }
        if (!strcmp(line,"help")) { puts("Built-ins: exit, cd, help, jobs, history"); continue; }
        if (!strcmp(line,"jobs")) { puts("Job control not yet implemented."); continue; }
        if (*line) execute_line(line);
    }
    free(line); for(int i=0;i<count;i++) free(history[i]); return 0;
}
