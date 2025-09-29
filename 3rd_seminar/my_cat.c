#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int fd_write (int src, char* buf, int dest);
ssize_t safe_write(int fd, char* buffer, size_t n);

#define PAGE_SIZE 4096

//--------------------------------------------------------------
int main(int argc, char** argv)
{   
    char buf[PAGE_SIZE] = {};
    
    if(argc == 1)
        fd_write(0, buf, 1);

    for(int i = 1; i < argc; ++i)
    {
        int fd = open(argv[i], O_RDONLY);
        if(fd < 0)
        {
            fprintf(stderr, "%s: %s\n",argv[i], strerror(errno));
            continue;
        }
        
        int error = fd_write(fd, buf, 1);
        if(error < 0)
        {
            fprintf(stderr, "%s: %s\n", argv[i], strerror(errno));
        }

        error = close(fd);
        if(error < 0)
        {
            fprintf(stderr, "%s: %s\n", argv[i], strerror(errno));
            continue;
        }
    }
}
//--------------------------------------------------------------
int fd_write (int src, char* buf, int dest)
{
    while(true)
        {
            ssize_t num_sym = read(src, buf, PAGE_SIZE);
            if (num_sym < 0)
            {
                return num_sym;
            }
            else if(num_sym > 0)
            {
                int error = safe_write(dest, buf, num_sym);
                if(error < 0)
                    return error;

                continue;
            }
            else break;
        }
    
    return 0;
}
//--------------------------------------------------------------
ssize_t safe_write(int fd, char* buf, size_t n)
{
    ssize_t num_sym = 0;

    while(true)
    {
        buf += num_sym; n -= num_sym;
        num_sym = write(fd, buf, n);
        if(num_sym < 0)
        {
            if(errno != EINTR)
                return num_sym;
            
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
