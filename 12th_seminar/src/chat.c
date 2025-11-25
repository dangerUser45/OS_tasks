#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
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

#include "color.h"
#include "safe_lib.h"

struct Context {
  pid_t* mem_pids_table;
  bool* mem_join_table;

  int id_pids_table;
  int id_join_table;

  int num_people; 
};

const char* const NAME_SHM_PIDS = "pids_table";
const char* const NAME_SHM_JOIN_TABLE = "join_table";

static void human(pid_t pid_connector);
static void chat(pid_t my_pid, pid_t pid);

static void invite(int signumber, siginfo_t* siginfo, void* ucontext);
static void accept(int signumber, siginfo_t* siginfo, void* ucontext);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  check_args(argc, 2);

  setbuf(stdout, NULL);

  int pid_connector = atoi(argv[1]);

  // create shm with all pids
  // int shm_pids_table = safe_shm_open(NAME_SHM_PIDS, O_CREAT | O_EXCL | O_RDWR, 0666);
  // size_t length_pids_table = sizeof(pid_t) * num_people;
  // ftruncate(shm_pids_table, length_pids_table);

  // pid_t* mem_pids_table = safe_mmap(NULL, length_pids_table, PROT_READ | PROT_WRITE, MAP_SHARED, shm_pids_table, 0);

  // // create join table shgared memory
  // int shm_join_table = safe_shm_open(NAME_SHM_JOIN_TABLE, O_CREAT | O_EXCL | O_RDWR, 0666);

  // size_t length_join_table = sizeof(bool) * num_people;
  // ftruncate(shm_join_table, length_join_table);

  // bool* mem_join_table = safe_mmap(NULL, length_join_table, PROT_READ | PROT_WRITE, MAP_SHARED, shm_pids_table, 0);
  
  // struct Context ctx = {
  //   .mem_pids_table = mem_pids_table,
  //   .mem_join_table = mem_join_table,

  //   .id_pids_table = shm_pids_table,
  //   .id_join_table = shm_join_table,

  //   .num_people = num_people,
  // };
 
  // blocking signals
  sigset_t set;
  sigemptyset(&set);

  sigaddset(&set, SIGRTMIN); 
  sigaddset(&set, SIGRTMIN + 1); 
  
  sigprocmask(SIG_BLOCK, &set, NULL);

  // set own handlers
  struct sigaction act_invite = {.sa_sigaction = invite, .sa_flags = SA_SIGINFO};
  sigaction(SIGRTMIN, &act_invite, NULL);

  struct sigaction act_accept = {.sa_sigaction = accept, .sa_flags = SA_SIGINFO};
  sigaction(SIGRTMIN + 1, &act_accept, NULL);

  human(pid_connector);

  // //create new process
  // for(int i = 1; i < num_people + 1; ++i) {
  //   pid_t pid = fork();
  //   if(!pid){
  //     human(&ctx, i);
  //   }
  //   else {
  //     mem_pids_table[i] = pid;
  //   }
  // }

  // for(int i = 0; i < num_people; ++i)
  //   wait(NULL);

  // safe_munmap(ctx.mem_pids_table, length_pids_table);
  // safe_close(ctx.id_pids_table, NAME_SHM_PIDS);
  // safe_shm_unlink(NAME_SHM_PIDS);

  // safe_munmap(ctx.mem_join_table, length_join_table);
  // safe_close(ctx.id_join_table, NAME_SHM_JOIN_TABLE);
  // safe_shm_unlink(NAME_SHM_JOIN_TABLE);
}
//--------------------------------------------------------------
static void human(pid_t pid_connector) {
  sigset_t set;
  sigemptyset(&set);

  sigaddset(&set, SIGRTMIN); 
  sigaddset(&set, SIGRTMIN + 1); 
  
  sigprocmask(SIG_BLOCK, &set, NULL);
  
  chat(getpid(), pid_connector);
   
  scanf("%s", str);
  printf("%s", str);
  
  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
static void chat(pid_t my_pid, pid_t pid) {
  sigset_t unblock_mask;
  sigemptyset(&unblock_mask);

  if(pid ==  0) {
    fprintf(stdout, YELLOW "I'm a first member in chat! "
            "My pid = %d" RESET "\n", my_pid);
  }
  else {
    // send signal (request to connect) to process with pid*/
    kill(pid, SIGRTMIN);
    fprintf(stdout, ORANGE "I am a process with a pid = %d send a "
            "request to join the chat process with pid = %d" RESET "\n", my_pid, pid);
    
    sigsuspend(&unblock_mask);
    
    // TODO: make with say
    //fprintf(stdout, "pid = %d joined in the chat!\n", pid);

    // // add mark so process in chat in join_table
    // ctx->mem_join_table[human_num] = true;
  }
}
//--------------------------------------------------------------
static void invite(int signumber, siginfo_t* siginfo, void* ucontext) {
  if(signumber == SIGRTMIN) {}
  kill(siginfo->si_pid, SIGRTMIN + 1);
}
//--------------------------------------------------------------
static void accept(int signumber, siginfo_t* siginfo, void* ucontext) {}
