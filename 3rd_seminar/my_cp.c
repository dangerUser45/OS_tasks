#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

struct opt_flags
{
    bool verbose, interactive, force;
};

int fd_write (int src, char* buf, int dest);
ssize_t safe_write(int fd, char* buffer, size_t n);
int safe_open(const char* file, int oflag, int mode);
int safe_close(int fd, const char* filename);
int copy_to_dir(int argc,char** argv, char* buf, struct opt_flags* opt_flags);
int copy_to_file(int argc, char** argv, char* buf, struct opt_flags* opt_flags);

//TODO добавить возможность не указывать '/' в название директории при копировании в эту директорию

#define PAGE_SIZE 4096

//--------------------------------------------------------------
int main(int argc, char** argv)
{   
    if(argc == 1)
    {
        fprintf(stderr, "cp: missing file operand\nTry 'cp --help' for more information.\n");
        exit(-1);
    }

    struct option long_options[] =
    {
        {"verbose", no_argument,       0, 'v'},
        {"interactive", no_argument, 0, 'i'},
        {"force", no_argument, 0, 'f'},
        {"help",    no_argument,       0, 'h'},  
        {0, 0, 0, 0}
    };

    struct opt_flags* opt_flags = (struct opt_flags*) calloc(1, sizeof(struct opt_flags));
    int opt = 0;
    bool verbose, interactive, force = false;

    while ((opt = getopt_long(argc, argv, "vifh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v': 
            {
                opt_flags->verbose = true;
                /*FIXME */ fprintf(stderr, "Flag verbose is enable\n");
                break;
            }
            case 'i':
            {
                opt_flags->interactive = true;
                /*FIXME */ fprintf(stderr, "Flag interactive is enable\n");
                break;
            }
            case 'f':
            {
                opt_flags->force = true;
                /*FIXME */ fprintf(stderr, "Flag force is enable\n");
                break;
            }
            case 'h':
            {
                printf(""); //TODO
                /*FIXME */ fprintf(stderr, "Flag help is enable\n");
                return 0;
            }
            case '?':
            {
                printf(""); //TODO
                /*FIXME */ fprintf(stderr, "Unknown flag\n");
                return -1;
            }
        }
    }

    char buf[PAGE_SIZE] = {};

    if(argc == 3)
        copy_to_file(argc, argv, buf, opt_flags);

    else
        copy_to_dir(argc, argv, buf, opt_flags);

    free(opt_flags);
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
    {
        fprintf(stderr, "%s: %s\n", file, strerror(errno));
    }

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
        num_sym = write(fd, buf + num_sym, n);
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
int copy_to_file(int argc, char** argv, char* buf, struct opt_flags* opt_flags)
{
    int fd_src = safe_open(argv[optind], O_RDONLY, 0);
    if(fd_src < 0)
        // TODO add processing this error
        ;
    
    int o_flags = O_WRONLY | O_EXCL;
    if(opt_flags->interactive == true)
        o_flags |= O_EXCL;
    else
        ;

    int fd_dest = safe_open(argv[optind +1], o_flags, 0);
    if(fd_src < 0)
        // TODO add processing this error
        ;

    int error = fd_write(fd_src, buf, fd_dest);
    if(error < 0)
        fprintf(stderr, "%s\n", strerror(errno));
    
    error = safe_close(fd_dest, argv[optind + 1]);
    if(error < 0)
        //TODO processing this error
            ;

    error = safe_close(fd_src, argv[optind]);
    if(error < 0)
        //TODO processing this error
            ;
    
    return 0;
}
//--------------------------------------------------------------
int copy_to_dir(int argc,char** argv, char* buf, struct opt_flags* opt_flags)
{
    char* dir = argv[argc - 1];
    size_t dir_length = strlen(dir);
    fprintf(stderr, "length of dir = %zu\n", dir_length);

    int error = 0;
    for (int i = optind; i < argc - 1; ++i)
    {   
        dir[dir_length] = '\0';
        const char* new_file_name = strncat(dir, argv[i], strlen(argv[i]));

        //FIXME
        fprintf(stderr, "new_file_name = %s\n", new_file_name);

        int fd_dest = safe_open(dir, O_WRONLY | O_CREAT, 0666);
        if(fd_dest < 0)
            exit(-1);

        int fd_src = safe_open(argv[i], O_RDONLY, 0);
        if(fd_src < 0)
            continue;
        
        error = fd_write(fd_src, buf, fd_dest);
        if(error < 0)
        {
            fprintf(stderr, "%s\n", strerror(errno));
        }

        error = safe_close(fd_src, argv[i]);
        if(error < 0)
            //TODO processing this error
            ;
    
        error = safe_close(fd_dest, dir);
        if(error < 0)
            //TODO processing this error
            ;
    }

    return error;
}
//--------------------------------------------------------------
