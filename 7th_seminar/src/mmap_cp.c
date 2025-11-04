#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#include "safe_lib.h"

//--------------------------------------------------------------
int main(int argc, char** argv) {
  check_args(argc, 3);
  
  int fd_src =  safe_open(argv[1], O_RDONLY, 0);
  int fd_dest = safe_open(argv[2], O_CREAT | O_RDWR, 0666);

  struct stat st = {};
  fstat(fd_src, &st);
  size_t size_file_src = st.st_size;

  ftruncate(fd_dest, size_file_src);

  char* mem_src = (char*)mmap(NULL, size_file_src, PROT_READ,
                                MAP_SHARED, fd_src, 0);

  char* mem_dest = (char*)mmap(NULL, size_file_src, PROT_WRITE,
                              MAP_SHARED, fd_dest, 0);
  
  memcpy(mem_dest, mem_src, size_file_src);

  munmap(mem_src, size_file_src);
  munmap(mem_dest, size_file_src);

  fsync(fd_dest);
  safe_close(fd_src, "source file");
  safe_close(fd_dest, "destination file");
}
//--------------------------------------------------------------
