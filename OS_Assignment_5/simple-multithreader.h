#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <time.h>
#include <chrono>

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

/* This struct stores all
  * the information needed to execute a 
  * parallel for loop in a single thread
  * And also stores start and end times.
  */
typedef struct {
    int start;
    int end;
    std::function<void(int)> func;
} ThreadData;

void* thread_func(void* arg) {
  ThreadData* data = (ThreadData*)arg;
  for (int i = data->start; i < data->end; ++i) {
    data->func(i);
  }
  return NULL;
}

void parallel_for(int low, int high, std::function<void(int)> lambda, int numThreads) {
  int range = high - low;
  int iterations_per_thread = range / numThreads;

  pthread_t threads[numThreads];
  ThreadData threadData[numThreads];


  for (int i = 0; i < numThreads; ++i) {
    threadData[i].start = low + i * iterations_per_thread;
    threadData[i].end = (i == numThreads - 1) ? high : threadData[i].start + iterations_per_thread;
    threadData[i].func = lambda;

    pthread_create(&threads[i], NULL, thread_func, &threadData[i]);
  }

  auto start_time = std::chrono::steady_clock::now(); // Start measuring total execution time
  for (int i = 0; i < numThreads; ++i) {
    pthread_join(threads[i], NULL);
  }

  auto end_time = std::chrono::steady_clock::now(); // End measuring total execution time
  std::cout << "Total execution time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
            << " milliseconds" << std::endl;
}



/* This struct stores all
  * the information needed to execute a 2d
  * parallel for loop in a single thread
  * And also stores start and end times.
  */
typedef struct {
  int start1;
  int end1;
  int start2;
  int end2;
  std::function<void(int, int)> func;
} ThreadData2D;

void* thread_func_2d(void* arg) {
  ThreadData2D* data = (ThreadData2D*)arg;
  for (int i = data->start1; i < data->end1; ++i) {
    for (int j = data->start2; j < data->end2; ++j) {
      data->func(i, j);
    }
  }
  return NULL;
}

void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> lambda, int numThreads) {
  pthread_t threads[numThreads];
  ThreadData2D threadData[numThreads];

  int range1 = high1 - low1;
  int range2 = high2 - low2;
  int iterations_per_thread1 = range1 / numThreads;
  int iterations_per_thread2 = range2 / numThreads;


  for (int i = 0; i < numThreads; ++i) {
    threadData[i].start1 = low1 + i * iterations_per_thread1;
    threadData[i].end1 = (i == numThreads - 1) ? high1 : threadData[i].start1 + iterations_per_thread1;
    threadData[i].start2 = low2;
    threadData[i].end2 = high2;
    threadData[i].func = lambda;

    pthread_create(&threads[i], NULL, thread_func_2d, &threadData[i]);
  }

  auto start_time = std::chrono::steady_clock::now(); // Start measuring total execution time
  for (int i = 0; i < numThreads; ++i) {
    pthread_join(threads[i], NULL);
  }
  auto end_time = std::chrono::steady_clock::now(); // End measuring total execution time

  std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " milliseconds" << std::endl;
}



int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);

  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main


