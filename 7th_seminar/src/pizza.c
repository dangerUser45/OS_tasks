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


/*                  A pseudo-code pizza cooking algorithm                     *\
                    only with semaphores and shared memory

  num_tables, num_chiefs, num_couriers; 

  semaphore ready, empty, mutex;
  shm table_space, shm table_context;

  ready = 0;
  empty = num_tables;
  mutex = 1;

  for(i = 0; i < num_table; ++i) {
    table_space[i]   = empty;
    table_context[i] = empty;
  }

  chief() {
    while (the working day is going on) {
      --empty;

      --mutex;
      num_found_table = found_free_table();
      table_context[num_found_table] = taken;
      ++mutex;

      cook_pizza(table_space[num_found_table]); 

      --mutex;
      table_context[num_found_table] = ready;
      ++ready;
      ++mutex;
    }
  }
    

  courier() {
    while (not all chiefs are left && not all pizzas are delivered) {
      --ready;

      --mutex;
      num_found_table = found_table_with_cooked_pizza();
      table_context[num_found_table] = taken;
      ++mutex;

      take_cooked_pizza_from_table(table_space[num_found_table]);

      --mutex;
      table_context[num_found_table] = empty;
      ++empty;
      ++mutex;
    }
  }

\*                                                                            */

#define $ fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);

const int NECCESARY_NUM_COOKED_PIZZA = 100;
const int NUM_PIZZAS_INGRIDIENTS     = 5;

const char* NAME_TABLE_CONTEXT = "/table_context";
const char* NAME_TABLE_SPACE   = "/table_space";
const char* NAME_READY_SEM     = "/ready";
const char* NAME_EMPTY_SEM     = "/empty";
const char* NAME_MUTEX         = "/mutex";

struct Context {
  sem_t* sem_empty;
  sem_t* sem_ready;
  sem_t* sem_mutex;

  char* mem_table_space;
  enum condition* mem_table_context;
  
  size_t length_table_space;
  size_t length_table_context;
  
  int num_tables, num_chiefs, num_couriers;
  int shm_table_space, shm_table_context;
};

enum condition {EMPTY, TAKEN, READY};

const char*  print_cond(enum condition cond) {
  switch(cond) {
    case EMPTY: return "empty";
    case READY: return "ready";
    case TAKEN: return "taken";
    default: fprintf(stderr, RED "ERROR!!!" RESET "\n");
  }
}

/* FIXME */
void dbg_print(const char* file, int line, struct Context* context, int num_human, const char* profession, const char* format, ... );

#define DBG_PRINT(context, num_human, profession, format, ...) do { dbg_print(__FILE__, __LINE__, context, num_human, profession, format, ##__VA_ARGS__); } while(0); 
  
