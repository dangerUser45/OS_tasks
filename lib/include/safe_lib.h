
#pragma once

#include <mqueue.h>     // for mqd_t
#include <stdbool.h>    // for bool
#include <stddef.h>     // for size_t
#include <sys/types.h>  // for ssize_t, key_t, pid_t

struct msqid_ds;
struct sembuf;

bool check_args(int argc, int neccesary_argc);
bool vcheck_args(int argc, int neccesary_argc);

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

mqd_t safe_mq_open (const char* name, int oflag, ...);
int safe_mq_send(mqd_t mqdes, const char *msg_ptr,
                 size_t msg_len, unsigned msg_prio);
ssize_t safe_mq_receive(mqd_t mqdes, char *msg_ptr,
                        size_t msg_len, unsigned *msg_prio);
int safe_mq_close(mqd_t queue_id);
int safe_mq_unlink(const char* name);

int safe_semget (key_t key, int num_semaphors, int semflg);
int safe_semctl (int semid, int semnum, int cmd, ...);
int safe_semop(int semid, struct sembuf *operations_array,
               unsigned number_operations);
