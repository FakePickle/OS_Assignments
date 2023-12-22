# Custom ELF Loader

This C program is a custom ELF (Executable and Linkable Format) loader. ELF is a common standard file format for executables, object code, shared libraries, and core dumps. The loader reads an ELF file, maps the segments into memory, and then executes the program.

## Global Variables

The program begins by defining global variables, including pointers to the ELF header (`ehdr`) and program header (`phdr`), a file descriptor (`fd`), and various counters and sizes.

## Cleanup Function

The `loader_cleanup` function is used to free allocated memory and close the file when the loader is done.

## Finding Segments

The `find_segment` function is used to find the memory segment that contains a given address. It iterates over all the segments in the program header table and returns the one that contains the fault address.

## Segmentation Fault Handler

The `segfault_handler` function is a signal handler for segmentation faults. When a segmentation fault occurs, this function is called with the address that caused the fault. It finds the segment containing the fault address, allocates a page of memory at the correct address using `mmap`, and then reads the segment from the file into the allocated memory. It also updates various counters and prints diagnostic information.

## Main Function

The `load_and_run_elf` function is the main function of the loader. It opens the ELF file, reads the ELF header and program header table, and sets up the segmentation fault handler. It then iterates over the program header table to find the segment that contains the entry point of the program. Once it finds the correct segment, it calculates the address of the entry point function (`_start`), calls it, and prints the return value. Finally, it prints the total number of page faults, page allocations, and the total internal fragmentation.

## Lazy Loading

The loader uses lazy loading to load segments into memory. Instead of loading all segments when the program starts, it only loads a segment when a segmentation fault occurs because the program tried to access an address in the segment. This can save memory if the program doesn't access all segments, but it requires handling segmentation faults, which is a complex task.

### CONTRIBUTIONS
* Aditya:- find_segment, segfault_handler
* Harsh:- segfault_handler, load_and_run_elf

#### GITHUB REPOSITORY
- https://github.com/Jaagss/OS_Assignment_4