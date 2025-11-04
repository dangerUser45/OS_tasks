#include <fcntl.h>
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

#include "safe_lib.h"

#define NUM_PIZZAS_INGRIDIENTS 5
#define NAME_SHM "/table_space"

struct Context {
  sem_t* sem_empty;
  sem_t* sem_ready;
  char* mem_table_space;
  
  size_t length_table_space;
  
  int num_tables, num_chiefs, num_couriers;
  int shm_table_space;
};

static void ctor(int argc, char** argv, struct Context* context);
static void dtor(struct Context* context);
static void check_values(int num_tables, int num_chiefs, int num_couruiers);

static void launch_group(int num_people, void (* func_ptr)());

static void chief();
static void courier();

//--------------------------------------------------------------
int main(int argc, char** argv) {
  struct Context context = {};
  ctor(argc, argv, &context);

  launch_group(context.num_chiefs, chief);
  launch_group(context.num_couriers, courier);
  
  dtor(&context);
}
//--------------------------------------------------------------
static void ctor(int argc, char** argv, struct Context* context) {
  check_args(argc, 4);

  int num_tables   = atoi(argv[1]);
  int num_chiefs   = atoi(argv[2]);
  int num_couriers = atoi(argv[3]);

  check_values(num_tables, num_chiefs, num_couriers);

  sem_t* sem_empty = sem_open("empty", O_CREAT | O_RDWR, 0666, num_tables);
  sem_t* sem_ready = sem_open("ready", O_CREAT | O_RDWR, 0666, 0);

  int shm_table_space = shm_open(NAME_SHM, O_CREAT | O_RDWR, 0666);
  size_t length_table_space = NUM_PIZZAS_INGRIDIENTS * num_tables + 2;
  ftruncate(shm_table_space, length_table_space);

  char* mem_table_space = mmap(NULL, length_table_space, PROT_READ | PROT_WRITE, MAP_SHARED, shm_table_space, 0);

  context->num_tables = num_tables;
  context->num_chiefs = num_chiefs;
  context->num_couriers = num_couriers;

  context->sem_empty = sem_empty;
  context->sem_ready = sem_ready;
  
  context->shm_table_space = shm_table_space;
  context->length_table_space = length_table_space;

  context->mem_table_space = mem_table_space;
}
//--------------------------------------------------------------
static void check_values(int num_tables, int num_chiefs, int num_couruiers) {
  char error = 0;
  if(num_tables <= 0) { ++error; error <<= 1;}
  if(num_chiefs <= 0) error <<= 1;
  if(num_couruiers <= 0) error <<= 1;
  
  if(error != 0) {
    fprintf(stderr, "%s:%d: Error: check_values():"
      "The value must be strictly greater than zero.\n", __FILE__, __LINE__);

    exit(EXIT_FAILURE);
  }
}
//--------------------------------------------------------------
static void dtor(struct Context* context) {
  sem_close(context->sem_empty);
  sem_close(context->sem_ready);

  munmap(context->mem_table_space, context->length_table_space);

  shm_unlink(NAME_SHM);
}
//--------------------------------------------------------------
static void launch_group(int num_people, void (* func_ptr)()) {
for(int i = 0; i < num_people; ++i) {
    pid_t pid = safe_fork();
    if(pid == 0) {
      func_ptr();
    }
  }
}
//--------------------------------------------------------------
static void chief() {

}
//--------------------------------------------------------------
static void courier() {

}
//--------------------------------------------------------------
