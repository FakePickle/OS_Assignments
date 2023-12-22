# OS_Assignment_2

This is a C program that appears to be the implementation of a basic Unix-like shell with some additional functionalities. Below is a brief overview of the program components and their functionalities:

### Program Structure
1. The program starts with the inclusion of the "bash.h" header file, which contains function declarations, constants, and data structure definitions.

2. It defines several global variables and data structures to maintain command history, background processes, and timing information.

### Function Definitions

#### 1. `store_history`
   - This function stores executed commands and related information in a history structure.
   
#### 2. `slice_command`
   - Slices a command into smaller segments based on specified start and end indices. Used for handling piped commands.

#### 3. `execute_piped_commands`
   - Executes piped commands by splitting the input command into multiple commands and creating pipes between them.

#### 4. `exec_cmd`
   - Executes various built-in shell commands such as `cd`, `exit`, `help`, `jobs`, and `history`, as well as external commands.
   - We have also implemented the `>` function which redirects the output to the specified file. The reason behind the implementation of this command was to make it easier for the background process to run and also makes it easier to store the outputs.
   - The reason behind implementation of `cd` was to change directories as it is not a built in function for our shell.
   - The reason behind implementation of `help` was for people to understand what each and every command does and also makes it easier for the new users for the shell to understand it better.
   - The reason behind implementation of `jobs` was to print out the background processes which are running currently.

#### 5. `new_process`
   - Forks a new process to execute a command, handles input and output redirection, and manages background processes.

#### 6. Command-Specific Functions
   - `help`: Displays information about implemented shell commands.
   - `cd`: Changes the current working directory.
   - `exiting`: Handles shell exit, displays command history, and exits the shell.
   - `print_jobs`: Prints information about background processes.
   - `print_history`: Prints the command history.

#### 7. `read_line` and `parse`
   - `read_line` reads a line of input from the user.
   - `parse` tokenizes the input line into individual command arguments.

#### 8. `sigint_handler`
   - A signal handler for SIGINT (Ctrl+C) that displays command history and exits the shell gracefully.

#### 9. `shell_interactive`
   - The main interactive shell loop that continuously reads and executes user commands.

#### 10. `main`
    - The program's entry point, which checks if it's running in an interactive shell and calls `shell_interactive` accordingly.

### Header File (bash.h)
- Defines constants, data structures, and function declarations used in the program.

### Usage
- The program provides a basic command-line interface where users can execute shell commands, including built-in ones like `cd`, `exit`, `help`, `jobs`, and `history`.
- Users can also execute external commands.
- Background process execution is supported by appending `&` to a command.
- The program maintains a command history and displays it on exit or when the user presses Ctrl+C.
- It also keeps track of background processes and provides a list of currently running background jobs.

### Contribution
- void shell_interactive(void);  /*Harsh done*/
- char *read_line(void); /* Adi done*/
- char **parse(char *line); /* Harsh done*/
- int exec_cmd(char **cmd); /* Harsh done*/
- void execute_piped_commands(char **cmd); /* Harsh done*/
- int new_process(char **cmd, int input_fd, int output_fd); /*Harsh done*/
- char** slice_command(char **cmd, int start, int end);/*Adi done*/
- int help(char **cmd); /*Harsh* done*/
- int cd(char **cmd); /*Adi done*/
- int exiting(char **cmd); /*Adi done*/
- int print_jobs();/*Harsh done*/
- int store_history(char **cmd, pid_t pid, double difference);/*Adi done*/
- int print_history();/*Adi done*/

### Github Repository Link
- https://github.com/jaagss/OS_Assignment_2.git
