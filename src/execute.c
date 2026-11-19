#define _POSIX_C_SOURCE 200809L
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
int execute_line(const char *line) {
    char *copy = strdup(line), *save = NULL, *args[128]; int n=0, status=0;
    if (!copy) return 1;
    for (char *t=strtok_r(copy," \t",&save); t && n<127; t=strtok_r(NULL," \t",&save)) args[n++]=t;
    args[n]=NULL;
    if (!n) {free(copy);return 0;}
    pid_t pid=fork();
    if (pid==0) {execvp(args[0],args);perror(args[0]);_exit(127);}
    if (pid<0) {perror("fork");free(copy);return 1;}
    while(waitpid(pid,&status,0)<0) {}
    free(copy);return WIFEXITED(status)?WEXITSTATUS(status):128;
}
