/* Вычислить размер системного буфера pipe используя факт блокировки write*/

#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <stdint.h>
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
#include <semaphore.h>
#include <signal.h>

#include "color.h"
#include "safe_lib.h"

void sender(int);
void receiver(int);
void timeout_handler(int);

static volatile sig_atomic_t timed_out = 0;

//--------------------------------------------------------------

int main(int argc, char** argv) {
  int fds[2] = {}; // pipe[0] - end, pipe[1] - start

  if (pipe(fds) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  pid_t pid_parent = getpid();
  pid_t pid_child  = fork();

  char buf[2] = {};
  size_t pipe_size_counter = 0;

  sigset_t unblock_mask;
  sigemptyset(&unblock_mask);

  sigset_t set;
  sigemptyset(&set);

  sigaddset(&set, SIGUSR1); 
  sigaddset(&set, SIGALRM);
  
  sigprocmask(SIG_BLOCK, &set, NULL);

  if (pid_child > 0) // parent pid - only reader
  {

    struct sigaction act_usr = {.sa_handler = (void (*)(int))receiver };
    sigemptyset(&act_usr.sa_mask);
    act_usr.sa_flags = 0;
    sigaction(SIGUSR1, &act_usr, NULL);

    // set timer handler
    struct sigaction act_alrm = {.sa_handler = (void (*)(int))timeout_handler };
    sigemptyset(&act_alrm.sa_mask);
    act_alrm.sa_flags = 0;
    sigaction(SIGALRM, &act_alrm, NULL);

    safe_close(fds[1], "pipe input");

    unsigned timeout_sec = 3;
    alarm(timeout_sec);

    while (!timed_out) {
      sigsuspend(&unblock_mask);

      if (timed_out)
        break;

      ++pipe_size_counter;
      kill(pid_child, SIGUSR1);
      alarm(timeout_sec);
    }

    printf("pipe buffer size: %zu bytes\n", pipe_size_counter);


    kill(pid_child, SIGKILL);
    waitpid(pid_child, NULL, 0);

    exit(EXIT_SUCCESS);
  }
  else {
    safe_close(fds[0], "pipe output");
    struct sigaction act = {.sa_handler = (void (*)(int))sender };
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    while(true) {
      buf[0] = (char)pipe_size_counter;
      int error = safe_write(fds[1], "tmp buffer", buf, 1);

      kill(pid_parent, SIGUSR1);
      sigsuspend(&unblock_mask);

      ++pipe_size_counter;
    }

    exit(EXIT_SUCCESS);
  }
}

//--------------------------------------------------------------
void sender(int signal) {}
void receiver(int signal) {}

void timeout_handler(int signal) {
  timed_out = 1;
}
