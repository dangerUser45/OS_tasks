#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "safe_lib.h"

struct wc_stat {
  ssize_t bytes_counter;
  ssize_t words_counter;
  ssize_t lines_counter;
};

struct word_state_t {
  char prev_char;
  bool in_word;
};

#define PAGE_SIZE 4096

static void find_lines(ssize_t* counter, const char* str, ssize_t len);
static void find_words(ssize_t* counter, const char* str, ssize_t len, struct word_state_t* state);


//--------------------------------------------------------------
int main(int argc, char** argv) {
  // check_args(argc, 3);
  setbuf(stdout, NULL);

  char buf[4096] = {};
  struct wc_stat stat_stdout = {};
  struct wc_stat stat_stderr = {};

  struct word_state_t word_state_stdout = { ' ', false };
  struct word_state_t word_state_stderr = { ' ', false };

  int pipe_stdout[2] = {}; // pipe[0] - end, pipe[1] - start
  int pipe_stderr[2] = {}; // pipe[0] - end, pipe[1] - start

  pipe(pipe_stdout);
  pipe(pipe_stderr);

  pid_t pid = fork();

  if(pid == 0) { // child - writer
    safe_close(pipe_stdout[0], "pipe output");
    dup2(pipe_stdout[1], STDOUT_FILENO);
    close(pipe_stdout[1]);

    safe_close(pipe_stderr[0], "pipe_stdout output");
    dup2(pipe_stderr[1], STDERR_FILENO);
    close(pipe_stderr[1]);

    execvp( argv[1], &(argv[1]));
  }
  
  //parent - reader
  // safe_close(pipe_stdout[1], "pipe_stdout input");
  // fd_stat_write(pipe_stdout[0], STDOUT_FILENO, "pipe output", "parent's stdout", buf, &stat_stdout);

  // safe_close(pipe_stderr[1], "pipe_stderr input");
  // fd_stat_write(pipe_stderr[0], STDERR_FILENO, "pipe output", "parent's stderr", buf, &stat_stderr);

  // wait(NULL);
  // printf("From stdout:\n"
  //   "Bytes = %ld, Words = %ld, Lines = %ld\n",
  //   stat_stdout.bytes_counter, stat_stdout.words_counter, stat_stdout.lines_counter);
  // printf("From stderr:\n"
  //   "Bytes = %ld, Words = %ld, Lines = %ld\n",
  //   stat_stderr.bytes_counter, stat_stderr.words_counter, stat_stderr.lines_counter);

  // parent - reader
  safe_close(pipe_stdout[1], "pipe_stdout write");
  safe_close(pipe_stderr[1], "pipe_stderr write");

  bool stdout_open = true;
  bool stderr_open = true;

  int maxfd = pipe_stdout[0] > pipe_stderr[0] ? pipe_stdout[0] : pipe_stderr[0];

  while (stdout_open || stderr_open) {
    fd_set readfds;
    FD_ZERO(&readfds);

    if (stdout_open)
      FD_SET(pipe_stdout[0], &readfds);
    if (stderr_open)
      FD_SET(pipe_stderr[0], &readfds);

    int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (ready < 0) {
      if (errno == EINTR)
        continue;
      perror("select");
      break;
    }

    // stdout pipe ready
    if (stdout_open && FD_ISSET(pipe_stdout[0], &readfds)) {
      ssize_t n = read(pipe_stdout[0], buf, PAGE_SIZE);
      if (n < 0) {
        fprintf(stderr, "pipe_stdout read: %s\n", strerror(errno));
        stdout_open = false;
        safe_close(pipe_stdout[0], "pipe_stdout read");
      } else if (n == 0) {
        stdout_open = false;
        safe_close(pipe_stdout[0], "pipe_stdout read");
      } else {
        // int err = safe_write(STDOUT_FILENO, "parent's stdout", buf, n);
        // if (err < 0) {
        // }
        find_words(&stat_stdout.words_counter, buf, n, &word_state_stdout);
        find_lines(&stat_stdout.lines_counter, buf, n);
        stat_stdout.bytes_counter += n;
      }
    }

    // stderr pipe ready
    if (stderr_open && FD_ISSET(pipe_stderr[0], &readfds)) {
      ssize_t n = read(pipe_stderr[0], buf, PAGE_SIZE);
      if (n < 0) {
        fprintf(stderr, "pipe_stderr read: %s\n", strerror(errno));
        stderr_open = false;
        safe_close(pipe_stderr[0], "pipe_stderr read");
      }
      else if (n == 0) {
        stderr_open = false;
        safe_close(pipe_stderr[0], "pipe_stderr read");
      }
      else {
        // int err = safe_write(STDERR_FILENO, "parent's stderr", buf, n);
        // if (err < 0) {
        // }
        find_words(&stat_stderr.words_counter, buf, n, &word_state_stderr);
        find_lines(&stat_stderr.lines_counter, buf, n);
        stat_stderr.bytes_counter += n;
      }
    }
  }

  wait(NULL);

  printf("From stdout:\n"
         "Bytes = %ld, Words = %ld, Lines = %ld\n",
         stat_stdout.bytes_counter, stat_stdout.words_counter, stat_stdout.lines_counter);
  printf("From stderr:\n"
         "Bytes = %ld, Words = %ld, Lines = %ld\n",
         stat_stderr.bytes_counter, stat_stderr.words_counter, stat_stderr.lines_counter);
}
//--------------------------------------------------------------
static void find_lines(ssize_t* counter, const char* str, ssize_t len) {
  for (ssize_t i = 0; i < len; ++i) {
    if (str[i] == '\n')
      ++(*counter);
  }
}
//--------------------------------------------------------------
static void find_words(ssize_t* counter, const char* str, ssize_t len,
                       struct word_state_t* state) {
  if (str == NULL || len <= 0) {
    return;
  }

  for (ssize_t i = 0; i < len; ++i) {
    char current_char = str[i];

    if (isspace((unsigned char)current_char)) {
      if (state->in_word) {
        state->in_word = false;
      }
    } else {
      if (!state->in_word) {
        state->in_word = true;
        (*counter)++;
      }
    }
    state->prev_char = current_char;
  }
}
//--------------------------------------------------------------
