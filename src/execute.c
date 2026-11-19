#define _POSIX_C_SOURCE 200809L
#include "shell.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAXTOK 256
#define MAXJOBS 128
static struct {pid_t pid; char *cmd;} jobs[MAXJOBS];
static int njobs;
void reap_jobs(void) {
    int status;pid_t p;
    while ((p=waitpid(-1,&status,WNOHANG))>0) {
        for(int i=0;i<njobs;i++) if(jobs[i].pid==p) {
            free(jobs[i].cmd);memmove(&jobs[i],&jobs[i+1],(njobs-i-1)*sizeof(jobs[0]));njobs--;break;
        }
    }
}
void print_jobs(void) {reap_jobs();for(int i=0;i<njobs;i++) printf("[%d] %ld %s\n",i+1,(long)jobs[i].pid,jobs[i].cmd);}
static char *tokens[MAXTOK];static int nt;
static void clear_tokens(void) {for(int i=0;i<nt;i++)free(tokens[i]);nt=0;}
static int put_token(const char *s,size_t n) {if(nt>=MAXTOK-1)return -1;tokens[nt++]=strndup(s,n);return tokens[nt-1]?0:-1;}
static int tokenize(const char *line) {
    clear_tokens();const char *p=line;
    while(*p) {
        while(isspace((unsigned char)*p))p++;
        if(!*p)break;
        if(strchr("<>|;&",*p)) {if(put_token(p++,1))return -1;continue;}
        char buf[8192];size_t k=0;char quote=0;
        while(*p && (quote || (!isspace((unsigned char)*p) && !strchr("<>|;&",*p)))) {
            if(*p=='\\' && p[1]) {p++;if(k<sizeof(buf)-1)buf[k++]=*p++;continue;}
            if(*p=='\'' || *p=='"') {if(!quote){quote=*p++;continue;}if(quote==*p){quote=0;p++;continue;}}
            if(k<sizeof(buf)-1)buf[k++]=*p;
            p++;
        }
        if(quote) {fprintf(stderr,"unclosed quote\n");return -1;}
        if(put_token(buf,k))return -1;
    }
    return 0;
}
static int isop(const char *s) {return strlen(s)==1 && strchr("<>|;&",s[0])!=NULL;}
static int run_pipeline(int begin,int end,int background,const char *line) {
    int pipes[2], prev=-1, nchild=0;pid_t last=-1;
    pid_t children[MAXTOK];
    for(int pos=begin;pos<end;) {
        int stop=pos;while(stop<end && strcmp(tokens[stop],"|"))stop++;
        char *args[MAXTOK];int na=0;const char *in=NULL,*out=NULL;
        for(int i=pos;i<stop;i++) {
            if(!strcmp(tokens[i],"<") || !strcmp(tokens[i],">")) {
                if(i+1>=stop || isop(tokens[i+1])) {fprintf(stderr,"redirection: missing filename\n");if(prev!=-1)close(prev);return 2;}
                if(tokens[i][0]=='<')in=tokens[++i];else out=tokens[++i];
            } else args[na++]=tokens[i];
        }
        args[na]=NULL;
        if(!na) {fprintf(stderr,"empty command\n");if(prev!=-1)close(prev);return 2;}
        int next=(stop<end);
        if(next && pipe(pipes)<0) {perror("pipe");if(prev!=-1)close(prev);return 1;}
        pid_t pid=fork();
        if(pid==0) {
            if(prev!=-1){dup2(prev,STDIN_FILENO);close(prev);}
            if(next){close(pipes[0]);dup2(pipes[1],STDOUT_FILENO);close(pipes[1]);}
            if(in){int fd=open(in,O_RDONLY);if(fd<0){perror(in);_exit(1);}dup2(fd,STDIN_FILENO);close(fd);}
            if(out){int fd=open(out,O_WRONLY|O_CREAT|O_TRUNC,0666);if(fd<0){perror(out);_exit(1);}dup2(fd,STDOUT_FILENO);close(fd);}
            execvp(args[0],args);perror(args[0]);_exit(127);
        }
        if(pid<0){perror("fork");if(next){close(pipes[0]);close(pipes[1]);}if(prev!=-1)close(prev);return 1;}
        children[nchild++]=pid;last=pid;
        if(prev!=-1)close(prev);
        prev=next?pipes[0]:-1;
        if(next)close(pipes[1]);
        pos=stop+1;
    }
    if(prev!=-1)close(prev);
    if(background) {
        /* Do not block on background children; the next prompt reaps them. */
        for(int i=0;i<nchild;i++) if(njobs<MAXJOBS) {jobs[njobs].pid=children[i];jobs[njobs].cmd=strdup(line);njobs++;}
        printf("[background] %ld\n",(long)last);return 0;
    }
    int result=0;
    for(int i=0;i<nchild;i++) {int s=0;while(waitpid(children[i],&s,0)<0 && errno==EINTR){}if(children[i]==last)result=WIFEXITED(s)?WEXITSTATUS(s):128;}
    return result;
}
int execute_line(const char *line) {
    if(tokenize(line))return 2;
    int result=0,start=0;
    for(int i=0;i<=nt;i++) {
        if(i==nt || !strcmp(tokens[i],";") || !strcmp(tokens[i],"&")) {
            if(i>start)result=run_pipeline(start,i,i<nt&&!strcmp(tokens[i],"&"),line);
            start=i+1;
        }
    }
    clear_tokens();return result;
}
