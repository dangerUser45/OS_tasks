#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/safe_lib.h"

#ifndef PAGE_SIZE
  #define PAGE_SIZE 4096
#endif

//--------------------------------------------------------------
int safe_open(const char* file, int oflag, int mode)
{
    int fd = 0;
    if(mode == 0)
        fd = open(file, oflag);
    else
        fd =  open(file, oflag, mode);
    
    if(fd < 0)
        fprintf(stderr, "%s: %s\n", file, strerror(errno));
    
    return fd;
}
//--------------------------------------------------------------
int safe_close(int fd, const char* filename)
{
    int error = close(fd);
    if(error < 0)
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));

    return error;
}
//--------------------------------------------------------------
int fd_write(int fd_src, int fd_dest, const char* filename_src,
             const char *filename_dest, char *buf)
{
    while(true)
        {
            ssize_t num_sym = read(fd_src, buf, PAGE_SIZE);
            if (num_sym < 0)
            {
                fprintf(stderr, "%s: %s\n", filename_src, strerror(errno));
                return num_sym;
            }
            else if(num_sym > 0)
            {
                int error = safe_write(fd_dest, filename_dest, buf, num_sym);
                if(error < 0)
                    return error;
                continue;
            }
            else break;
        }
    
    return 0;
}
//--------------------------------------------------------------
ssize_t safe_write(int fd, const char* filename, char* buf, ssize_t n)
{
    ssize_t num_sym = 0;

    while(true)
    {
        buf += num_sym; n -= num_sym;
        num_sym = write(fd, buf, n);
        if(num_sym < 0)
        {
            if(errno != EINTR)
            {
                fprintf(stderr, "%s: %s\n", filename, strerror(errno));
                return num_sym;
            }
            else
                continue;
        }

        else if(num_sym == n)
            return num_sym;
        else if(num_sym == 0)
            break;
        else
            continue;
    }

    return 0;
}
//--------------------------------------------------------------
pid_t safe_fork(void) {
  pid_t pid = fork();
  if(pid == -1) {
    perror("fork");
    exit(-1);
  }
  return pid;
}
//--------------------------------------------------------------
int safe_pipe(int pipedes[2]) {
  int code_error = pipe(pipedes);
  if(code_error == - 1) {
    perror("pipe");
    exit(-1);
  }
  return code_error;
}
//--------------------------------------------------------------
int safe_dup2 (int fd, int fd2) {
  int code_error = dup2(fd, fd2);
  if(code_error == -1) {
    perror("dup2");
    exit(-1);
  }
  return code_error;
}
//--------------------------------------------------------------
