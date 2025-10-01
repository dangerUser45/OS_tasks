#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../lib/include/safe_lib.h"

/* FIXME */
#include <stdio.h>

struct ShellContext {
  char** parsed_argv;
  int num_pipes;
};

char** parse_argv(int argc, char** argv);

//--------------------------------------------------------------
int main(int argc, char** argv) {

  struct ShellContext shell_context = {};
  char** parsed_argv = parse_argv(argc, argv);

  int pipedes[2];
  safe_pipe(pipedes);

  pid_t pid_left = safe_fork();

  if (pid_left == 0) { //stdout -> pipedes[1]
    safe_close(pipedes[0], "pipe output");
    safe_dup2(pipedes[1], STDOUT_FILENO);
    safe_close(pipedes[1], "pipe input(origin)");

    argv[pipeline] = NULL;
    execvp(argv[1], &argv[1]);
  }

  pid_t pid_right = safe_fork();
  if (pid_right == 0) { // pipedes[0] -> stdin 
    safe_close(pipedes[1], "pipe input");
    safe_dup2(pipedes[0], STDIN_FILENO);
    safe_close(pipedes[0], "pipe output(origin)");

    execvp(argv[pipeline + 1], &argv[pipeline + 1]);
  }

  safe_close(pipedes[0], "pipedes[0]");
  safe_close(pipedes[1], "pipedes[1]");

  int status1 = 0, status2 = 0;
  waitpid(pid_left, &status1, 0);
  waitpid(pid_right, &status2, 0);
}
//--------------------------------------------------------------
char** parse_argv(int argc, char** argv) {
  int num_pipes = 0;

  for(int i = 1; i < argc; ++i)
    if(strncmp(argv[i], "|", 1) == 0) {
      argv[i] = NULL;
      if(i == argc - 1) {
        fprintf(stderr,
          "parse_argv: Uncorrected form of use: ... arg | (empty) ...\n"
                "Add another argument after pipeline");
      }
      else ++num_pipes;
    }

  char** start_arg_array = 0;
  if(num_pipes != 0) {
    start_arg_array = (char**) calloc(num_pipes, sizeof(char**));
  
    for(int i = 1, idx_arr = 0; i < argc; ++i)
      if(strncmp(argv[i], "|", 1) == 0) {
        start_arg_array[idx_arr] = argv[i];
        ++idx_arr; 
    }
  }
  return start_arg_array;
}
//--------------------------------------------------------------
