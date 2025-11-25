#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "color.h"
#include "safe_lib.h"

enum condition {EMPTY, TAKEN, READY};

struct Monitor {
  pthread_mutex_t* mutex;
  pthread_cond_t* cond_empty;
  pthread_cond_t* cond_ready;

  char* mem_table_space;
  enum condition* mem_table_context;
  
  int* current_num_chiefs;
  int* ready;
  int* empty;

  int num_tables, num_chiefs, num_couriers;
};

struct Args {
  struct Monitor* monitor;
  int num_human;
};

static void ctor(int argc, char** argv, struct Monitor* monitor);
static void check_values(int num_tables, int num_chiefs, int num_couriers);

static void launch_group(int num_people, struct Args* arg,
  struct Monitor* monitor, pthread_t* threads, void* (*func)(void*));

static void* chief(void* arguments);
static void* courier(void* arguments);

static void dtor(struct Monitor* monitor);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  struct Monitor monitor = {};
  ctor(argc, argv, &monitor);
  int num_people = monitor.num_chiefs + monitor.num_couriers;

  pthread_t threads[num_people] = {};
  struct Args arg[num_people]   = {};
  launch_group(monitor.num_chiefs, arg, &monitor,threads, chief);
  launch_group(monitor.num_couriers, arg, &monitor,threads + monitor.num_chiefs, courier);

  for(int i = 0; i < num_people; ++i)
    pthread_join(threads[i], NULL);
  dtor(&monitor);
}
//--------------------------------------------------------------
static void ctor(int argc, char** argv, struct Monitor* monitor) {
  check_args(argc, 4);

  setbuf(stdout, NULL);

  int num_tables   = atoi(argv[1]);
  int num_chiefs   = atoi(argv[2]);
  int num_couriers = atoi(argv[3]);

  check_values(num_tables, num_chiefs, num_couriers);

  char* mem_table_space = (char*)calloc(num_tables, sizeof(char));
  enum condition* mem_table_context = (enum condition*) calloc (num_tables, sizeof(enum condition));

  int* sem_array = (int*) calloc(3, sizeof(int));
  sem_array[0] = num_chiefs; // current_num_chiefs
  sem_array[1] = num_tables; // empty = num_tables
                             // ready = 0

  pthread_mutex_t* mutex = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex, NULL); 
  
  pthread_cond_t* cond_array = (pthread_cond_t*) calloc(2, sizeof(pthread_cond_t)); 
  pthread_cond_init(cond_array, NULL);
  pthread_cond_init(cond_array + 1, NULL); 

  monitor->mem_table_space    = mem_table_space;
  monitor->mem_table_context  = mem_table_context;
  
  monitor->current_num_chiefs = sem_array;
  monitor->empty = sem_array + 1;
  monitor->ready = sem_array + 2;
  
  monitor->mutex = mutex;
  
  monitor->cond_empty = cond_array;
  monitor->cond_ready = cond_array + 1;

  monitor->num_tables   = num_tables;
  monitor->num_chiefs   = num_chiefs;
  monitor->num_couriers = num_couriers;
}
//--------------------------------------------------------------
static void check_values(int num_tables, int num_chiefs, int num_couriers) {
  char error = 0;
  if(num_tables   <= 0) error |= 1 << 0;
  if(num_chiefs   <= 0) error |= 1 << 1;
  if(num_couriers <= 0) error |= 1 << 2;
  
  if(error != 0) {
    if(error & 1 << 0)
      fprintf(stderr, "%s:%d: Error: check_values(): "
        "The number of tables must be strictly greater than zero.\n", __FILE__, __LINE__);
    if(error & 1 << 1)
      fprintf(stderr, "%s:%d: Error: check_values(): "
        "The number of chiefs must be strictly greater than zero.\n", __FILE__, __LINE__);
    if(error & 1 << 2)
      fprintf(stderr, "%s:%d: Error: check_values(): "
        "The number of couriers must be strictly greater than zero.\n", __FILE__, __LINE__);

    exit(EXIT_FAILURE);
  }
}
//--------------------------------------------------------------
static void dtor(struct Monitor* monitor) {
  pthread_mutex_destroy(monitor->mutex);
  pthread_cond_destroy(monitor->cond_empty);
  pthread_cond_destroy(monitor->cond_ready);

  free(monitor->mem_table_space);
  free(monitor->mem_table_context);
  free(monitor->current_num_chiefs);
  free(monitor->cond_empty);
  free(monitor->mutex);
}
//--------------------------------------------------------------
static void launch_group(int num_people, struct Args* arg,
  struct Monitor* monitor, pthread_t* threads, void* (*func)(void*)) {
  for (int i = 0; i < num_people; ++i) {
    arg[i].monitor = monitor;
    arg[i].num_human = i + 1;
    pthread_create(&threads[i], NULL, func, arg + i);    
  }
}
//--------------------------------------------------------------
void* chief(void* arguments) {
  struct Monitor* monitor = ((struct Args*)arguments)->monitor;
  int num_human = ((struct Args*)arguments)->num_human;
  printf(GREEN "I'm a chief №%d" RESET "\n", num_human);

  
  
  pthread_exit(NULL);
}
//--------------------------------------------------------------
void* courier(void* arguments) {
  struct Monitor* monitor = ((struct Args*)arguments)->monitor;
  int num_human = ((struct Args*)arguments)->num_human;
  printf(ORANGE "I'm a courier №%d" RESET "\n", num_human);
  



  pthread_exit(NULL);
}
//--------------------------------------------------------------