void dbg_print(const char* file, int line, struct Context* context, int num_human, const char* profession, const char* format, ... ) {
  safe_sem_wait(context->sem_mutex); 

  char buf[4096] = {};
  va_list args;
  va_start(args, format); 
  vsprintf(buf, format, args); 
  va_end(args); 

  fprintf(stderr, GREY "-----------------------------------------\n"); 
  fprintf(stderr, "%s:%d\n", file, line); 
  fprintf(stderr, "I'm a %s №%d: my pid = %d\n", profession, num_human, getpid()); 
  fprintf(stderr, "Current action: %s\n\n", buf); 
  fprintf(stderr, "num_table   = %d\nnum_chiefs  = %d\nnum_courier = %d\n\n", context->num_tables, context->num_chiefs, context->num_couriers); 
  fprintf(stderr, "length_table_space   = %zu\nlength_table_context = %zu\n\n", context->length_table_space, context->length_table_context); 

  int empty = 0, ready = 0, mutex = 0;
  sem_getvalue(context->sem_empty, &empty);
  sem_getvalue(context->sem_ready, &ready);
  sem_getvalue(context->sem_mutex, &mutex);


  fprintf(stderr, "Semaphores:\n"); 
  fprintf(stderr, "\tempty = %d\n", empty); 
  fprintf(stderr, "\tready = %d\n", ready); 
  fprintf(stderr, "\tmutex = %d\n\n", mutex); 

  fprintf(stderr, "Shm context:\n\t№     Address         Value\n"); 
  for(size_t i = 0; i < context->length_table_context / sizeof(int); ++i) { 
    if(i == context->length_table_context / sizeof(int) - 1) { 
      fprintf(stderr, "\t      %p '%d'", context->mem_table_context + i, context->mem_table_context[i]);
      fprintf(stderr, " <-- current num_chiefs\n");
    } 
    else
      fprintf(stderr, "\t%-5zu %p '%s'\n", i + 1, context->mem_table_context + i, print_cond(context->mem_table_context[i]));
  }

  fprintf(stderr, "\nShm space:\n\t№     Address         Value\n");
  for(size_t i = 0; i < context->length_table_space / NUM_PIZZAS_INGRIDIENTS; ++i) { 
    fprintf(stderr, "\t%-5zu %p ", i + 1, context->mem_table_space + i * NUM_PIZZAS_INGRIDIENTS); 
    for(size_t j = 0; j < NUM_PIZZAS_INGRIDIENTS; ++j) 
      fprintf(stderr, "'%c' ", context->mem_table_space[i * NUM_PIZZAS_INGRIDIENTS +j]); 
    fprintf(stderr, "\n");
  }

  fprintf(stderr, "-----------------------------------------\n" RESET);
  safe_sem_post(context->sem_mutex);
}

static void ctor(int argc, char** argv, struct Context* context);
static void dtor(struct Context* context);
static void check_values(int num_tables, int num_chiefs, int num_couruiers);

static void launch_group(int num_people, struct Context* const context,
                         void (* func_ptr)(struct Context* const context,
                         int num_human));

static void chief(struct Context* const context, int num_human);
static void cook_pizza(const struct Context* context,
                       int num_table, int num_human);
static int find_table(const struct Context* context, enum condition cond);

static void courier(struct Context* const context, int num_human);
static void take_pizza(char* table, int num_human);
static bool all_pizzas_taken(enum condition* tables, int num_tables);

const char*  print_cond(enum condition cond);

