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

static void chief();
static void courier();

#define PIZZAS_INGRIDIENTS 5

//--------------------------------------------------------------
int main(int argc, char** argv) {

  check_args(argc, 4);

  int num_tables    = atoi(argv[1]);
  int num_chiefs   = atoi(argv[2]);
  int num_couriers = atoi(argv[3]);

  sem_t* sem_empty = sem_open("empty", O_CREAT | O_RDWR, 0666, num_tables);
  sem_t* sem_ready = sem_open("ready", O_CREAT | O_RDWR, 0666, 0);

  int shm_tables = shm_open("tables", O_CREAT | O_RDWR, 0666);
  size_t length_table_space = PIZZAS_INGRIDIENTS * num_tables + 2;
  ftruncate(shm_tables, length_table_space);

  char* mem_tables = mmap(NULL, length_table_space, PROT_READ | PROT_WRITE, MAP_SHARED, shm_tables, 0);

  for(int i = 0; i < num_chiefs; ++i) {
    pid_t pid = safe_fork();
    if(pid == 0) {
      chief();
    }
  }

  for(int i = 0; i < num_couriers; ++i) {
    pid_t pid = safe_fork();
    if(pid == 0) {
      courier();
    }
  }

  sem_close(sem_empty);
  sem_close(sem_ready);
}
//--------------------------------------------------------------
static void chief() {

}
//--------------------------------------------------------------
static void courier() {

}
//--------------------------------------------------------------
