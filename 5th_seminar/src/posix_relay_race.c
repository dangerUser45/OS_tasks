#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "color.h"
#include "safe_lib.h"

// #define DEBUG
#ifdef DEBUG
  #define printf(format, ...) { \
    fprintf(stdout, format, ##__VA_ARGS__); fflush(stdout);\
  }
#endif

#define NO_PRIORITY 0
#define BUF_SIZE 256

void judge(mqd_t* mq_des, int number_runners);
void runners(mqd_t* mq_des, int number_runners, int id_runner);
const char* where_from(char* str);

//--------------------------------------------------------------
int main(int argc, char** argv){

  if(argc != 2) {
    fprintf(stderr, "incorrect argument numbers\n");
    exit(-1);
  }

  int number_runners = atoi(argv[1]);

  struct mq_attr attr = {
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_msgsize = BUF_SIZE,
  };
  char buf[BUF_SIZE] = {};
  mqd_t mq_des[number_runners + 1] = {};
  for(int i = 0; i < number_runners + 1; ++i) {
    sprintf(buf, "/queue_%d", i);
    mq_des[i] = safe_mq_open(buf, O_CREAT | O_RDWR, 0666, &attr);
  }
  
  pid_t pid_judge = safe_fork();
  if(pid_judge == 0) {
    judge(mq_des, number_runners);
    exit(0);
  }

  for(int i = 0; i < number_runners; ++i) {
    pid_t pid_runners = safe_fork();
    if(pid_runners == 0) {
      runners(mq_des, number_runners, i + 1);
      exit(0);
    }
  }

  for(int i = 0; i < number_runners + 1; ++i) {
    wait(NULL);
    safe_mq_close(mq_des[i]);
    sprintf(buf, "/queue_%d", i);
    safe_mq_unlink(buf);
  }
}
//--------------------------------------------------------------
void judge(mqd_t* mq_des, int number_runners) {
  {
  printf(COLOR_TEXT(YELLOW,"-Judge:    ")
          "Hello! I'm a judge. Now i will wait %d runners...\n",
    number_runners); }

  //Judge wait runners
  char buf[BUF_SIZE] = {};
  for (int i = 0; i < number_runners; ++i) {
    safe_mq_receive(mq_des[0], buf, BUF_SIZE, NO_PRIORITY);
    printf(COLOR_TEXT(YELLOW, "-Judge:    ")
          "I waited for the %s runner\n", buf);
  }

  //Judge passes the baton to the first runner
  printf(YELLOW "-Judge:    " RESET
          "I'm starting to give the baton to the first runner\n");
  printf(YELLOW "-Judge:    " RED "Ready, " YELLOW "set, " GREEN "go!" RESET "\n");
  struct timeval start, end;
  gettimeofday(&start, 0);

  sprintf(buf, "judge");
  safe_mq_send(mq_des[1], buf, BUF_SIZE, NO_PRIORITY);

  // safe_msgrcv(queue_id, &msgbuf, sizeof(msgbuf.senders_id), number_runners + 1, 0);
  safe_mq_receive(mq_des[0], buf, BUF_SIZE, NO_PRIORITY);
  gettimeofday(&end, 0);

  printf(YELLOW "-Judge:    " RESET
      "I get the batton from %s\n", where_from(buf));
  double elapsed_time = (double)(end.tv_usec - start.tv_usec
                      + (1000000)*(end.tv_sec - start.tv_sec));
  printf(YELLOW "-Judge:    " GREEN "The relay race is over!\n\n" RESET);
  printf(SKY_BLUE "Elapsed_time = %lf microseconds" RESET "\n", elapsed_time);
}
//--------------------------------------------------------------
void runners(mqd_t* mq_des, int number_runners, int id_runner) {
  printf(COLOR_TEXT(RED, "-Runner " SKY_BLUE "%d:" RESET " ")
          "I'm ready to run\n", id_runner);

  //Runners tell the judge that they are ready for the relay race
  char buf[256] = {};
  sprintf(buf, "%d", id_runner);
  safe_mq_send(mq_des[0], buf, 256, NO_PRIORITY);

  //Runners pass the batton
  safe_mq_receive(mq_des[id_runner], buf, BUF_SIZE, NO_PRIORITY);
  printf(RED "-Runner " SKY_BLUE "%d: " RESET
        "I get the batton from %s "
        "and now start running\n", id_runner, where_from(buf));

  sprintf(buf, "%d", id_runner);
  mqd_t mqd = mq_des[id_runner + 1];
  if(id_runner == number_runners) mqd = mq_des[0]; 
  safe_mq_send(mqd, buf, BUF_SIZE, NO_PRIORITY);
}
//--------------------------------------------------------------
const char* where_from(char* str) {
  if (!strncmp(str, "judge", 5))  return YELLOW "judge" RESET;
  else if (!strncmp(str, "1\0", 2)) return SKY_BLUE "1st" RESET " runner";
  else if (!strncmp(str, "2\0", 2)) return SKY_BLUE "2nd" RESET " runner";
  else if (!strncmp(str, "3\0", 2)) return SKY_BLUE "3rd" RESET " runner";
  else {
    static char buf[256] = {};
    sprintf(buf, SKY_BLUE "%sth" RESET " runner", str);
    return buf;
  }
  return str;
}
//--------------------------------------------------------------
