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
const char* MUTEX_BRIDGE          = "/sem_bridge";
const char* JUST_MUTEX            = "/just_mutex";
const char* SEM_QUEUE_SHIPS       = "/sem_queue_ships";

struct Context {
  sem_t* mutex_cars;
  sem_t* mutex_ships;
  sem_t* mutex_bridge;
  sem_t* just_mutex;
  sem_t* sem_queue_ships;

  int* mem_shm_waiting_ships;

  size_t length_shm_waiting_ships;

  int num_cars;
  int num_ships;

  int shm_waiting_ships;
};

static void ctor (int argc, char** argv, struct Context * const context);
static void dtor (struct Context* context);

static void launch_group(int num_people, struct Context* const context,
                         void (* func_ptr)(struct Context* const context,
                                           int num_human));

static void car  (struct Context* const context, int car_id);
static void ship (struct Context* const context, int ship_id);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  struct Context context = {};
  ctor(argc, argv, &context);

  launch_group(context.num_cars,  &context, car);
  launch_group(context.num_ships, &context, ship);

  int num_process = context.num_cars + context.num_ships;
  for (int i = 0; i < num_process; ++i)
    wait(NULL);

  dtor(&context);
  return 0;
}
//--------------------------------------------------------------
static void ctor(int argc, char** argv, struct Context* const context) {
  // num_cars + num_ships + program
  check_args(argc, 3);

  setbuf(stdout, NULL);

  int num_cars  = atoi(argv[1]);
  int num_ships = atoi(argv[2]);

  sem_unlink(MUTEX_CARS);
  sem_unlink(MUTEX_SHIPS);
  sem_unlink(MUTEX_BRIDGE);
  sem_unlink(JUST_MUTEX);
  sem_unlink(SEM_QUEUE_SHIPS);
  shm_unlink(SHM_NUM_WAITING_SHIPS);

  sem_t* mutex_cars = safe_sem_open(MUTEX_CARS,
                                    O_CREAT | O_EXCL | O_RDWR, 0666, 1);

  sem_t* mutex_ships = safe_sem_open(MUTEX_SHIPS,
                                     O_CREAT | O_EXCL | O_RDWR, 0666, 1);

  sem_t* sem_bridge = safe_sem_open(MUTEX_BRIDGE,
                                    O_CREAT | O_EXCL | O_RDWR, 0666, 1);


  sem_t* just_mutex = safe_sem_open(JUST_MUTEX,
                                    O_CREAT | O_EXCL | O_RDWR, 0666, 1);

  sem_t* sem_queue_ships = safe_sem_open(SEM_QUEUE_SHIPS,
                                         O_CREAT | O_EXCL | O_RDWR, 0666, 0);

  int shm_waiting_ships = safe_shm_open(SHM_NUM_WAITING_SHIPS,
                                        O_CREAT | O_EXCL | O_RDWR, 0666);
  size_t length_shm_waiting_ships = sizeof(int);
  ftruncate(shm_waiting_ships, length_shm_waiting_ships);

  int* mem_shm_waiting_ships =
      safe_mmap(NULL, length_shm_waiting_ships,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                shm_waiting_ships, 0);

  *mem_shm_waiting_ships = 0;

  context->mutex_cars        = mutex_cars;
  context->mutex_ships       = mutex_ships;
  context->mutex_bridge      = sem_bridge;
  context->just_mutex        = just_mutex;
  context->sem_queue_ships   = sem_queue_ships;

  context->num_cars          = num_cars;
  context->num_ships         = num_ships;
  context->shm_waiting_ships = shm_waiting_ships;
  context->mem_shm_waiting_ships   = mem_shm_waiting_ships;
  context->length_shm_waiting_ships = length_shm_waiting_ships;
}
//--------------------------------------------------------------
static void dtor(struct Context* const context) {
  safe_sem_close(context->mutex_cars);
  safe_sem_close(context->mutex_ships);
  safe_sem_close(context->mutex_bridge);
  safe_sem_close(context->just_mutex);
  safe_sem_close(context->sem_queue_ships);

  safe_sem_unlink(MUTEX_CARS);
  safe_sem_unlink(MUTEX_SHIPS);
  safe_sem_unlink(MUTEX_BRIDGE);
  safe_sem_unlink(JUST_MUTEX);
  safe_sem_unlink(SEM_QUEUE_SHIPS);

  safe_munmap(context->mem_shm_waiting_ships,
              context->length_shm_waiting_ships);
  safe_close(context->shm_waiting_ships, SHM_NUM_WAITING_SHIPS);
  safe_shm_unlink(SHM_NUM_WAITING_SHIPS);
}
//--------------------------------------------------------------
static void launch_group(int num_people, struct Context* const context,
                         void (* func_ptr)(struct Context* const context,
                                           int num_human)) {
  for (int i = 1; i <= num_people; ++i) {
    pid_t pid = safe_fork();
    if (!pid) {
      func_ptr(context, i);
      exit(EXIT_SUCCESS);
    }
  }
}
//--------------------------------------------------------------
static void car(struct Context* const context, int car_id) {
  printf(RED "Я %d машина и я еду к мосту." RESET "\n", car_id);

  safe_sem_wait(context->mutex_cars);
  safe_sem_wait(context->mutex_bridge);
  printf(RED "Я %d машина и я подъехала к мосту." RESET "\n", car_id);

  safe_sem_post(context->mutex_cars);

  printf(RED "Я %d машина и я сейчас " YELLOW "еду по мосту." RESET "\n", car_id);
  usleep(2000);

  safe_sem_post(context->mutex_bridge);
  printf(RED "Я %d машина и я проехала мост." RESET "\n", car_id);

  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
static void ship(struct Context* const context, int ship_id) {
  printf(SKY_BLUE "Я %d корабль. Я плыву к мосту." RESET "\n", ship_id);
  printf(SKY_BLUE "Я %d корабль и я подплыл к мосту." RESET "\n", ship_id);

  safe_sem_wait(context->just_mutex);

  int waiting_ships = ++(*(context->mem_shm_waiting_ships));

  if (waiting_ships == 3) {
    printf(SKY_BLUE "Я %d корабль: третий в очереди, прошу развести мост." RESET "\n",
           ship_id);
    safe_sem_wait(context->mutex_cars);

    safe_sem_post(context->sem_queue_ships);
  }

  safe_sem_post(context->just_mutex);
  safe_sem_wait(context->sem_queue_ships);
  safe_sem_wait(context->mutex_bridge);

  printf(SKY_BLUE "Я %d корабль и я проплываю "
                  YELLOW "под мостом." RESET "\n", ship_id);
  usleep(2000);
  printf(SKY_BLUE "Я %d корабль и я проплыл мост." RESET "\n", ship_id);

  safe_sem_post(context->mutex_bridge);

  safe_sem_wait(context->just_mutex);

  waiting_ships = --(*(context->mem_shm_waiting_ships));

  if (waiting_ships > 0) {
    safe_sem_post(context->sem_queue_ships);
  } else {

    printf(SKY_BLUE "Я %d корабль: я последний, мост сводят, машины могут ехать."
           RESET "\n", ship_id);
    safe_sem_post(context->mutex_cars);
  }

  safe_sem_post(context->just_mutex);

  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
