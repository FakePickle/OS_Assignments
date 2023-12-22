# OS_Assignment_3

## In this assignment we have made a simple scheduler for OS.

### Scheduler Readme

#### Assumption :- We are giving a signal of our own to run the scheduler. It is still a daemon and you will be able to run simpleshell commands while the scheduler is running. The signal to run the scheduler is `sched`.

#### The code above is a process scheduler written in C. The program takes two command line arguments, `NCPU` and `TSLICE`, which specify the number of CPUs and the time slice for each process, respectively. The program uses semaphores to synchronize access to shared resources, such as the ready queue and the CPU.

#### The `enqueue` function adds a process to the ready queue. If the queue is full, the function prints an error message and returns. Otherwise, the function adds the process to the rear of the queue and updates the queue size and rear index.

#### The `dequeue` function removes a process from the ready queue. If the queue is empty, the function prints an error message and returns a dummy process with PID -1. Otherwise, the function removes the process from the front of the queue and updates the queue size and front index.

#### The `is_empty` function checks if the ready queue is empty. It returns 1 if the queue is empty and 0 otherwise.

#### The `scheduler` function is the main loop of the program. It runs until the ready queue is empty. The function waits for a signal from the `submit` function that a new process has been added to the queue. It then checks if there are any available CPUs and assigns them to processes in the queue. The function then waits for the time slice to elapse and checks if any processes have finished executing. If a process has finished executing, the function calculates its execution time, wait time, and burst time and prints them to the console.

#### The `submit` function creates a new process using the `fork` system call. If the `fork` call fails, the function prints an error message and returns. Otherwise, the function sends a `SIGSTOP` signal to the child process to pause it. The function then executes the command specified by the user using the `execvp` system call. If the `execvp` call fails, the function returns. Otherwise, the function adds the process to the ready queue and prints a message to the console.

#### To improve the code's readability, we added comments to explain the purpose of each function and variable. We used descriptive variable names to make the code easier to understand. To improve the code's performance.