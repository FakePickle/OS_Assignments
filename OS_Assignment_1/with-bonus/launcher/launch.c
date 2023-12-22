#include "../loader/loader.h"

/*
  * Checks for input ELF File
*/

//Checking ELF File
int check_file(Elf32_Ehdr *hdr) {
  if (!hdr) return 1;
  if (hdr->e_ident[EI_MAG0] != ELFMAG0) return 1;
  if (hdr->e_ident[EI_MAG1] != ELFMAG1) return 1;
  if (hdr->e_ident[EI_MAG2] != ELFMAG2) return 1;
  if (hdr->e_ident[EI_MAG3] != ELFMAG3) return 1;
  return 0;
}

//Checking if ELF Supported
int check_elf(char** exe) {

  //Opening the file and then reading the Elf header of the file
  int ptr = open(exe[1], O_RDONLY);
  if (ptr == -1) {
      perror("Error opening file");
      return 1;
  }
  Elf32_Ehdr hdr;
  uint64_t ptrdata = read(ptr, &hdr, sizeof(Elf32_Ehdr));
  
  //Error Handling
  if (ptrdata != sizeof(Elf32_Ehdr)) {
      perror("Error reading header");
      close(ptr);
      return 1;
  }

  // printf("Pointer at ELF Header: %p\n", &hdr);
  //More ERROR HANDLING
  if (check_file(&hdr) != 0) {
      perror("Invalid ELF header\n");
      close(ptr);
      return 1;
  }
  close(ptr);
  return 0;
}


int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // printf("Returned value after checking the file %d\n",check_elf(argv));
  if (check_elf(argv) == 0){
    // printf("Loading the File\n");
    load_and_run_elf(argv[1]);
    // printf("Loaded the File\n");
    // printf("Cleaning the File\n");
    loader_cleanup();
    // printf("Cleaned the File\n");
  } else {
    perror("Error Checking file\n");
    exit(1);
  }
  return 0;
}
