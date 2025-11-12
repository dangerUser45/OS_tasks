#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "color.h"
#include "safe_lib.h"

#define $ fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);

static void parent(pid_t pid, char* str, size_t length);
static void child(pid_t parent_pid, size_t length);

static void receiver(int sig);
static void sender(void);

const int BITS_IN_BYTE = 8;

volatile sig_atomic_t current_bit = 0;

//--------------------------------------------------------------
int main(int argc, char** argv) {
  char* str = "Hello World!\n"; 
  if(argc != 1) str = argv[1];
  size_t length = strlen(str);

  sigset_t set;
  sigemptyset(&set);

  sigaddset(&set, SIGUSR1); 
  sigaddset(&set, SIGUSR2);
  
  sigprocmask(SIG_BLOCK, &set, NULL);
  
  pid_t parent_pid = getpid();
  pid_t pid = safe_fork();
  if(pid != 0) {
    parent(pid, str, length);
    wait(NULL);
  }
  else child(parent_pid, length);
}
//--------------------------------------------------------------
static void parent(pid_t pid, char* str, size_t length) {
  struct sigaction act = {.sa_handler = (void (*)(int))sender };
  sigaction(SIGUSR1, &act, NULL);

  sigset_t unblock_mask;
  sigemptyset(&unblock_mask);
  
  printf("Source data (little endian):\n0 1 2 3 4 5 6 7\n"); fflush(stdout);
  for(size_t i = 0; i < length; ++i) {
    for(int j = 0; j < BITS_IN_BYTE; ++j) {
      bool bit = str[i] & 1 << j;
      /* FIXME: DEBUG */ printf(GREEN "%d " RESET, bit);
      
      if(bit == 0) kill(pid, SIGUSR1);
      else kill(pid, SIGUSR2);
      sigsuspend(&unblock_mask);
    }
    printf("- '%c'" RESET, str[i]); fflush(stdout);
    printf("\n"); fflush(stdout); 
  }
  kill(pid, SIGTERM);
}
//--------------------------------------------------------------
static void child(pid_t parent_pid, size_t length) {
  struct sigaction act = {.sa_handler = receiver};
  sigaction(SIGUSR1, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGTERM, &act, NULL);

  sigset_t unblock_mask;
  sigemptyset(&unblock_mask);

  char buf[length + 1] = {};

  while(true) {
    for(size_t i = 0; i < length; ++i) {
      char symbol = 0;
      for(int j = 0; j < BITS_IN_BYTE; ++j) {
        sigsuspend(&unblock_mask);
        if(current_bit == -1) goto exit;
        symbol |= current_bit << j;
        kill(parent_pid, SIGUSR1);
      }
      buf[i] = symbol;
    }
  }
  exit:
  printf( "Received data:\n"); fflush(stdout);
  buf[length] = '\0';
  printf(ORANGE "%s" RESET "\n", buf); fflush(stdout);

  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
static void receiver(int sig) {
  if(sig == SIGUSR1) current_bit = 0;
  else if(sig == SIGUSR2) current_bit = 1;
  else if(sig == SIGTERM) current_bit = -1;
}
//--------------------------------------------------------------
static void sender(void) {}
//--------------------------------------------------------------
