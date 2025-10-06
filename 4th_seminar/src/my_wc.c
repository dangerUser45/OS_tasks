#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct wc_stat {
  ssize_t bytes_counter;
  ssize_t words_counter;
  ssize_t lines_counter;
};

struct word_state_t {
  char prev_char;
  bool in_word;
};

int safe_open(const char* file, int oflag, int mode);
int safe_close(int fd, const char* filename);
int fd_write(int fd_src, int fd_dest, const char *filename_src,
             const char* filename_dest, char* buf, struct wc_stat* stat);
ssize_t safe_write(int fd, const char* filename, char* buf, ssize_t n);

void find_lines(ssize_t* counter, const char* str);
void find_words(ssize_t* counter, const char* str, struct word_state_t* state);

#define PAGE_SIZE 1

//--------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc == 1) {
    fprintf(stderr, "main: too few arguments\n");
    return -1;
  }
  
  char buf[PAGE_SIZE] = {};
  struct wc_stat stat = {};
  int fds[2] = {}; // pipe[0] - end, pipe[1] - start
  pipe(fds);

  pid_t pid = fork();

  if(pid != 0) // parent pid - only reader
  {
      safe_close(fds[1], "pipe input");
      fd_write(fds[0], 1,
  "pipe output", "pipe_input", buf, &stat);
      int status = 0;
      wait(&status);
      fprintf(stdout, "% ld %ld %ld %s\n",
              stat.lines_counter, stat.words_counter,
              stat.bytes_counter, argv[2]);
  }
  else //child pid - only writer
  {
      safe_close(fds[0], "pipe output");
      dup2(fds[1], 1);
      close(fds[1]);
      execvp( argv[1], &(argv[1]));
  }
}
//--------------------------------------------------------------
int safe_open(const char* file, int oflag, int mode)
{
    int fd = 0;
    if(mode == 0)
        fd = open(file, oflag);
    else
        fd = open(file, oflag, mode);
    
    if(fd < 0)
        fprintf(stderr, "%s: %s\n", file, strerror(errno));
    
    return fd;
}
//--------------------------------------------------------------
int safe_close(int fd, const char* filename)
{
    int error = close(fd);
    if(error < 0)
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));

    return error;
}
//--------------------------------------------------------------
int fd_write(int fd_src, int fd_dest, const char* filename_src,
             const char *filename_dest, char *buf, struct wc_stat* stat) {
  struct word_state_t state = {' ', 0};
  while(true)
  {
    ssize_t num_sym = 0;
    num_sym = read(fd_src, buf, PAGE_SIZE);
    
    if (num_sym < 0)
    {
        fprintf(stderr, "%s: %s\n", filename_src, strerror(errno));
        return num_sym;
    }
    else if(num_sym > 0)
    {
        int error = safe_write(fd_dest, filename_dest, buf, num_sym);
        if(error < 0)
            return error;
        find_words(&(stat->words_counter), buf, &state);
        find_lines(&(stat->lines_counter), buf);
        stat->bytes_counter += num_sym;
        continue;
    }
    else break;
  }
    return 0;
}
//--------------------------------------------------------------
ssize_t safe_write(int fd, const char* filename, char* buf, ssize_t n)
{
    ssize_t num_sym = 0;

    while(true)
    {
        buf += num_sym; n -= num_sym;
        num_sym = write(fd, buf, n);
        if(num_sym < 0)
        {
            if(errno != EINTR)
            {
                fprintf(stderr, "%s: %s\n", filename, strerror(errno));
                return num_sym;
            }
            else
                continue;
        }

        else if(num_sym == n)
            return num_sym;
        else if(num_sym == 0)
            break;
        else
            continue;
    }

    return 0;
}
//--------------------------------------------------------------
void find_lines(ssize_t* counter, const char* str) {
    const char* ptr = str;
    while((ptr = strchr(ptr, '\n')) != NULL) {
        ++(*counter);
        ++ptr;
    }
}
//--------------------------------------------------------------
void find_words(ssize_t* counter, const char* str, struct word_state_t* state) {
  if (str == NULL || *str == '\0') {
    return;
  }

  for (size_t i = 0; str[i] != '\0'; i++) {
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
