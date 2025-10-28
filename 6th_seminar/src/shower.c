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
  SHOWER_CAPACITY,
  LOCAL_MEN_NUM,
  LOCAL_WOMEN_NUM,
  GLOBAL_MEN_NUM,
  GLOBAL_WOMEN_NUM,
  BEGIN_MUTEX,
  END_MUTEX
};

struct Context {
  int sem_id;
  int shower_capacity;
  int number_men;
  int number_women;  
};

//from <bits/sem.h>
#if defined(_SEM_SEMUN_UNDEFINED)
union semun
{
  int val;                   //<= value for SETVAL
  struct semid_ds *buf;      //<= buffer for IPC_STAT & IPC_SET
  unsigned short int *array; //<= array for GETALL & SETALL
  struct seminfo *__buf;     //<= buffer for IPC_INFO
};
#endif

static void man(int man_id, const struct Context* context);
static void woman(int woman_id, const struct Context* context);

static void wait_sem(int sem_id, int num_sem);
static void signal_sem(int sem_id, int num_sem);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  check_args(argc, 4);
  setbuf(stdout, NULL);

  int sem_id = safe_semget(IPC_PRIVATE, 7, IPC_CREAT | 0666);

  int shower_capacity = atoi(argv[1]);
  int number_men    = atoi(argv[2]);
  int number_women  = atoi(argv[3]);
  int number_people = number_men + number_women;
  
  const struct Context context = {.sem_id = sem_id,
                                  .shower_capacity = shower_capacity,
                                  .number_men   = number_men,
                                  .number_women = number_women};
  unsigned short value_arr[] = {
    shower_capacity,
    0,
    0,
    number_men,
    number_women,
    1,
    1,
  };
 
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
  int sem_id = context->sem_id;

  wait_sem(sem_id, BEGIN_MUTEX);
  int is_first = safe_semctl(sem_id, GLOBAL_WOMEN_NUM, GETVAL) == context->number_women; 
  if(is_first) {
    union semun arg = {.val = 0};
    safe_semctl(sem_id, GLOBAL_WOMEN_NUM, SETVAL, arg);

    wait_sem(sem_id, GLOBAL_MEN_NUM);
    signal_sem(sem_id, BEGIN_MUTEX);
  }
  else {
    signal_sem(sem_id, BEGIN_MUTEX);
    wait_sem(sem_id, GLOBAL_MEN_NUM);
  }

  struct sembuf op_open[] = {
    {SHOWER_CAPACITY, -1, 0},
    {LOCAL_MEN_NUM, 1  , 0},
  };
  safe_semop(context->sem_id, op_open, 2);

  printf(BLUE "I'm a %d man" RESET "\n", man_id);

  struct sembuf op_exit[] = {
    {SHOWER_CAPACITY, 1, 0},
    {LOCAL_MEN_NUM, -1, 0},
  };
  safe_semop(context->sem_id, op_exit, 2);

  wait_sem(sem_id, END_MUTEX);
  if(safe_semctl(sem_id, GLOBAL_MEN_NUM, GETVAL) == 0
    && safe_semctl(sem_id, LOCAL_MEN_NUM, GETVAL) == 0) {
    union semun arg = {.val = context->number_women};
    safe_semctl(sem_id, GLOBAL_WOMEN_NUM, SETVAL, arg);
  }
  signal_sem(sem_id, END_MUTEX);

  exit(0);
}
//--------------------------------------------------------------
void woman(int woman_id, const struct Context* context) {
   int sem_id = context->sem_id;

  wait_sem(sem_id, BEGIN_MUTEX);
  int is_first = safe_semctl(sem_id, GLOBAL_MEN_NUM, GETVAL) == context->number_men; 
  if(is_first) {
    union semun arg = {.val = 0};
    safe_semctl(sem_id, GLOBAL_MEN_NUM, SETVAL, arg);

    wait_sem(sem_id, GLOBAL_WOMEN_NUM);
    signal_sem(sem_id, BEGIN_MUTEX);
  }
  else {
    signal_sem(sem_id, BEGIN_MUTEX);
    wait_sem(sem_id, GLOBAL_WOMEN_NUM);
  }

  struct sembuf op_open[] = {
    {SHOWER_CAPACITY, -1, 0},
    {LOCAL_WOMEN_NUM, 1, 0}  };
  safe_semop(context->sem_id, op_open, 2);

  printf(PINK "I'm a %d woman" RESET "\n", woman_id);

  struct sembuf op_exit[] = {
    {SHOWER_CAPACITY, 1, 0},
    {LOCAL_WOMEN_NUM, -1, 0}
  };
  safe_semop(context->sem_id, op_exit, 2);

  wait_sem(sem_id, END_MUTEX);
  if(safe_semctl(context->sem_id, GLOBAL_WOMEN_NUM, GETVAL) == 0
   && safe_semctl(sem_id, LOCAL_WOMEN_NUM, GETVAL) == 0) {
    union semun arg = {.val = context->number_men};
    safe_semctl(sem_id, GLOBAL_MEN_NUM, SETVAL, arg);
  }
  signal_sem(sem_id, END_MUTEX);

  exit(0);
}
//--------------------------------------------------------------
static void wait_sem(int sem_id, int num_sem) {
  struct sembuf op = {num_sem, -1, 0};
  safe_semop(sem_id, &op, 1);
}
//--------------------------------------------------------------
static void signal_sem(int sem_id, int num_sem) {
  struct sembuf op = {num_sem, 1, 0};
  safe_semop(sem_id, &op, 1);
}
//--------------------------------------------------------------
