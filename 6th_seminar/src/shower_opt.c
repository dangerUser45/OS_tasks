#include <features.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "color.h"
#include "safe_lib.h" 

enum names_semaphors {
  SHOWER_CAPACITY,
  GLOBAL_MEN_NUM,
  GLOBAL_WOMEN_NUM,
  SWITCH_MEN_NUM,
  SWITCH_WOMEN_NUM,
  SWITCH_MUTEX
};
enum genders {MAN, WOMAN};
struct Context {
  int sem_id;
  int shower_capacity;
  int number_men;
  int number_women;
  int switch_number;
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

static int ctor(int argc, char** argv, struct Context* context);
static void dtor(int sem_id, int number_people);

static void check_num_gender(unsigned short value_arr[], int number_men,
      int number_women);
static void switch_order(int argc, char** argv,
      unsigned short value_arr[], unsigned short switch_number);

static void gender(int person_id, const struct Context* context,
                   enum genders gender);
static void init_sem(int* global_num, int* switch_num, int* opposite_global_num,
      int* opposite_switch_num, int* number_gender,
      enum genders gender, const struct Context* context,
      const char** color, const char** name_gender);

static void entry_shower(int sem_id, int switch_num, int global_num);
static void exit_shower(int sem_id, int switch_num,
      int global_num, int opposite_global_num, int opposite_switch_num,
      const struct Context* context);

static void man(int man_id, const struct Context* context);
static void woman(int woman_id, const struct Context* context);

static void wait_sem(int sem_id, int num_sem);
static void signal_sem(int sem_id, int num_sem);

static int getvalue_sem(int sem_id, int num_sem);
static int setvalue_sem(int sem_id, int num_sem, int val);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  vcheck_args(argc, 6);
  setbuf(stdout, NULL);

  struct Context context = {};
  if(ctor(argc, argv, &context) != 0) exit(EXIT_FAILURE);

  int number_people = context.number_men + context.number_women;
  for(int i = 1; i < number_people + 1; ++i) {
    pid_t pid = safe_fork();
    if(pid == 0) {
      if(i < context.number_men + 1) { if(context.number_men != 0) man(i, &context); }
      else if(context.number_women != 0) woman(i - context.number_men, &context);
    }
  }

