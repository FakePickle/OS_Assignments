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
#include <sched.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/ptrace.h>


//ASSIGNMENT 2 PART

#define DELIMITER " \t\r\n\a\""

#define SUCCESS 0
#define FAILED 1
#define MAX_BACKGROUND_PROCESSES 100
#define MAX_TOKENS 64
#define TIME_QUANTUM 4
#define MAX_HISTORY_SIZE 1024


/*Shell Interaction ie Terminal*/
struct history{
    int sno;
    pid_t pid;
    char **cmd;
    time_t time;
    double execution_time;
};


/*Bonus part for background processes*/
struct BackgroundProcess{
    int sno;
    pid_t pid;
    char cmd[MAX_TOKENS];
};

/*Required by shell_interactive*/
char *read_line(void); 
char **parse(char *line);
int exec_cmd(char **cmd);
void execute_piped_commands(char **cmd);
/*Required for execute_cmds*/
int new_process(char **cmd, int input_fd, int output_fd);
char** slice_command(char **cmd, int start, int end);
int store_history(char **cmd, pid_t pid, double difference);
int cd(char **cmd);

/*Commands implemented on our own(NOT MENTIONED IN THE ASSIGNMENT)*/
int help(char **cmd); 
int exiting(char **cmd);
int print_jobs();
int print_history();
int main(int argc, char **argv);

//ASSIGNMENT 3 PART

/*Storing processes in ready state*/
#define MAX_PROCESSES 100
#define MAX_COMMAND_LENGTH 100

struct process {
    pid_t pid;
    time_t arrival_time;
    time_t start_time;
    time_t end_time;
    time_t wait_time;
    time_t burst_time;
};

struct queue {
    struct process processes[MAX_PROCESSES];
    int front;
    int rear;
    int size;
};

// struct sigaction{
//     void (*sa_handler)(int);
//     void (*sa_sigaction)(int, siginfo_t *, void *);
//     sigset_t sa_mask;
//     int sa_flags;
//     void (*sa_restorer)(void);
// };


int is_empty(struct queue *q);
void enqueue(struct queue *q, struct process p);
struct process dequeue(struct queue *q);
void scheduler();
void submit(char **cmd);
int dummy_main(int argc, char **argv);

#endif