#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../../lib/include/safe_lib.h"
#include "../../lib/include/color.h"

#define DEBUG
#ifdef DEBUG
  #define printf(format, ...) { \
    fprintf(stdout, format, ##__VA_ARGS__); fflush(stdout);\
  }
#endif
struct msgbuf {  
  long mtype;
  int senders_id;
};

void judge(int queue_id, int number_runners);
void runners(int queue_id, int number_runners, int id_runner);
const char* where_from(int id_runner);

//--------------------------------------------------------------
int main(int argc, char** argv){

  if(argc != 2) {
    fprintf(stderr, "incorrect argument numbers\n");
    exit(-1);
  }

  int number_runners = atoi(argv[1]);
  int queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
  
  pid_t pid_judge = safe_fork();
  if(pid_judge == 0) {
    //sleep(3);
    judge(queue_id, number_runners);
    exit(0);
  }

  for(int i = 0; i < number_runners; ++i) {
    pid_t pid_runners = safe_fork();
    if(pid_runners == 0) {
      runners(queue_id, number_runners, i + 1);
      exit(0);
    }
  }

  for(int i = 0; i < number_runners + 1; ++i) {
    int status = 0;
    wait(&status);
  }

  msgctl(queue_id, IPC_RMID, 0);
}
//--------------------------------------------------------------
void judge(int queue_id, int number_runners) {
  {
  printf(COLOR_TEXT(YELLOW,"-Judge:    ")
          "Hello! I'm a judge. Now i will wait %d runners...\n",
    number_runners); }

  //Judge wait runners
  struct msgbuf msgbuf = {};
  for (int i = 0; i < number_runners; ++i) {
    msgrcv(queue_id, &msgbuf,
      sizeof(msgbuf.senders_id), number_runners + 2, 0);
      printf(COLOR_TEXT(YELLOW, "-Judge:    ")
          "I waited for the %d runner\n", msgbuf.senders_id);
  }

  //Judge passes the baton to the first runner
  printf(YELLOW "-Judge:    " RESET
          "I'm starting to give the baton to the first runner\n");
  printf(YELLOW "-Judge:    " RED "Ready, " YELLOW "set, " GREEN "go!" RESET "\n");
  struct timeval start, end;
  gettimeofday(&start, 0);

  msgbuf.mtype = 1;
  msgbuf.senders_id = 0;
  msgsnd(queue_id, &msgbuf, sizeof(msgbuf.senders_id),  0);

  msgrcv(queue_id, &msgbuf, sizeof(msgbuf.senders_id), number_runners + 1, 0);
  gettimeofday(&end, 0);

  printf(YELLOW "-Judge:    " RESET
      "I get the batton from %s\n", where_from(msgbuf.senders_id));
  double elapsed_time = (double)(end.tv_usec - start.tv_usec
                      + (1000000)*(end.tv_sec - start.tv_sec));
  printf(YELLOW "-Judge:    " GREEN "The relay race is over!\n\n" RESET);
  printf(SKY_BLUE "Elapsed_time = %lf microseconds" RESET "\n", elapsed_time);
}
//--------------------------------------------------------------
void runners(int queue_id, int number_runners, int id_runner) {
  printf(COLOR_TEXT(RED, "-Runner " SKY_BLUE "%d:" RESET " ")
          "I'm ready to run\n", id_runner);

  //Runners tell the judge that they are ready for the relay race
  struct msgbuf msgbuf = {number_runners + 2, id_runner};
  msgsnd(queue_id, &msgbuf, sizeof(msgbuf.senders_id), 0);

  //Runners pass the baton
  msgrcv(queue_id, &msgbuf, sizeof(msgbuf.senders_id), id_runner, 0);
  printf(RED "-Runner " SKY_BLUE "%d: " RESET
        "I get the batton from %s "
        "and now start running\n", id_runner, where_from(msgbuf.senders_id));

  msgbuf.mtype = id_runner + 1;
  msgbuf.senders_id = id_runner;
  msgsnd(queue_id, &msgbuf, sizeof(msgbuf.senders_id), 0);
}
//--------------------------------------------------------------
const char* where_from(int senders_id) {  
  if(senders_id == 0)      return YELLOW "the judge" RESET;
  else if(senders_id == 1) return SKY_BLUE "1st " RESET "runner";
  else if(senders_id == 2) return SKY_BLUE "2nd " RESET "runner";
  else if(senders_id == 3) return SKY_BLUE "3rd " RESET "runner";
  else {
    static char buf[256];
    sprintf(buf, SKY_BLUE "%dth " RESET "runner", senders_id - 1);
    return buf;
  }
}
//--------------------------------------------------------------
