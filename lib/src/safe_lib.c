#include <errno.h>     // for EINTR, errno
#include <fcntl.h>     // for open, O_CREAT
#include <features.h>  // for __GNU_LIBRARY__
#include <mqueue.h>    // for mqd_t, mq_open, mq_close, mq_receive, mq_send
#include <stdarg.h>    // for va_arg, va_end, va_list, va_start
#include <stdbool.h>   // for true, bool, false
#include <stdio.h>     // for perror, fprintf, stderr
#include <stdlib.h>    // for exit
#include <sys/ipc.h>   // for IPC_RMID
#include <sys/msg.h>   // for msgctl, msgget, msgrcv, msgsnd
#include <sys/sem.h>   // for semctl, semget, semop, GETNCNT, GETPID, GETVAL
#include <unistd.h>    // for close, dup2, fork, pipe, read, write

#include "safe_lib.h"

#ifndef PAGE_SIZE
  #define PAGE_SIZE 4096
#endif


#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
      int val;                  /* value for SETVAL */
      struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
      unsigned short *array;    /* array for GETALL, SETALL */
                                /* Linux specific part: */
      struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif

//--------------------------------------------------------------
bool check_args(int argc, int neccesary_argc) {
  if (argc != neccesary_argc) {
    fprintf(stderr, "Error: check_args: Invalid number of arguments\n");
    exit(-1);
  }
  return false;
}
//--------------------------------------------------------------
int safe_open(const char* file, int oflag, int mode) {
  int fd = 0;
  if(mode == 0)
    fd = open(file, oflag);
  else
    fd = open(file, oflag, mode);
  
  if(fd < 0)
    perror(file);

  return fd;
}
//--------------------------------------------------------------
int safe_close(int fd, const char* filename) {
  int error = close(fd);
  if(error ==  -1)
    perror(filename);
  
  return error;
}
//--------------------------------------------------------------
int fd_write(int fd_src, int fd_dest, const char* filename_src,
             const char *filename_dest, char *buf) {
  while(true) {
    ssize_t num_sym = read(fd_src, buf, PAGE_SIZE);
    if (num_sym < 0){
        perror(filename_src);
        return num_sym;
    }
    else if(num_sym > 0) {
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

  while(true) {
    buf += num_sym; n -= num_sym;
    num_sym = write(fd, buf, n);
    if(num_sym < 0) {
      if(errno != EINTR) {
          perror(filename);
          return num_sym;
      }
      else continue;
    }
    else if(num_sym == n) return num_sym;
    else if(num_sym == 0) break;
    else continue;
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
int safe_msgget(key_t key, int flags) {
  int code_error = msgget(key, flags);
  if(code_error == -1) {
    perror("msgget");
    exit(-1);
  }

  return code_error;
}
//--------------------------------------------------------------
int safe_msgsnd(int queue_id, const void* msg_buf, size_t msg_size,
                int msg_flags) {
  int code_error = msgsnd(queue_id, msg_buf, msg_size, msg_flags);
  if(code_error == -1) {
    perror("msgsnd");
    exit(-1);
  }

  return code_error;
}
//--------------------------------------------------------------
ssize_t safe_msgrcv(int queue_id, void* msg_buf, size_t msg_size,
                    long msg_type, int msg_flags) {
  ssize_t code_error = msgrcv(queue_id, msg_buf, msg_size, msg_type, msg_flags);
  if(code_error == -1) {
    perror("msgrcv");
    exit(-1);
  }

  return code_error;
}
//--------------------------------------------------------------
int safe_msgctl(int queue_id, int cmd, struct msqid_ds* buf) {
  int code_error = msgctl(queue_id, cmd, buf);
  if(code_error == -1) {
    perror("msgctl");
    exit(-1);
  }

  return code_error;
}   
//--------------------------------------------------------------
mqd_t safe_mq_open (const char* name, int oflag, ...) {
  mqd_t mqd = {};
  if (oflag & O_CREAT) {
    va_list args;
    va_start(args, oflag);
    mode_t mode = va_arg(args, mode_t);
    struct mq_attr* attr = va_arg(args, struct mq_attr*);
    va_end(args);

    mqd = mq_open(name, oflag, mode, attr);
  }
  else mqd = mq_open(name, oflag);

  if (mqd == (mqd_t) -1) {
    perror("mq_open");
    exit(-1);
  }
  return mqd;
}
//--------------------------------------------------------------
int safe_mq_send(mqd_t mqdes, const char *msg_ptr,
                 size_t msg_len, unsigned msg_prio) {
  int code_error = mq_send(mqdes, msg_ptr, msg_len, msg_prio);
  if(code_error == -1) {
    perror("mq_send");
    exit(-1);
  }
  return code_error;
}
//--------------------------------------------------------------
ssize_t safe_mq_receive(mqd_t mqdes, char *msg_ptr,
                        size_t msg_len, unsigned *msg_prio) {
  int code_error = mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
  if(code_error == -1) {
    perror("mq_receive");
    exit(-1);
  }
  return code_error;
}
//--------------------------------------------------------------
int safe_mq_close(mqd_t queue_id) {
  int code_error = mq_close(queue_id);
  if(code_error == -1) {
    perror("mq_close");
    exit(-1);
  }
  return code_error;
}
//--------------------------------------------------------------
int safe_mq_unlink(const char* name) {
  int code_error = mq_unlink(name);
  if(code_error == -1) {
    perror("mq_inlink");
    exit(-1);
  }
  return code_error;
}
//--------------------------------------------------------------
int safe_semget (key_t key, int num_semaphors, int semflg) { 
  int code_error = semget(key, num_semaphors, semflg);
  if(code_error == -1) {
    perror("semget");
    exit(-1);
  }
  return code_error;
}
//--------------------------------------------------------------
int safe_semctl (int semid, int semnum, int cmd, ...) {
  int error = 0;

  // Команды, которые не используют arg
  if (cmd == GETVAL  || cmd == GETPID || cmd == GETNCNT || 
      cmd == GETZCNT || cmd == IPC_RMID)
    error = semctl(semid, semnum, cmd);
  else {
    va_list args;
    va_start(args, cmd);
    union semun arg = va_arg(args, union semun);
    va_end(args);

    error = semctl(semid, semnum, cmd, arg);
  }
  if(error == -1) {
    perror("semctl");
    exit(-1);
  }
  return error;
}
//--------------------------------------------------------------
int safe_semop(int semid, struct sembuf *operations_array,
               unsigned number_operations) {
  int error = semop(semid, operations_array, number_operations);
  if(error < 0) {
    perror("semop");
    exit(-1);
  }
  return error;
}
//--------------------------------------------------------------
