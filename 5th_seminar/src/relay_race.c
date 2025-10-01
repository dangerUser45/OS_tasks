//#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../lib/include/safe_lib.h"

enum race_status {
  runner_ready = 1,
  passing_baton = 2,
};

struct msgbuf {  
  long mtype;               /* тип сообщения, должен быть > 0 */
  enum race_status status;  /* содержание сообщения */
};

void judge(int queue_id, int number_runners);
void runners(int queue_id, int id_runner);

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
      runners(queue_id, i + 1);
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
  printf("Hello! I'm a judge. Now i will wait %d runners...\n",
    number_runners); 
  //fflush(stdout);

  struct msgbuf buf = {};
  for (int i = 0; i < number_runners; ++i) {
    msgrcv(queue_id, &buf,
      sizeof(buf.status), 1, 0);

    if(buf.status == runner_ready) {
      printf("I'm the judge and I've been waiting for %d runner\n", i + 1);
      //fflush(stdout);
    }
  }

  printf("I'm the judge, and I'm starting to give the baton to the first contestant\n");
  struct timeval start, end;
  gettimeofday(&start, 0);

  buf.mtype = 2;
  buf.status = passing_baton;
  msgsnd(queue_id, &buf, sizeof(enum race_status),  0);

  gettimeofday(&end, 0);
  double elapsed_time = (double)(end.tv_usec - start.tv_usec + (1000000)*(end.tv_sec - start.tv_sec));
  
  printf("Elapsed_time = %lf microseconds\n", elapsed_time);
}
//--------------------------------------------------------------
void runners(int queue_id, int id_runner) {
  printf("I'm a %d runner and I'm ready to run\n", id_runner);
  //fflush(stdout);

  struct msgbuf msgbuf = {1, runner_ready};
  msgsnd(queue_id, &msgbuf, sizeof(msgbuf.status), 0);

  msgrcv(queue_id, &buf, );
}
//--------------------------------------------------------------
