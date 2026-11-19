#define _POSIX_C_SOURCE 200809L
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#if __has_include(<readline/readline.h>)
#include <readline/readline.h>
#include <readline/history.h>
#else
extern char *readline(const char *);
extern void add_history(const char *);
#endif
#define HISTORY_SIZE 100
static char *history[HISTORY_SIZE];
static int count=0;
static void add_cmd(const char *cmd) {
    if (count==HISTORY_SIZE) { free(history[0]); memmove(history,history+1,(HISTORY_SIZE-1)*sizeof(char*)); count--; }
    history[count++]=strdup(cmd);
}
static int read_input(char **line,size_t *cap,const char *prompt) {
    if(isatty(STDIN_FILENO)) {free(*line);*line=readline(prompt);if(!*line)return 0;*cap=strlen(*line)+1;}
    else {if(getline(line,cap,stdin)<0)return 0;(*line)[strcspn(*line,"\n")]=0;}
    return 1;
}
static int run_if(const char *condition,char **line,size_t *cap) {
    int success=(execute_line(condition)==0), mode=0, result=0;
    while(read_input(line,cap,"if> ")) {
        if(!strcmp(*line,"then")){mode=1;continue;}
        if(!strcmp(*line,"else")){mode=2;continue;}
        if(!strcmp(*line,"fi"))return result;
        if(mode==0){fprintf(stderr,"if: expected then\n");return 2;}
        if((success && mode==1)||(!success && mode==2))result=execute_line(*line);
    }
    fprintf(stderr,"if: missing fi\n");return 2;
}
int shell_loop(void) {
    char *line = NULL; size_t cap = 0;
    while (1) {
        reap_jobs();
        if (!read_input(&line,&cap,"myshell$ ")) break;
        if (!strncmp(line,"if ",3)) {
            char *condition=strdup(line+3);
            if(condition){run_if(condition,&line,&cap);free(condition);}
            continue;
        }
        if (*line) add_history(line);
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
        if (assign_variable(line)) continue;
        if (!strcmp(line,"set")) {print_variables();continue;}
        if (!strcmp(line,"exit")) break;
        if (!strncmp(line,"cd",2) && (line[2]==0 || line[2]==' ')) {
            char *dir=line+2; while (*dir==' ') dir++;
            if (!*dir) dir=getenv("HOME");
            if (!dir || chdir(dir)) perror("cd");
            continue;
        }
        if (!strcmp(line, "help")) {
    puts("========== MyShell Help ==========");
    puts("exit          - Exit the shell");
    puts("cd <dir>      - Change directory");
    puts("help          - Display this help message");
    puts("jobs          - List background jobs");
    puts("history       - Show command history");
    puts("!n            - Execute history command number n");
    puts("set           - Display shell variables");
    puts("NAME=value    - Assign a shell variable");
    puts("==================================");
    continue;
}
        if (!strcmp(line,"jobs")) { print_jobs(); continue; }
        if (*line) execute_line(line);
    }
    free(line); for(int i=0;i<count;i++) free(history[i]); free_variables(); return 0;
}
