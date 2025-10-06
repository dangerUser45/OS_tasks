#pragma once

#include <stddef.h>
#include <sys/types.h>
#include <sys/ipc.h>

struct msqid_ds;

int safe_open(const char* file, int oflag, int mode);
int safe_close(int fd, const char* filename);
int fd_write(int fd_src, int fd_dest, const char *filename_src,
             const char* filename_dest, char* buf);
ssize_t safe_write(int fd, const char* filename, char* buf, ssize_t n);

pid_t safe_fork(void);
int safe_pipe(int pipedes[2]);
int safe_dup2 (int fd, int fd2);

int safe_msgget(key_t key, int flags);
int safe_msgsnd(int queue_id, const void* msg_buf, size_t msg_size,
                int msg_flags);
ssize_t safe_msgrcv(int queue_id, void* msg_buf, size_t msg_size,
                    long msg_type, int msg_flags);
int safe_msgctl(int queue_id, int cmd, struct msqid_ds* buf);
