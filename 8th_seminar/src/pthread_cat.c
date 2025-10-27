#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "safe_lib.h"

struct Monitor {
  pthread_mutex_t monitor_mutex;
};

struct ThreadsContext {
  struct Monitor monitor;
  char* buf;
};

void* reader(void* arg);
void* writer(void* arg);

#define PAGE_SIZE 4096
#define BUF_SIZE 16*PAGE_SIZE

//--------------------------------------------------------------
int main(int argc, char** argv) {
  struct ThreadsContext context;
  
  int file_des = safe_open(argv[0], O_RDONLY, 0);

  char* buf = (char*)calloc(BUF_SIZE, sizeof(char));
  if(buf == NULL) { perror("calloc"); exit(-1); }

  pthread_mutex_t mutex = {};
  int error = pthread_mutex_init(&mutex, NULL);

  struct Monitor monitor = {.monitor_mutex = mutex}; 
  context

  pthread_t reader_thread = 0;
  error = pthread_create(&reader_thread, NULL,
              reader, buf);
  if(error != 0) { perror("pthread_create"); exit(-1); }

  pthread_t writer_thread = 0;
  error = pthread_create(&writer_thread, NULL,
              writer, buf);
  if(error != 0) { perror("pthread_create"); exit(-1); }

  
  pthread_join(reader_thread, NULL);
  pthread_join(writer_thread, NULL);

  error = pthread_mutex_destroy(&mutex);
  
  free(buf);

  safe_close(file_des, "file which opened to be printed");
}
//--------------------------------------------------------------
void* reader(void* arg) {
  printf("Hello World\n");

  safe_write();

  return  NULL;
}
//--------------------------------------------------------------
void* writer(void* arg) {

  return NULL;
}
//--------------------------------------------------------------
void* mon_get_read(pthread_mutex_t* mutex) {
  pthread_mutex_lock(mutex);

  pthread_mutex_unlock(mutex);
}
//--------------------------------------------------------------
