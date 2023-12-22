#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
int page_faults = 0;
int page_allocations = 0;
int internal_fragmentations = 0;
size_t page_size;
Elf32_Phdr* prev_phdr;
void* allocated_memory;

//this is a linked list that points to the memory that has been allocated as it gets overwritten every time i allocate new memory to munmap it later
struct address_space {
  void* allocated_memory;
  struct address_space* next;
};

struct address_space* head = NULL;

//this function creates a node in the linked list
void* create_node(void *allocated)
{
  if (head == NULL)
  {
    head = (struct address_space*)malloc(sizeof(struct address_space));
    head->allocated_memory = allocated;
    head->next = NULL;
  }
  else
  {
    struct address_space* temp = head;
    while(temp->next != NULL)
    {
      temp = temp->next;
    }
    temp->next = (struct address_space*)malloc(sizeof(struct address_space));
    temp->next->allocated_memory = allocated;
    temp->next->next = NULL;
  }
}

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  free(ehdr);
  free(phdr);

  //this loop frees all the memory that has been allocated using munmap
  while (head != NULL)
  {
    struct address_space* temp = head;
    head = head->next;
    munmap(temp->allocated_memory, page_size);
    free(temp);
  }

  close(fd);
}

Elf32_Phdr* find_segment(void* fault_address)
{
  for(int i = 0; i<ehdr->e_phnum; i++)
  {
    Elf32_Phdr* segment = &phdr[i];
    Elf32_Addr start = segment->p_vaddr;
    Elf32_Addr end = start + segment->p_memsz;

    // printf("Address of start:- %p\nAddress of endpoint:- %p\nAddress of fault:- %p\n\n", (void*)start, (void*)end, fault_address);

    if(fault_address >= (void*)start && fault_address < (void*)end)
    {
      return segment;
    }
  }
  return NULL;
}

void segfault_handler(int signo, siginfo_t* info, void* ucontext) {
  // printf("in seg fault\n");
  void* fault_address = info->si_addr;

  if (fault_address == NULL)
  {
    printf("NO address given\n");
  }

  // printf("Fault address: %p\n", fault_address);
  
  Elf32_Phdr* segment = find_segment(fault_address);

  if (segment == NULL)
  {
    printf("Segment not found\n");
    exit(1);
  }

  size_t segment_size = (segment->p_memsz + page_size - 1) & ~(page_size - 1);
  void* page_start = (void*)((uintptr_t)fault_address & ~(page_size - 1));
  allocated_memory = mmap(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0, 0);

  create_node(allocated_memory);

  if(allocated_memory == MAP_FAILED)
  {
    perror("MMAP failed");
    exit(1);
  }

  // printf("Allocated memory address: %p\n", allocated_memory);

  if (allocated_memory == NULL)
  {
    printf("Memory allocation failed\n");
    exit(1);
  }

  // printf("Address of segment:- %p\n", (void*)segment->p_offset);

  lseek(fd, segment->p_offset, SEEK_SET);

  // Read the segment from the file into the allocated memory
  if(read(fd, allocated_memory, segment->p_filesz) != segment->p_filesz)
  {
    perror("File read failed");
    exit(1);
  }

  // printf("Memory copied successfully\n");
  page_faults ++;
  page_allocations ++;
  // printf("Memsz:- %d\nFilesz:- %d\n", segment->p_memsz, segment->p_filesz);
  // printf("Segment Size:- %d\n", segment_size);
  if (segment->p_memsz == segment->p_filesz)
  {
    internal_fragmentations += segment_size - segment->p_memsz;
  }
  if (prev_phdr != NULL && prev_phdr->p_memsz != segment->p_memsz)
  {
    internal_fragmentations += segment_size - (segment->p_memsz - segment->p_filesz);
  }
  if (prev_phdr != segment)
  {
    prev_phdr = segment;
  }
  // printf("Internal fragmentation: %d B\n", internal_fragmentations);
  // printf("Resuming program execution\n");
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char *exe)
{
  fd = open(exe, O_RDONLY);
  //Checking if the file has opened properly without any errors
  if (fd == -1) {perror("File not opened properly\n"); exit(1);}

  //Allocating memory to ehdr to read the input file
  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
  if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
  {
    perror("File not read properly\n");
    exit(1);
  } //Error handling if read not done properly

  //seeking to the program header offset
  lseek(fd, ehdr->e_phoff, SEEK_SET);

  //allocating memory to program header to find the PT_LOAD and the entrypoint
  phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
  if (read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum) != ehdr->e_phentsize * ehdr->e_phnum)
  {
    perror("File not read properly\n");
    exit(1);
  } //Error Handling

  page_size = getpagesize();

  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = segfault_handler;
  sigaction(SIGSEGV, &act, NULL);

  int (*_start)(void) = (int (*)(void)) ehdr->e_entry;
  int result = _start();
  printf("User _start return value = %d\n",result);

  printf("Total page faults: %d\n", page_faults);
  printf("Total page allocations: %d\n", page_allocations);
  printf("Total internal fragmentation: %f KB\nTotal internal fragmentation: %d B\n", (float)(internal_fragmentations)/1024, (internal_fragmentations));
}
