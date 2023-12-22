#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  free(ehdr);
  free(phdr);
  close(fd);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char *exe) {
  fd = open(exe, O_RDONLY);
  //Checking if the file has opened properly without any errors
  //Checking if the file has opened properly without any errors
  if (fd == -1) {perror("File not opened properly\n"); exit(1);}
  
  phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
  if (read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum) != ehdr->e_phentsize * ehdr->e_phnum) {perror("File not read properly\n"); exit(1);} //Error Handling


  void *virtual_mem;
  int index;
  //Finding the program header table with p_type as PT_LOAD and then assigning the virtual memory to the program and seeking towards the entry point and calling it
  for (int i = 0; i < ehdr->e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < (phdr[i].p_vaddr + phdr[i].p_memsz)) {
      virtual_mem = mmap(NULL, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
      if (virtual_mem == MAP_FAILED) {perror("MMAP FAILED!\n"); exit(1);}
      lseek(fd, phdr[i].p_offset, SEEK_SET);
      if (read(fd, virtual_mem, phdr[i].p_memsz) != phdr[i].p_memsz) {perror("File not Read properly\n"); exit(1);} //Error Handling
      index = i;
      break;
    }
  }


  // printf("hello world\n");

  int pointer = (int) virtual_mem + (ehdr->e_entry - phdr[index].p_vaddr);

  //Type casting the entry point to a function pointer and then calling it
  int (*_start)(void) = (int (*)(void)) pointer;
    // printf("hello world\n");

  int result = _start();
  printf("User _start return value = %d\n",result);
}
}
