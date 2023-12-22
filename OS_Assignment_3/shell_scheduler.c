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


struct history command_history[MAX_HISTORY_SIZE];
static int counter = 0;
int is_background = 0;
int num_background_processes = 0;
clock_t start,end;
struct BackgroundProcess running_processes[MAX_BACKGROUND_PROCESSES];
struct process completed_process[MAX_PROCESSES];


/*Assignment 2 part starts */
int store_history(char **cmd, pid_t pid, double difference) {
    struct history *entry = &command_history[counter];
    counter++;
    entry->sno = counter;
    entry->cmd = (char **)malloc((MAX_TOKENS + 1) * sizeof(char *));
    
    if (entry->cmd == NULL) {
        fprintf(stderr, "Memory allocation error for history command.\n");
        exit(FAILED);
    }

    int i = 0;
    while (cmd[i] != NULL) {
        entry->cmd[i] = strdup(cmd[i]);
        if (entry->cmd[i] == NULL) {
            fprintf(stderr, "Memory allocation error for history command part.\n");
            exit(FAILED);
        }
        i++;
    }
    
    entry->cmd[i] = NULL;
    entry->pid = pid;
    entry->execution_time = difference;
    entry->time = time(NULL);

    return 0;
}


char **slice_command(char **cmd, int start, int end) {
    if (cmd == NULL || cmd[start] == NULL || start > end) {
        return NULL;
    }

    int num_tokens = end - start + 1;
    char **sliced_cmd = (char **)malloc((num_tokens + 1) * sizeof(char *)); // +1 for the NULL terminator

    if (sliced_cmd == NULL) {
        fprintf(stderr, "Memory allocation error for sliced command.\n");
        exit(FAILED);
    }

    int i, j = 0;
    for (i = start; i <= end; i++) {
        sliced_cmd[j] = strdup(cmd[i]);
        if (sliced_cmd[j] == NULL) {
            fprintf(stderr, "Memory allocation error for sliced command part.\n");
            exit(FAILED);
        }
        j++;
    }

    sliced_cmd[j] = NULL; // Null-terminate the array

    return sliced_cmd;
}

void execute_piped_commands(char **cmd) {
    int cmd_count = 0;
    int pipe_count = 0;
    int i = 0;

    // Count the number of pipes to determine the number of commands
    while (cmd[i] != NULL) {
        if (strcmp(cmd[i], "|") == 0) {
            pipe_count++;
        } else if (strcmp(cmd[i], "&") == 0) {
            is_background = 1; // Set the background flag
            cmd[i] = NULL;
        }
        cmd_count++;
        i++;
    }

    if (pipe_count == 0) {
        // No pipes, execute a single command
        exec_cmd(cmd);
        return;
    }

    // Allocate memory for an array of command arrays
    char ***commands = (char ***)malloc((pipe_count + 1) * sizeof(char **));

    if (commands == NULL) {
        fprintf(stderr, "Memory allocation error for commands.\n");
        exit(FAILED);
    }

    int cmd_start = 0;
    int cmd_end = 0;
    i = 0;

    // Split the input command into multiple commands
    while (i < cmd_count) {
        if (strcmp(cmd[i], "|") == 0) {
            commands[cmd_end] = slice_command(cmd, cmd_start, i - 1);
            cmd_start = i + 1;
            cmd_end++;
        }
        i++;
    }

    // Handle the last command
    commands[cmd_end] = slice_command(cmd, cmd_start, i - 1);

    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    // Execute the commands in a pipeline
    for (i = 0; i <= pipe_count; i++) {
        int pipefd[2];

        if (i < pipe_count) {
            // Create a pipe for commands except the last one
            if (pipe(pipefd) == -1) {
                perror("Pipe creation failed");
                exit(FAILED);
            }
            output_fd = pipefd[1]; // Set output to the write end of the pipe
        } else {
            // Last command, no need to create a pipe
            output_fd = STDOUT_FILENO;
        }

        // Execute the current command
        new_process(commands[i], input_fd, output_fd);

        // Close input_fd if it's not the initial STDIN_FILENO
        if (input_fd != STDIN_FILENO) {
            close(input_fd);
        }

        // Close output_fd if it's not the last command's STDOUT_FILENO
        if (output_fd != STDOUT_FILENO) {
            close(output_fd);
        }

        // Set the input_fd for the next command to be the read end of the pipe
        input_fd = pipefd[0];
    }

    // Wait for all child processes to complete
    int status;
    for (i = 0; i <= pipe_count; i++) {
        wait(&status);
    }

    if (!is_background) {
        int status;
        for (i = 0; i <= pipe_count; i++) {
            wait(&status);
        }
    }

    // Free memory allocated for command arrays
    for (i = 0; i <= pipe_count; i++) {
        free(commands[i]);
    }
    free(commands);
}