//--------------------------------------------------------------
int main(int argc, char** argv) {
  struct Context context = {};
  ctor(argc, argv, &context); 

  launch_group(context.num_chiefs, &context, chief); 
  launch_group(context.num_couriers, &context, courier); 
  
  for (int i = 0; i < context.num_chiefs + context.num_couriers; ++i) wait(NULL);
  dtor(&context); 
}
//--------------------------------------------------------------
static void ctor(int argc, char** argv, struct Context* context) {
  check_args(argc, 4);

  int num_tables   = atoi(argv[1]);
  int num_chiefs   = atoi(argv[2]);
  int num_couriers = atoi(argv[3]);

  check_values(num_tables, num_chiefs, num_couriers);

  sem_unlink(NAME_EMPTY_SEM);
  sem_unlink(NAME_READY_SEM);
  sem_unlink(NAME_MUTEX);
  shm_unlink(NAME_TABLE_SPACE);
  shm_unlink(NAME_TABLE_CONTEXT);

  sem_t* sem_empty = safe_sem_open(NAME_EMPTY_SEM, O_CREAT | O_EXCL | O_RDWR, 0666, num_tables);
  sem_t* sem_ready = safe_sem_open(NAME_READY_SEM, O_CREAT | O_EXCL | O_RDWR, 0666, 0);
  sem_t* sem_mutex = safe_sem_open(NAME_MUTEX,     O_CREAT | O_EXCL | O_RDWR, 0666, 1);

  int shm_table_space = safe_shm_open(NAME_TABLE_SPACE, O_CREAT | O_EXCL | O_RDWR, 0666);
  size_t length_table_space = NUM_PIZZAS_INGRIDIENTS * num_tables;
  ftruncate(shm_table_space, length_table_space);

  int shm_table_context = safe_shm_open(NAME_TABLE_CONTEXT, O_CREAT | O_EXCL | O_RDWR, 0666);
  size_t length_table_context = sizeof(int) * (num_tables + 1);
  ftruncate(shm_table_context, length_table_context);

  char* mem_table_space = safe_mmap(NULL, length_table_space, PROT_READ | PROT_WRITE, MAP_SHARED, shm_table_space, 0);
  enum condition* mem_table_context = safe_mmap(NULL, length_table_context, PROT_READ | PROT_WRITE, MAP_SHARED, shm_table_context, 0);
  
  mem_table_context[num_tables] = num_chiefs;
 
  context->num_tables = num_tables;
  context->num_chiefs = num_chiefs;
  context->num_couriers = num_couriers;

  context->sem_empty = sem_empty;
  context->sem_ready = sem_ready;
  context->sem_mutex = sem_mutex;
  
  context->shm_table_space = shm_table_space;
  context->length_table_space = length_table_space;

  context->shm_table_context = shm_table_context;
  context->length_table_context = length_table_context;

  context->mem_table_space = mem_table_space;
  context->mem_table_context = mem_table_context;
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
static void dtor(struct Context* context) {
  safe_sem_close(context->sem_empty);
  safe_sem_close(context->sem_ready);
  safe_sem_close(context->sem_mutex);
  safe_sem_unlink(NAME_EMPTY_SEM);
  safe_sem_unlink(NAME_READY_SEM);
  safe_sem_unlink(NAME_MUTEX);

  safe_munmap(context->mem_table_space, context->length_table_space);
  safe_close(context->shm_table_space, NAME_TABLE_SPACE);
  safe_shm_unlink(NAME_TABLE_SPACE);

  safe_munmap(context->mem_table_context, context->length_table_context);
  safe_close(context->shm_table_context, NAME_TABLE_CONTEXT);
  safe_shm_unlink(NAME_TABLE_CONTEXT);
}
//--------------------------------------------------------------
static void launch_group(int num_people, struct Context* const context,
                         void (* func_ptr)(struct Context* const context,
                         int num_human)) {
for(int i = 1; i < num_people + 1; ++i) {
    pid_t pid = safe_fork();
    if(pid == 0) func_ptr(context, i);
  }
}
//--------------------------------------------------------------
static void chief(struct Context* const context, int num_human) {
  /* FIXME: debug */ DBG_PRINT(context, num_human, "chief", "Только родился");
  safe_sem_wait(context->sem_empty); 
  /* FIXME: debug */ DBG_PRINT(context, num_human, "chief", "Сделал --empty");
  
  safe_sem_wait(context->sem_mutex);  // mutex: on
  int num_table = find_table(context, EMPTY);
  context->mem_table_context[num_table] = TAKEN;
  safe_sem_post(context->sem_mutex);  //mutex: off
  /* FIXME: debug */ DBG_PRINT(context, num_human, "chief", "Нашёл стол (№%d) и изменил значение нужного стола на taken", num_table + 1);
  
  cook_pizza(context, num_table, num_human);
  /* FIXME: debug */ DBG_PRINT(context, num_human, "chief", "Приготовил на столе(№%d) пиццу", num_table + 1);


  safe_sem_wait(context->sem_mutex);  //mutex: on
  context->mem_table_context[num_table] = READY;
  safe_sem_post(context->sem_ready); 
  safe_sem_post(context->sem_mutex);  //mutex: off
  /* FIXME: debug */ DBG_PRINT(context, num_human, "chief", "поставил значение на столе (№%d) на READY и сделал ++ready", num_table + 1);
  
  safe_sem_wait(context->sem_mutex);  //mutex: on
  --context->mem_table_context[context->num_tables];
  safe_sem_post(context->sem_mutex);  //mutex: off
  /* FIXME: debug */ DBG_PRINT(context, num_human, "chief", "Уменьшил значение живых поваров");

  /* FIXME: debug */ DBG_PRINT(context, num_human, "chief", "Прям перед смертью");
  /* FIXME: debug */ fprintf(stderr, BLUE "Chief №%d exited" RESET "\n", num_human);

  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
static int find_table(const struct Context* context, enum condition cond) {
  enum condition* table_context = context->mem_table_context;
  for(int i = 0; i < context->num_tables; ++i)
    if(table_context[i] == cond)
      return i;
  return -1;
}
//--------------------------------------------------------------
static void cook_pizza(const struct Context* context,
                       int num_table, int num_human) {
  char* mem = context->mem_table_space;
  memcpy(mem + num_table * NUM_PIZZAS_INGRIDIENTS, "pizza", 5);
  
  // work immitation 
   printf(ORANGE "I'm a chief №%d and i cooked pizza!" RESET "\n", num_human);
  fflush(stdout);

  usleep(500000);
}
//--------------------------------------------------------------
static void courier(struct Context* const context, int num_human) {
  char* mem_table_space = context->mem_table_space;
  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", "Только что родился");

  safe_sem_wait(context->sem_mutex);  //mutex: on
  int current_num_chiefs = context->mem_table_context[context->num_tables];
  safe_sem_post(context->sem_mutex);  //mutex: off

  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", "Прям циклом while");
  while(current_num_chiefs != 0
        /*|| !all_pizzas_taken(context->mem_table_context, context->num_tables)*/) {
  
  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", "Жду перед ready-семафоромыы");
  safe_sem_wait(context->sem_ready);
  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", "Сделал --ready");

  
  safe_sem_wait(context->sem_mutex);  //mutex: on
  int num_table = find_table(context, READY);
  context->mem_table_context[num_table] = TAKEN;
  safe_sem_post(context->sem_mutex);  //mutex: off
  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", "Нашёл стол с готовой пиццей (№%d) и поставил туда значение taken", num_table);

  take_pizza(mem_table_space + num_table * NUM_PIZZAS_INGRIDIENTS, num_human); 
  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", " Взял готовую пиццу со стола (№%d)", num_table);
  

  safe_sem_wait(context->sem_mutex);  //mutex: on
  context->mem_table_context[num_table] = EMPTY;
  safe_sem_post(context->sem_empty);
  safe_sem_post(context->sem_mutex);  //mutex: off
  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", "Поставил значение стола опять на empty; иду снова на цикл");

  current_num_chiefs = context->mem_table_context[context->num_tables];
  

  //   /* FIXME */ fprintf(stderr, GREY "mem_shm_context_: %p\n", context->mem_table_context);
  // for(int i = 0; i < context->num_chiefs + 2; ++i) {
  //   fprintf(stderr, "%p ", context->mem_table_context + i);
  // }
  // fprintf(stderr, "\n");
  // for(int i = 0; i < context->num_chiefs + 2; ++i) {
  //   fprintf(stderr, "%-14d ", context->mem_table_context[i]);
  // }
  // fprintf(stderr, RESET "\n");

  }

  /* FIXME: debug */ DBG_PRINT(context, num_human, "courier", "Прямо перед смертью");

  /* FIXME: debug */ fprintf(stderr, RED "Courier №%d exited" RESET "\n", num_human);
  exit(EXIT_SUCCESS);
}
//--------------------------------------------------------------
static bool all_pizzas_taken(enum condition* tables, int num_tables) {
  for(int i = 0; i < num_tables; ++i) if(tables[i] != EMPTY) return false;
  return true;
}
//--------------------------------------------------------------
static void take_pizza(char* table, int num_human) {
  if(strncmp(table, "pizza", NUM_PIZZAS_INGRIDIENTS)) {
    fprintf(stderr, RED "Error: chief cooked not a pizza!!!\n" RESET);
    exit(EXIT_FAILURE);
  }
  // work immitation
   printf(YELLOW "I'm a courier №%d and i taked the pizza!" RESET "\n", num_human);
  fflush(stdout);
  
  usleep(500000);
}
//--------------------------------------------------------------
