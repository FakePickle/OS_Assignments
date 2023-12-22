# OS_Assignment_1
# Group 64 - Harsh PriteshKumar Mistry and Aditya Jagadale

SimpleLoader Implementation:-
  Step 1:-
    First we allocated memory to the ehdr variable as it was a pointer. Then we read the ehdr data from the file input and put it in the ehdr variable to which we allocated memory.
    Then we seek towards the program header offset for the step 2 to be implemented.
  Step 2:-
    We allocate memory to the program header table and read it from the file input with size of each program header table structure * the number of program header table entry.
    Then we go in a for loop to find at which index does the entry point of phdr(Program header table) is not NULL and also the p_type is PT_LOAD.
  Step 3:-
    Then we create a void pointer to allocate memory to p_memsz using mmap and then seek towards the entry point of the table as soon as we find p_type in the for loop.
  Step 4:-
    Then we type casted _start() to int pointer from void
  Error Handling:-
    We added error handling at places where there can be certain edge cases while/before opening the file.
  

CONTRIBUTION:-
  Aditya:- Launcher Implementation make file based on the same.
  Harsh:- Loader Implementation and the make file regarding the same and also the main make file to compile all of the codes.