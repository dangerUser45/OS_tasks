#include <features.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "color.h"
#include "safe_lib.h"

enum names_semaphors {
  SHOWER_CAPACITY = 0,
  MEN_CUR_NUM     = 1,
  WOMEN_CUR_NUM   = 2,
  PRINT_SEM       = 3
};

struct Context {
  int sem_id;
  int number_seats;
  int number_men;
  int number_women;  
};

//from <bits/sem.h>
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun
{
  int val;				           //<= value for SETVAL
  struct semid_ds *buf;		   //<= buffer for IPC_STAT & IPC_SET
  unsigned short int *array; //<= array for GETALL & SETALL
  struct seminfo *__buf;		 //<= buffer for IPC_INFO
};
#endif

static void man(int man_id, const struct Context* context);
static void woman(int woman_id, const struct Context* context);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  check_args(argc, 4);

  int sem_id = safe_semget(IPC_PRIVATE, 4, IPC_CREAT | 0666);

  int number_seats  = atoi(argv[1]);
  int number_men    = atoi(argv[2]);
  int number_women  = atoi(argv[3]);
  int number_people = number_men + number_women;
  
  const struct Context context = {.sem_id = sem_id,
                                  .number_seats = number_seats,
                                  .number_men   = number_men,
                                  .number_women = number_women};
  
  unsigned short value_arr[4] = {number_seats, 0, 0, 1}; 
  union semun arg = {.array = value_arr};
  safe_semctl(sem_id, 0, SETALL, arg);

  for(int i = 1; i < number_people + 1; ++i) {
    pid_t pid = safe_fork();
    if(pid == 0) {
      if(i < number_men + 1)
        man(i, &context);
      else
        woman(i - number_men, &context);
    }
  }

  for(int i = 0; i < number_people; ++i) wait(NULL);
  safe_semctl(sem_id, IPC_RMID, 0);
}
//--------------------------------------------------------------
void man(int man_id, const struct Context* context) {
  struct sembuf op_open[3] = {
    {SHOWER_CAPACITY, -1, 0},
    {MEN_CUR_NUM, 1  , 0},
    {WOMEN_CUR_NUM, 0, 0}
  };
  semop(context->sem_id, op_open, 3);

  // int free_seats = safe_semctl(context->sem_id, SHOWER_CAPACITY, GETVAL);
  // printf("current people in shower = %d\n", context->number_seats - free_seats); fflush(stdout);

  struct sembuf print_op = {PRINT_SEM, -1, 0};
  safe_semop(context->sem_id, &print_op, 1);
  printf(BLUE "I'm a %d man" RESET "\n", man_id);
  print_op.sem_op = 1;
  safe_semop(context->sem_id, &print_op, 1);

  struct sembuf op_exit[3] = {
    {SHOWER_CAPACITY, 1, 0},
    {MEN_CUR_NUM, -1, 0},
    {WOMEN_CUR_NUM, 0, 0}
  };
  semop(context->sem_id, op_exit, 3);

  exit(0);
}
//--------------------------------------------------------------
void woman(int woman_id, const struct Context* context) {
  struct sembuf op_open[3] = {
    {SHOWER_CAPACITY, -1, 0},
    {MEN_CUR_NUM, 0  , 0},
    {WOMEN_CUR_NUM, 1, 0}
  };
  semop(context->sem_id, op_open, 3);

  // int free_seats = safe_semctl(context->sem_id, SHOWER_CAPACITY, GETVAL);
  // printf("current people in shower = %d\n", context->number_seats - free_seats); fflush(stdout);
  
  struct sembuf print_op = {PRINT_SEM, -1, 0};
  safe_semop(context->sem_id, &print_op, 1);
  printf(PINK "I'm a %d woman" RESET "\n", woman_id);
  print_op.sem_op = 1;
  safe_semop(context->sem_id, &print_op, 1);

  struct sembuf op_exit[3] = {
    {SHOWER_CAPACITY, 1, 0},
    {MEN_CUR_NUM, 0, 0},
    {WOMEN_CUR_NUM, -1, 0}
  };
  semop(context->sem_id, op_exit, 3);

  exit(0);
}
//--------------------------------------------------------------
