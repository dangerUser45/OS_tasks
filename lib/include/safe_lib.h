#pragma once

#include <sys/types.h>

int safe_open(const char* file, int oflag, int mode);
int safe_close(int fd, const char* filename);
int fd_write(int fd_src, int fd_dest, const char *filename_src,
             const char* filename_dest, char* buf);
ssize_t safe_write(int fd, const char* filename, char* buf, ssize_t n);

pid_t safe_fork(void);
int safe_pipe(int pipedes[2]);
int safe_dup2 (int fd, int fd2);