int exec_cmd(char **cmd) {
    char *funclist[] = {
        "cd",
        "exit",
        "help",
        "jobs",
        "history"
    };

    int (*func_list[])(char **) = {
        &cd,
        &exiting,
        &help,
        &print_jobs,
        &print_history
    };

    if (cmd[0] == NULL) {
        return -1;
    }

    for (long unsigned int i = 0; i < sizeof(funclist) / sizeof(char *); i++) {
        if (strcmp(cmd[0], funclist[i]) == 0) {
            // printf("%s\n",cmd[0]);
            return ((*func_list[i])(cmd));
        }
    }

    return new_process(cmd, STDIN_FILENO, STDOUT_FILENO);
}


int new_process(char **cmd, int input_fd, int output_fd) {
    pid_t process_id;
    int stat;
    start = clock();

    process_id = fork();

    if (process_id == 0) {
        
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO){
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        // Handle output redirection when '>' is detected
        int output_redirect = 0;
        for (int i = 0; cmd[i] != NULL; i++) {
            if (strcmp(cmd[i], ">") == 0) {
                output_redirect = 1;
                cmd[i] = NULL; // Remove the '>' symbol
                i++; // Move to the next argument which should be the output file
                if (cmd[i] != NULL) {
                    int fd = open(cmd[i], O_WRONLY | O_TRUNC);
                    if (fd == -1) {
                        perror("open");
                        exit(FAILED); 
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                } else {
                    fprintf(stderr, "Missing output file after '>'\n");
                    exit(FAILED);
                }
            }
        }

        if (output_redirect) {
            // If output redirection is used, execute the command and exit
            if (execvp(cmd[0], cmd) == -1) {
                fprintf(stderr, "Error in child process.\n");
                exit(FAILED);
            }
        } else {
            // If no output redirection, check for background processing
            if (is_background) {
                // If it's a background process, redirect output to /dev/null
                int null_fd = open("/home/fakepickle/OS_Assignment_2/background_process.txt", O_WRONLY | O_TRUNC);
                if (null_fd == -1) {
                    perror("open");
                    exit(FAILED);
                }
                dup2(null_fd, STDOUT_FILENO);
                close(null_fd);
            }

            // Execute the command as a foreground or background process
            if (execvp(cmd[0], cmd) == -1) {
                fprintf(stderr, "Error in child process.\n");
                exit(FAILED);
            }
        }
    } else if (process_id < 0) {
        fprintf(stderr, "Error while forking.\n");
    } else {
        // Parent process
        if (!is_background) {
            // Wait for the child process to complete if not in the background
            waitpid(process_id, &stat, 0);
            end = clock();
            double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
            store_history(cmd, process_id, elapsed_time);
        } else {
            // In the background, fork again to detach from the terminal
            pid_t bg_process_id = fork();
            if (bg_process_id == 0) {
                // Child of the parent process (detached background process)
                // This child will immediately exit, leaving the background process running
                exit(0);
            } else if (bg_process_id < 0) {
                fprintf(stderr, "Error while forking background process.\n");
            } else {
                // Parent of the detached background process
                // Add the process to the list
                if (num_background_processes < MAX_BACKGROUND_PROCESSES) {
                    running_processes[num_background_processes].pid = process_id;
                    strncpy(running_processes[num_background_processes].cmd, cmd[0], sizeof(running_processes[num_background_processes].cmd));
                    end = clock();
                    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
                    store_history(cmd, bg_process_id, elapsed_time);
                    num_background_processes++;
                } else {
                    fprintf(stderr, "Maximum number of background processes reached.\n");
                }
                is_background = 0;
            }
        }
    }

    return -1;
}

int help(char **cmd){
    char *funclist[] = {
        "cd",
        "exit",
        "help",
        "jobs",
        "history"
    };

    (void)(**cmd);

    printf("This is a simple shell created by Harsh and Aditya.\n");
    printf("The commands that are implemented are:- \n");
    for (int i = 0; i < 3; i++){
        printf("\t-> %s\n", funclist[i]);
    }
    printf("Reason for the implementation of these commands are available in the documentation.\n");
    end = clock();
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    store_history(cmd, 0, elapsed_time);
    return 0;
}

int exiting(char **cmd){
    if (cmd[1]) return (atoi(cmd[1]));
    else{
        if (completed_process[0].pid == -1){
            printf("\nNo Commands Executed\n");
            exit(SUCCESS);
        } else {
            printf("Processes run in scheduler\n");
            for (int i = 0; i < MAX_PROCESSES; i++){
                if (completed_process[i].pid == 0) break;
                printf("Process %d: pid=%d, execution time=%ld, wait time=%ld\n", i+1, completed_process[i].pid, completed_process[i].burst_time, completed_process[i].wait_time);
            }
        }
        if (command_history[0].cmd == NULL){
            printf("\nNo Commands Executed\n");
            exit(SUCCESS);
        }
        printf("\n\nHistory:\n");
        for (int i = 0; i < counter; i++){
            printf("[%d]\nPID: %d\n", command_history[i].sno, command_history[i].pid);
            printf("Command Executed: ");
            int j = 0;
            while (command_history[i].cmd[j] != NULL){
                printf("%s ", command_history[i].cmd[j]);
                j++;
            }
            printf("\nTimeStamp at which command was executed: %sExecution Time: %f\n", ctime(&command_history[i].time) + 11, command_history[i].execution_time);
            printf("\n");
        }
        exit(0);
    }
    return 0;
}

int cd(char **cmd){
    if(cmd[1] != NULL){
        if(chdir(cmd[1]) != 0) {printf("error in cd.c: changing dir\n"); return 1;}
        end = clock();
        double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
        store_history(cmd, 0, elapsed_time);
    }else fprintf(stderr,"expected argument to \"cd\"\n");
    return (-1);
}

int print_jobs() {
    if (num_background_processes == 0){
        printf("NO CURRENT PROCESS RUNNING IN THE BACKGROUND\n");
    } else {
        printf("Background processes:\n");
        for (int i = 0; i < num_background_processes; i++) {
            printf("[%d] %s (PID: %d)\n", i + 1, running_processes[i].cmd, running_processes[i].pid);
        }
    }
    return 0;
}

int print_history(){
    printf("History:\n");
    for (int i = 0; i < counter; i++){
        printf("[%d]\nPID: %d\n", command_history[i].sno, command_history[i].pid);
        printf("Command Executed: ");
        int j = 0;
        while (command_history[i].cmd[j] != NULL){
            printf("%s ", command_history[i].cmd[j]);
            j++;
        }
        printf("\n");
    }
    return 0;
}

char *read_line(void) {
    char *line = NULL;
    size_t buffersize = 0;

    ssize_t read = getline(&line, &buffersize, stdin);

    if (read == -1) {
        if (feof(stdin)) {
            free(line);
            return NULL;
        } else {
            free(line);
            perror("Line not read properly.\n");
            exit(FAILED);
        }
    }
    return line;
}

char **parse(char *line) {
    int buffersize = 64;
    int token_counter = 0;
    char **tokens = malloc(buffersize * sizeof(char *));

    if (tokens == NULL) {
        fprintf(stderr, "Memory allocation error for tokens.\n");
        exit(FAILED);
    }

    if (line == NULL) {
        fprintf(stderr, "Input line is NULL.\n");
        exit(FAILED);
    }

    char *line_copy = strdup(line);

    if (line_copy == NULL) {
        fprintf(stderr, "Memory allocation error for line_copy.\n");
        exit(FAILED);
    }

    char *token = strtok(line_copy, DELIMITER); // Tokenize by space, tab, and newline

    while (token != NULL) {
        if (strcmp(token, "#") == 0) {
            break;
        }

        if (strcmp(token, "|") == 0) {
            tokens[token_counter] = strdup(token);
            token_counter++;
        } else {
            tokens[token_counter] = strdup(token);
            token_counter++;
        }

        if (token_counter >= buffersize) {
            buffersize *= 2;
            tokens = realloc(tokens, buffersize * sizeof(char *));

            if (tokens == NULL) {
                fprintf(stderr, "Memory reallocation error.\n");
                exit(FAILED);
            }
        }

        token = strtok(NULL, " \t\n");
    }

    tokens[token_counter] = NULL;

    // for (int i = 0; i < token_counter; ++i){
    //     printf("Token :- %s\n", tokens[i]);
    // }

    free(line_copy);
    return tokens;
}

void sigquit_handler(int sig){
    if (sig == SIGQUIT){
        printf("SIGQUIT signal received\n");
        exit(SUCCESS);
    }
    printf("\n\nHistory:\n");
    for (int i = 0; i < counter; i++){
        printf("[%d]\nPID: %d\n", command_history[i].sno, command_history[i].pid);
        printf("Command Executed: ");
        int j = 0;
        while (command_history[i].cmd[j] != NULL){
            printf("%s ", command_history[i].cmd[j]);
            j++;
        }
        printf("\nTimeStamp at which command was executed: %sExecution Time: %f\n", ctime(&command_history[i].time) + 11, command_history[i].execution_time);
        printf("\n");
    }
}

void sigint_handler(int sig) {
    if (sig == SIGINT) {
        if (command_history[0].cmd == NULL){
            printf("\nNo Commands Executed\n");
            exit(SUCCESS);
        }
        if (completed_process[0].pid == -1){
            printf("\nNo Commands Executed\n");
            exit(SUCCESS);
        } else {
            printf("Processes run in scheduler\n");
            for (int i = 0; i < MAX_PROCESSES; i++){
                if (completed_process[i].pid == 0) break;
                printf("Process %d: pid=%d, execution time=%ld, wait time=%ld\n", i+1, completed_process[i].pid, completed_process[i].burst_time, completed_process[i].wait_time);
            }
        }
        printf("\n\nHistory:\n");
        for (int i = 0; i < counter; i++){
            printf("[%d]\nPID: %d\n", command_history[i].sno, command_history[i].pid);
            printf("Command Executed: ");
            int j = 0;
            while (command_history[i].cmd[j] != NULL){
                printf("%s ", command_history[i].cmd[j]);
                j++;
            }
            printf("\nTimeStamp at which command was executed: %sExecution Time: %f\n", ctime(&command_history[i].time) + 11, command_history[i].execution_time);
            printf("\n");
        }
    }
    exit(SUCCESS);
}
/*Assignment 2 part ends */

// #include "bash.h"

// Function to add a process to the queue
void enqueue(struct queue *q, struct process p) {
    if (q->size == MAX_PROCESSES) {
        printf("Queue is full\n");
        return;
    }
    q->rear = (q->rear + 1) % MAX_PROCESSES;
    q->processes[q->rear] = p;
    q->size++;
}

// Function to remove a process from the queue
struct process dequeue(struct queue *q) {
    if (q->size == 0) {
        printf("Queue is empty\n");
        struct process dummy = {-1, 0, 0, 0, 0, 0};
        return dummy;
    }
    struct process p = q->processes[q->front];
    q->front = (q->front + 1) % MAX_PROCESSES;
    q->size--;
    return p;
}

// Function to check if the queue is empty
int is_empty(struct queue *q) {
    return q->size == 0;
}


sem_t *mutex;
sem_t *full;
sem_t *empty;

int ncpu;
int tslice;
struct queue ready_queue;

// Function to schedule the processes in the queue
void scheduler() {
    // printf("Scheduler function\n");
    while (!is_empty(&ready_queue)) {
        printf("In while loop\n");
        sem_wait(full);
        sem_wait(mutex);
        int i;
        for (i = 0; i < ncpu && !is_empty(&ready_queue); i++) {
            struct process p = dequeue(&ready_queue);
            p.start_time = time(NULL);
            kill(p.pid, SIGCONT);
            printf("Process %d started\n", p.pid);
        }
        sem_post(mutex);
        usleep(tslice * 1000);
        sem_wait(mutex);
        for (i = 0; i < ncpu; i++)
        {
            int status;
            pid_t pid;
            while(pid = waitpid(-1, &status, WNOHANG) > 0)
            {
                int j;
                for (j = 0; j < ready_queue.size; j++)
                {
                    if (ready_queue.processes[j].pid == pid)
                    {
                        struct process p = dequeue(&ready_queue);
                        p.end_time = time(NULL);
                        p.burst_time = p.end_time - p.start_time;
                        p.wait_time = p.start_time - p.arrival_time - p.burst_time;
                        completed_process[j] = p;
                        // printf("Process %d: pid=%d, execution time=%ld, wait time=%ld\n", j+1, p.pid, p.burst_time, p.wait_time);
                        break;
                    }
                }
            }
        }
        sem_post(full);
        sem_post(mutex);
    }
}

// Function to add a process to the queue
void submit(char **cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        kill(getpid(), SIGSTOP);
        execvp(cmd[1], cmd+1);
        return;
    }
    else if (pid < 0)
    {
        printf("Error in creating process\n");
        return;
    }
    else
    {
        struct process p = {pid, time(NULL), 0, 0, 0, 0};
        // sem_wait(empty);
        // sem_wait(mutex);
        printf("enqueing\n");
        enqueue(&ready_queue, p);
        printf("enqueued\n");
        // sem_post(mutex);
        // sem_post(full);
        printf("Process %d added to queue\n", pid);
    }
}


