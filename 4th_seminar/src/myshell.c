#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "safe_lib.h"
#include "parser.h"

static void die(const char *fmt, ...);
static char *join_args(int argc, char **argv);
static ssize_t read_line(const char *prompt, char **line, size_t *cap);
static int append_last_command_from_stdin(struct Pipeline *pl);
static int run_pipeline(struct Pipeline *pl);
static int run_one_line(const char *line);

//--------------------------------------------------------------
int main(int argc, char **argv) {
  if (argc > 1) {
    char *line = join_args(argc, argv);
    if (!line) die("oom");
    int rc = run_one_line(line);
    free(line);
    return rc;
  }
}
//--------------------------------------------------------------
static void die(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}
//--------------------------------------------------------------
static char *join_args(int argc, char **argv) {
  size_t total = 0;
  for (int i = 1; i < argc; ++i) total += strlen(argv[i]) + 1;
  char *s = calloc(total + 1, sizeof(char));
  if (!s) return NULL;
  s[0] = '\0';
  for (int i = 1; i < argc; ++i) {
    strcat(s, argv[i]);
    if (i + 1 < argc) strcat(s, " ");
  }
  return s;
}
//--------------------------------------------------------------
static ssize_t read_line(const char *prompt, char **line, size_t *cap) {
  if (prompt) {
    fputs(prompt, stdout);
    fflush(stdout);
  }
  ssize_t n = getline(line, cap, stdin);
  if (n > 0 && (*line)[n - 1] == '\n') (*line)[n - 1] = '\0';
  return n;
}
//--------------------------------------------------------------
static int append_last_command_from_stdin(struct Pipeline *pl) {
  char *line = NULL;
  size_t cap = 0;
  
  if (read_line("pipe> ", &line, &cap) <= 0) {
    free(line); return -1;
  }
  struct Pipeline tail = (struct Pipeline){0};
  int rc = parse_pipeline_simple(line, &tail);
  free(line);
  
  if (rc != 0) {
    free_pipeline(&tail);
    return -1; 
  }
  if (tail.trailing_stdin) {
    free_pipeline(&tail);
    return -1;
  }
  if (tail.ncmds != 1) {
    free_pipeline(&tail);
    return -1;
  }
  if (tail.in_path) {
    free_pipeline(&tail);
    return -1;
  }
    
  struct Cmd last = tail.cmds[0];
  tail.cmds[0].argv = NULL;
  tail.cmds[0].argc = 0;
  struct Cmd *newcmds = realloc(pl->cmds,
                               sizeof(struct Cmd) * (pl->ncmds + 1));
  
  if (!newcmds) {
    tail.cmds[0] = last;
    free_pipeline(&tail);
    return -1;
  }
  
  pl->cmds = newcmds;
  pl->cmds[pl->ncmds] = last;
  pl->ncmds += 1;
  
  if (pl->out_path)
    free(pl->out_path);
  
  pl->out_path = tail.out_path;
  tail.out_path = NULL;
  pl->trailing_stdin = 0;
  free_pipeline(&tail);
  return 0;
}
//--------------------------------------------------------------
static int run_pipeline(struct Pipeline *pl) {
  if (!pl || pl->ncmds <= 0) return 0;
  int n = pl->ncmds;
  int (*pipes)[2] = NULL;
  if (n > 1) {
    pipes = calloc(n - 1, sizeof(int[2]));
    if (!pipes) { perror("calloc"); return 1; }
    for (int i = 0; i < n - 1; ++i) {
      if (safe_pipe(pipes[i]) < 0) {
        for (int k = 0; k < i; ++k) {
          safe_close(pipes[k][0], "pipe");
          safe_close(pipes[k][1], "pipe");
        }
        free(pipes);
        return 1;
      }
    }
  }
  pid_t *pids = calloc(n, sizeof(pid_t));
  if (!pids) {
    perror("calloc");
    if (pipes) {
      for (int i = 0; i < n - 1; ++i) {
        safe_close(pipes[i][0], "pipe");
        safe_close(pipes[i][1], "pipe");
      }
    }
    free(pipes);
    return 1;
  }
  signal(SIGPIPE, SIG_IGN);
  for (int i = 0; i < n; ++i) {
    pid_t pid = safe_fork();
    if (pid == 0) {
      if (i == 0) {
        if (pl->in_path) {
          int fd = safe_open(pl->in_path, O_RDONLY, 0);
          if (safe_dup2(fd, STDIN_FILENO) < 0) exit(-1);
          safe_close(fd, pl->in_path);
        }
      } else {
        if (safe_dup2(pipes[i - 1][0], STDIN_FILENO) < 0)
          exit(-1);
      }
      if (i == n - 1) {
        if (pl->out_path) {
          int fd = safe_open(pl->out_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
          if (safe_dup2(fd, STDOUT_FILENO) < 0) exit(-1);
          safe_close(fd, pl->out_path);
        }
      } else {
        if (safe_dup2(pipes[i][1], STDOUT_FILENO) < 0) exit(-1);
      }
      if (pipes) {
        for (int k = 0; k < n - 1; ++k) {
          safe_close(pipes[k][0], "pipe");
          safe_close(pipes[k][1], "pipe");
        }
      }
      execvp(pl->cmds[i].argv[0], pl->cmds[i].argv);
      perror(pl->cmds[i].argv[0]);
      _exit(127);
    }
    pids[i] = pid;
    if (i > 0) safe_close(pipes[i - 1][0], "pipe");
    if (i < n - 1) safe_close(pipes[i][1], "pipe");
  }
  free(pipes);
  int status = 0;
  pid_t last_pid = pids[n - 1];
  int exit_code = 0;
  for (int i = 0; i < n; ++i) {
    pid_t w = waitpid(pids[i], &status, 0);
    if (w == last_pid) {
      if (WIFEXITED(status)) exit_code = WEXITSTATUS(status);
      else if (WIFSIGNALED(status)) exit_code = 128 + WTERMSIG(status);
    }
  }
  free(pids);
  return exit_code;
}
//--------------------------------------------------------------
static int run_one_line(const char *line) {
  struct Pipeline pl = (struct Pipeline){0};
  int rc = parse_pipeline_simple(line, &pl);
  if (rc != 0) {
    fprintf(stderr, "parse error: %d\n", rc);
    return 1;
  }
  if (pl.trailing_stdin) {
    if (append_last_command_from_stdin(&pl) != 0) {
      fprintf(stderr, "incomplete pipeline\n");
      free_pipeline(&pl);
      return 1;
    }
  }
  int ec = run_pipeline(&pl);
  free_pipeline(&pl);
  return ec;
}
//--------------------------------------------------------------
