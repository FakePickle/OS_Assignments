#ifndef BASH_H
#define BASH_H
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>


#define DELIMITER " \t\r\n\a\""

#define SUCCESS 0
#define FAILED 1
#define MAX_BACKGROUND_PROCESSES 100
#define MAX_TOKENS 64
#define MAX_HISTORY_SIZE 1024

/*Shell Interaction ie Terminal*/
struct history{
    int sno;
    pid_t pid;
    char **cmd;
    time_t time;
    double execution_time;
};

struct BackgroundProcess{
    int sno;
    pid_t pid;
    char cmd[MAX_TOKENS];
};

void shell_interactive(void);  /*Harsh done*/
/*Required by shell_interactive*/
char *read_line(void); /* Adi done*/
char **parse(char *line); /* Harsh done*/
int exec_cmd(char **cmd); /* Harsh done*/
void execute_piped_commands(char **cmd);
/*Required for execute_cmds*/
int new_process(char **cmd, int input_fd, int output_fd); /*Harsh done*/
char** slice_command(char **cmd, int start, int end);/*Adi done*/

/*Commands implemented on our own(NOT MENTIONED IN THE ASSIGNMENT)*/
int help(char **cmd); /*Harsh* done*/
int cd(char **cmd); /*Adi done*/
int exiting(char **cmd); /*Adi done*/
int print_jobs();/*Harsh done*/
int store_history(char **cmd, pid_t pid, double difference);/*Adi done*/
int print_history();/*Adi done*/

#endif
