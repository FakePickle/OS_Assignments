#include "bash.h"

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
        for (i = 0; i < ncpu; i++) {
            int status;
            pid_t pid = waitpid(-1, &status, WNOHANG);
            if (pid > 0) {
                int j;
                for (j = 0; j < ready_queue.size; j++) {
                    if (ready_queue.processes[(ready_queue.front + j) % MAX_PROCESSES].pid == pid) {
                        struct process p = dequeue(&ready_queue);
                        p.end_time = time(NULL);
                        p.burst_time = p.end_time - p.start_time;
                        p.wait_time = p.start_time - p.arrival_time - p.burst_time;
                        printf("Process %d: pid=%d, execution time=%ld, wait time=%ld\n", j+1, p.pid, p.burst_time, p.wait_time);
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
    } else if (pid < 0) {
        printf("Error in creating process\n");
        return;
    } else {
        struct process p = {pid, time(NULL), 0, 0, 0, 0};
        // sem_wait(empty);
        // sem_wait(mutex);
        enqueue(&ready_queue, p);
        // sem_post(mutex);
        // sem_post(full);
        printf("Process %d added to queue\n", pid);
    }
}

// Main function
int dummy_main(int argc, char **argv)
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
    sem_wait(empty);
    sem_post(full);
    while(1)
    {
        scheduler();
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