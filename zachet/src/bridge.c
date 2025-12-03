#include <fcntl.h>
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

const char* MUTEX_CARS            = "/cars_mutex";
const char* MUTEX_SHIPS           = "/ships_mutex";
const char* SHM_NUM_WAITING_SHIPS = "/shm_num_waiting_ships";

struct Context {
  sem_t* mutex_cars;
  sem_t* mutex_ships;
  int* mem_shm_waiting_ships;

  size_t length_shm_waiting_ships;

  int num_cars;
  int num_ships;

  int shm_waiting_ships;
};

static void ctor(int argc, char** argv, struct Context * const context);
static void dtor(struct Context* context);

static void launch_group(int num_people, struct Context* const context,
                         void (* funcy_ptr)(struct Context* const context,
                         int num_human));

static void car(struct Context* const context, int num_human);
static void ship(struct Context* const context, int num_human);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  struct Context context = {};
  ctor(argc, argv, &context);

  launch_group(context.num_cars, &context, car); 
  launch_group(context.num_ships, &context, ship);

  int num_process = context.num_cars+ context.num_ships;
  for(int i = 0; i < num_process; ++i) wait(NULL);
  dtor(&context);
}
//--------------------------------------------------------------
static void ctor(int argc, char** argv, struct Context* const context) {
  // num_cars + num_ship + programm
  check_args(argc, 3);

  setbuf(stdout, NULL);
  
  int num_cars  = atoi(argv[1]);
  int num_ships = atoi(argv[2]);

  sem_unlink(MUTEX_CARS);
  sem_unlink(MUTEX_SHIPS);

  sem_t* mutex_cars = safe_sem_open(MUTEX_CARS, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
  
  sem_t* mutex_ships = safe_sem_open(MUTEX_SHIPS, O_CREAT | O_EXCL | O_RDWR, 0666, 1);

  int shm_waiting_ships = safe_shm_open(SHM_NUM_WAITING_SHIPS, O_CREAT | O_EXCL | O_RDWR, 0666);
  size_t length_shm_waiting_ships = sizeof(int);
  ftruncate(shm_waiting_ships, length_shm_waiting_ships);

  int* mem_shm_waiting_ships = safe_mmap(NULL, length_shm_waiting_ships, PROT_READ | PROT_WRITE, MAP_SHARED, shm_waiting_ships, 0);

  context->mutex_cars = mutex_cars;
  context->mutex_ships = mutex_ships;
  context->num_cars = num_cars;
  context->num_ships = num_ships;
  context->shm_waiting_ships = shm_waiting_ships;
  context->mem_shm_waiting_ships = mem_shm_waiting_ships;
  context->length_shm_waiting_ships = length_shm_waiting_ships;
}
//--------------------------------------------------------------
static void dtor(struct Context* const context) {
  safe_sem_close(context->mutex_cars);
  safe_sem_close(context->mutex_ships);

  safe_sem_unlink(MUTEX_CARS);
  safe_sem_unlink(MUTEX_SHIPS);

  safe_munmap(context->mem_shm_waiting_ships, context->length_shm_waiting_ships);
  safe_close(context->shm_waiting_ships, SHM_NUM_WAITING_SHIPS);
  safe_shm_unlink(SHM_NUM_WAITING_SHIPS);
}
//--------------------------------------------------------------
static void launch_group(int num_people, struct Context* const context,
                         void (* func_ptr)(struct Context* const context,
                         int num_human)) {
  for(int i = 1; i < num_people + 1; ++i) {
    pid_t pid = safe_fork();
    if(!pid) func_ptr(context, i);
  }
}
//--------------------------------------------------------------
static void car(struct Context* const context, int car_id) {
  printf(RED "Я %d машина и я еду к мосту." RESET "\n", car_id);

  printf(RED "Я %d машина и я подъехал мосту." RESET "\n", car_id);
  
  safe_sem_wait(context->mutex_cars);  //mutex: on
  // машина едет по мосту
  printf(RED "Я %d машина и я сейчас " YELLOW "еду на мосту." RESET "\n", car_id);
  usleep(2000);
  safe_sem_post(context->mutex_cars);  //mutex: off
  
  printf(RED "Я %d машина и я проехала мост." RESET "\n", car_id);

  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
static void ship(struct Context* const context, int ship_id) {
  printf(SKY_BLUE "Я %d коралбль. Я плыву к мосту." RESET "\n", ship_id);

  printf(SKY_BLUE "Я %d корабль и я подплыл к мосту." RESET "\n", ship_id);
  
  safe_sem_wait(context->mutex_ships);  //mutex: on
  // корабль плывёт по рукаву
  printf(RED "Я %d корабль и я проплываю " YELLOW "под мостом." RESET "\n", ship_id);
  usleep(2000);

  safe_sem_post(context->mutex_ships);  //mutex: off
  printf(RED "Я %d корабль и я проплыл мост." RESET "\n", ship_id);

  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