  dtor(context.sem_id, number_people);
}
//--------------------------------------------------------------
static int ctor(int argc, char** argv, struct Context* context) {
  int sem_id = safe_semget(IPC_PRIVATE, 6, IPC_CREAT | 0666);

  int shower_capacity = atoi(argv[1]);
  int number_men    = atoi(argv[2]);
  int number_women  = atoi(argv[3]);
  int switch_number = shower_capacity;
  if(argc == 6) switch_number = atoi(argv[5]);

  context->sem_id = sem_id;
  context->shower_capacity = shower_capacity;
  context->number_men = number_men;
  context->number_women = number_women;
  context->switch_number = switch_number;
  
  unsigned short value_arr[] = {
    shower_capacity,
    number_men,
    number_women,
    switch_number,
    0,
    1
  };
  
  switch_order(argc, argv, value_arr, context->switch_number);
  check_num_gender(value_arr, number_men, number_women);

  union semun arg = {.array = value_arr};
  safe_semctl(sem_id, 0, SETALL, arg);

  return 0;
}
//--------------------------------------------------------------
static void switch_order(int argc, char** argv,
      unsigned short value_arr[], unsigned short switch_number) {
    if (argc >= 5) {
    if(!strncmp("men",argv[4], 3));
    else if(!strncmp("women", argv[4], 5)) {
      value_arr[SWITCH_MEN_NUM] = 0;
      value_arr[SWITCH_WOMEN_NUM] = switch_number;
    }
    else { 
      fprintf(stderr, "Error: options \"%s\" are not exist. "
          "Choice \"men\" or \"women\"\n", argv[4]);
      exit(EXIT_FAILURE);
    }
  }
}
//--------------------------------------------------------------
static void check_num_gender(unsigned short value_arr[], int number_men, int number_women) {
  if(number_men == 0) {
    value_arr[SWITCH_MEN_NUM] = 0;
    value_arr[SWITCH_WOMEN_NUM] = number_women;
  }
  if(number_women == 0) {
    value_arr[SWITCH_WOMEN_NUM] = 0;
    value_arr[SWITCH_MEN_NUM] = number_men;
  }
}
//--------------------------------------------------------------
static void dtor(int sem_id, int number_people) {
  for(int i = 0; i < number_people; ++i) wait(NULL);
  safe_semctl(sem_id, IPC_RMID, 0);
}
//--------------------------------------------------------------
static void gender(int person_id, const struct Context* context,
                   enum genders gender) {
  int sem_id = context->sem_id;
  int GLOBAL_NUM = 0, SWITCH_NUM = 0, OPPOSITE_GLOBAL_NUM = 0,
      OPPOSITE_SWITCH_NUM = 0, number_gender = 0;
  const char* color = 0, * name_gender = 0;

 init_sem(&GLOBAL_NUM, &SWITCH_NUM, &OPPOSITE_GLOBAL_NUM,
          &OPPOSITE_SWITCH_NUM, &number_gender, gender, context,
          &color, &name_gender);

  entry_shower(sem_id, SWITCH_NUM, GLOBAL_NUM);

  printf("%sI'm a %d %s" RESET "\n", color, person_id, name_gender);
  
  exit_shower(sem_id, SWITCH_NUM, GLOBAL_NUM, OPPOSITE_GLOBAL_NUM,
              OPPOSITE_SWITCH_NUM, context);

  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
static void init_sem(int* global_num, int* switch_num, int* opposite_global_num,
      int* opposite_switch_num, int* number_gender,
      enum genders gender, const struct Context* context,
      const char** color, const char** name_gender) {
  if(gender == MAN) {
    *global_num = GLOBAL_MEN_NUM;
    *switch_num = SWITCH_MEN_NUM;
    *opposite_global_num = GLOBAL_WOMEN_NUM;
    *opposite_switch_num = SWITCH_WOMEN_NUM;
    *number_gender = context->number_men;
    *color = BLUE;
    *name_gender = "man";
  } else {
    *global_num = GLOBAL_WOMEN_NUM;
    *switch_num = SWITCH_WOMEN_NUM;
    *opposite_global_num = GLOBAL_MEN_NUM;
    *opposite_switch_num = SWITCH_MEN_NUM;
    *number_gender = context->number_women;
    *color = PINK;
    *name_gender = "woman";
  }
}
//--------------------------------------------------------------
static void entry_shower(int sem_id, int switch_num, int global_num) {
  struct sembuf op_open[] = {
    {switch_num, -1  , 0},
    {global_num, -1, 0},
    {SHOWER_CAPACITY, -1, 0},
  };
  safe_semop(sem_id, op_open, 3);
}
//--------------------------------------------------------------
static void exit_shower(int sem_id, int switch_num,
      int global_num, int opposite_global_num, int opposite_switch_num,
      const struct Context* context) {

  signal_sem(sem_id, SHOWER_CAPACITY);
  wait_sem(sem_id, SWITCH_MUTEX);
  int shower_capacity_val = getvalue_sem(sem_id, SHOWER_CAPACITY);
  int global_num_val = getvalue_sem(sem_id, global_num);
  int opposite_global_num_val = getvalue_sem(sem_id, opposite_global_num);
  int switch_num_val = getvalue_sem(sem_id, switch_num);
  
  if((global_num_val == 0 || switch_num_val == 0) &&
      shower_capacity_val == context->shower_capacity) {
    setvalue_sem(sem_id, switch_num, 0);
    
    int give_opp = (opposite_global_num_val < context->switch_number)
                       ? opposite_global_num_val : context->switch_number;
    if (give_opp > 0) {
      struct sembuf op = {opposite_switch_num, give_opp, 0};
      safe_semop(sem_id, &op, 1);
    }
    else if (global_num_val > 0 &&
             opposite_global_num_val == 0 &&
             switch_num_val == 0){
      int give_self = (global_num_val < context->switch_number)
                        ? global_num_val : context->switch_number;
      if (give_self > 0) {
        struct sembuf op = {switch_num, give_self, 0};
        safe_semop(sem_id, &op, 1);
      }
    }
  }
  signal_sem(sem_id, SWITCH_MUTEX);
}
//--------------------------------------------------------------
void man(int man_id, const struct Context* context) {
  gender(man_id, context, MAN);
}
//--------------------------------------------------------------
void woman(int woman_id, const struct Context* context) {
  gender(woman_id, context, WOMAN);
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
static int getvalue_sem(int sem_id, int num_sem) {
  return safe_semctl(sem_id, num_sem, GETVAL);
}
//--------------------------------------------------------------
static int setvalue_sem(int sem_id, int num_sem, int val) {
  union semun arg = {.val = val};
  return safe_semctl(sem_id, num_sem, SETVAL, arg);
}
//--------------------------------------------------------------