int main(int argc, char **argv)
{
    
    if (argc != 3)
    {
        printf("Usage: %s <NCPU> <TSLICE>\n", argv[0]);
        return 1;
    }
    ncpu = atoi(argv[1]);
    tslice = atoi(argv[2]);
    if (ncpu <= 0 || tslice <= 0)
    {
        printf("Invalid input\n");
        return 1;
    }
    mutex = sem_open("mutex", O_CREAT, 0666, 1);
    full = sem_open("full", O_CREAT, 0666, 0);
    empty = sem_open("empty", O_CREAT, 0666, MAX_PROCESSES);
    if (mutex == SEM_FAILED || full == SEM_FAILED || empty == SEM_FAILED)
    {
        perror("sem_open");
        return 1;
    }
    // printf("Scheduler function\n");
    // sem_wait(empty);
    // sem_post(full);

    int status = -1;
    char *line;
    // int compile_time = 5;
    // int *waiting_time = &compile_time;
    char cwd[1024];
    pid_t sched_pid = fork();
    if (sched_pid == 0)
    {
        // This is the child process
        // Fork again to create a grandchild process
        pid_t gchild_pid = fork();
        if (gchild_pid == 0)
        {
            // This is the grandchild process
            while(1)
            {
                scheduler();
            }
            exit(0); // Terminate the grandchild process
        }
        else if (gchild_pid < 0)
        {
            fprintf(stderr, "Error in forking grandchild process\n");
        }
        else
        {
            // The child process (scheduler) doesn't need to wait for the grandchild
            exit(0);
        }
    } else if (sched_pid < 0) {
        fprintf(stderr, "Error in forking scheduler process\n");
    } else{

        while (status == -1) {
            // printf("Entering loop\n");
            // wait(waiting_time);  
            printf("Shell@IIITD:%s$ ", getcwd(cwd, sizeof(cwd)));
            if (signal(SIGINT, sigint_handler) == SIG_ERR){
                fprintf(stderr, "Error while getting signal\n");
                exit(FAILED);
            }
            line = read_line();
            char **cmd = parse(line);
            // printf("%s\n", cmd[0]);
            if (strcmp(cmd[0], "submit") == 0)
            {
                // printf("HERE\n");
                // sharedmem(cmd);
                printf("submitting\n");
                submit(cmd);
                printf("submitted\n");
            }
            // else if (strcmp(cmd[0], "sched") == 0){
            //     // printf("HERE\n");
            // }
            else if (cmd != NULL) {
                // printf("sj\n");
                execute_piped_commands(cmd);
                free(cmd);
            }
            // printf("Out of if statements\n");
            if (line != NULL) {free(line);}
            // if (status >= 0) exit(status);
        }
        // Wait for the scheduler to finishclear
        int sched_status;
        waitpid(sched_pid, &sched_status, 0);
            if (WIFEXITED(sched_status)) {
            printf("Scheduler process has completed with exit status: %d\n", WEXITSTATUS(sched_status));
        } else {
            printf("Scheduler process terminated abnormally\n");
        }
    }
    printf("Ended\n");
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_unlink("mutex");
    sem_unlink("full");
    sem_unlink("empty");
    return 0;
}


