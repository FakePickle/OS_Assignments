# OS Assignment 5

## Simple Multithreader

This C++ header file provides a simple way to execute for loops in parallel using multiple threads. It provides two main functions: `parallel_for` and `parallel_for_2d`.

#### `parallel_for`

This function executes a for loop in parallel. It takes four arguments:

- `low` and `high`: These define the range of the for loop.
- `lambda`: This is a function that takes an integer argument. It is the body of the for loop.
- `numThreads`: This is the number of threads to use.

The function divides the range of the for loop among the threads. Each thread executes the lambda function on its portion of the range.

#### `parallel_for_2d`

This function executes a 2D for loop in parallel. It takes six arguments:

- `low1`, `high1`, `low2`, and `high2`: These define the ranges of the two dimensions of the for loop.
- `lambda`: This is a function that takes two integer arguments. It is the body of the for loop.
- `numThreads`: This is the number of threads to use.

The function divides the ranges of the for loop among the threads. Each thread executes the lambda function on its portion of the ranges.

### ThreadData and ThreadData2D

These are structs that store the information needed to execute the for loops in the threads. They store the start and end indices for the for loop, the lambda function, and the start and end times for the execution of the for loop.

### Mutex

A mutex is used to ensure that the threads do not interfere with each other when executing the lambda function.

### Error Handling

The functions check for errors in initializing the mutex and allocating memory for the threads and the ThreadData structs. If an error occurs, an error message is printed and the program exits.

### Timing

The start and end times for the execution of the for loop in each thread are stored in the ThreadData structs. After all threads have finished, the time taken by each thread is printed.

### How to Run the code

#### Open any terminal of your choice:
* make
* ./matrix
* ./vector

### Github Repo

https://github.com/FakePickle/OS_Assignment_5

### Contributions
- Aditya:- First Parallel For Loop
- Harsh:- Second Parallel_For loop
