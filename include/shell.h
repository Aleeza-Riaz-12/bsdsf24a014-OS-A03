#ifndef MYSHELL_H
#define MYSHELL_H
int shell_loop(void);
int execute_line(const char *line);
void reap_jobs(void);
void print_jobs(void);
#endif
