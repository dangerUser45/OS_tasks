#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "safe_lib.h"

static void handler();

const char* const string = "Goodbye!\n";

//--------------------------------------------------------------
int main() {
  signal(SIGINT, handler);
  printf("Hello World!\n");

  while(true){ pause(); }
}
//--------------------------------------------------------------
static void handler(){
  safe_write(STDOUT_FILENO, "stdout", (char*)string, 9);
  _exit(0);
}
//--------------------------------------------------------------
