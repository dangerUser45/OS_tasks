#pragma once

#include <dirent.h>    // for DIR, struct dirent
#include <mqueue.h>    // for mqd_t
#include <semaphore.h> // for sem_t
#include <stdbool.h>   // for bool
#include <stddef.h>    // for size_t
#include <sys/types.h> // for ssize_t, key_t, pid_t

struct msqid_ds;
struct sembuf;

// For checking number of arguments
bool check_args(int argc, int neccesary_argc);
bool vcheck_args(int argc, int neccesary_argc);

// File operations
int safe_open(const char* file, int oflag, int mode);
int safe_close(int fd, const char* filename);
int fd_write(int fd_src, int fd_dest, const char *filename_src,
             const char* filename_dest, char* buf);
ssize_t safe_write(int fd, const char* filename, char* buf, ssize_t n);

pid_t safe_fork(void);
int safe_pipe(int pipedes[2]);
int safe_dup2 (int fd, int fd2);

// SystemV IPC queue operations
int safe_msgget(key_t key, int flags);
int safe_msgsnd(int queue_id, const void* msg_buf, size_t msg_size,
                int msg_flags);
ssize_t safe_msgrcv(int queue_id, void* msg_buf, size_t msg_size,
                    long msg_type, int msg_flags);
int safe_msgctl(int queue_id, int cmd, struct msqid_ds* buf);

// POSIX IPC message queue operations
mqd_t safe_mq_open (const char* name, int oflag, ...);
int safe_mq_send(mqd_t mqdes, const char *msg_ptr,
                 size_t msg_len, unsigned msg_prio);
ssize_t safe_mq_receive(mqd_t mqdes, char *msg_ptr,
                        size_t msg_len, unsigned *msg_prio);
int safe_mq_close(mqd_t queue_id);
int safe_mq_unlink(const char* name);

// SystemV IPC semaphore operations
int safe_semget (key_t key, int num_semaphors, int semflg);
int safe_semctl (int semid, int semnum, int cmd, ...);
int safe_semop(int semid, struct sembuf *operations_array,
               unsigned number_operations);

// POSIX semaphore operations               
sem_t* safe_sem_open(const char* name, int oflag, ...);
int safe_sem_close(sem_t *sem);
int safe_sem_wait(sem_t* sem);
int safe_sem_post(sem_t* sem);
int safe_sem_unlink(const char* name);

// Mapping operations
void* safe_mmap(void* start, size_t length, int prot , int flags,
                int fd, off_t offset);
int safe_munmap(void* start, size_t length);

// POSIX Shared memory operations
int safe_shm_open(const char *name, int oflag, mode_t mode);
int safe_shm_unlink(const char* name);

// POSIX Dirent operations
DIR* safe_opendir(const char* name);
struct dirent* safe_readdir(DIR* dir);
int safe_closedir(DIR* dir);
