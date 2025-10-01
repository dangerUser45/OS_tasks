#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int safe_open(const char* file, int oflag, int mode);
int safe_close(int fd, const char* filename);
int fd_write(int fd_src, int fd_dest, const char *filename_src,
             const char* filename_dest, char* buf);
ssize_t safe_write(int fd, const char* filename, char* buf, ssize_t n);

#define PAGE_SIZE 4096

//--------------------------------------------------------------
int main(int argc, char** argv)
{
    char buf[PAGE_SIZE] = {};
    int fds[2] = {}; // pipe[0] - end, pipe[1] - start
    pipe(fds);

    pid_t pid = fork();

    if(pid != 0) // parent pid - only reader
    {
        safe_close(fds[1], "pipe input");
        int status = 0;
        fd_write(fds[0], 1,
            "pipe output", "pipe_input", buf);
        wait(&status);
    }
    else //child pid - only writer
    {
        safe_close(fds[0], "pipe output");

        for(int i = 1; i < argc; ++i)
        {
            int fd_src = safe_open(argv[i], O_RDONLY, 0);
            if (fd_src < 0)
                continue;

            fd_write(fd_src, fds[1], "stdin", "pipe input", buf);

            int error = safe_close(fd_src, argv[i]);
            if(error < 0)
              continue;       
        }
    }
}
//--------------------------------------------------------------
int safe_open(const char* file, int oflag, int mode)
{
    int fd = 0;
    if(mode == 0)
        fd = open(file, oflag);
    else
        fd =  open(file, oflag, mode);
    
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
             const char *filename_dest, char *buf)
{
    while(true)
        {
            ssize_t num_sym = read(fd_src, buf, PAGE_SIZE);
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
