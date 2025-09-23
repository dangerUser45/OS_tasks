#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct opt_flags
{
    bool verbose, interactive, force;
};

int pars_opt(int argc, char** argv, struct opt_flags* opt_flags);
void check_args(int argc);
bool is_directory(const char *path);
void clear_stdin();
int fd_write(int fd_src, int fd_dest, const char *filename_src,
             const char* filename_dest, char* buf);
ssize_t safe_write(int fd, const char* filename, char* buf, size_t n);
int safe_open(const char* file, int oflag, int mode);
int safe_close(int fd, const char* filename);
int copy_to_dir(int argc, char** argv, char* buf, struct opt_flags* opt_flags);
int copy_to_file(const char *filename_src, const char *filename_dest, char *buf,
                 struct opt_flags *opt_flags);

#define PAGE_SIZE 4096

//--------------------------------------------------------------
int main(int argc, char** argv)
{   
    struct opt_flags opt_flags = {};
    
    int error = pars_opt(argc, argv, &opt_flags);
    if(error == -1)
        exit(-1);

    check_args(argc);
    char buf[PAGE_SIZE] = {};

    if(argc - optind == 2)
    {
        if(!is_directory(argv[optind + 1]))
        {
            int error = copy_to_file(argv[optind],
            argv[optind + 1], buf, &opt_flags);
            if(error < 0)
                exit(-1);
        }
    }

    error = copy_to_dir(argc, argv, buf, &opt_flags);
    if(error < 0)
        exit(-1);
}
//--------------------------------------------------------------
int pars_opt(int argc, char** argv, struct opt_flags* opt_flags)
{
    struct option long_options[] =
    {
        {"verbose", no_argument,       0, 'v'},
        {"interactive", no_argument, 0, 'i'},
        {"force", no_argument, 0, 'f'},  
        {0, 0, 0, 0}
    };

    int opt = 0;
    bool verbose, interactive, force = false;

    while ((opt = getopt_long(argc, argv, "ihvf",
  long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'v':
            {
                opt_flags->verbose = true;
                break;
            }
            case 'i':
            {
                opt_flags->interactive = true;
                break;
            }
            case 'f':
            {
                opt_flags->force = true;
                break;
            }
            case '?':
            {
                fprintf(stderr, "Try 'my_cp --help' for more information.\n");
                return -1;
            }
        }
    }

    return 0;
}
//--------------------------------------------------------------
void check_args(int argc)
{
    if (argc - optind == 0)
    {
      fprintf(stderr, "my_cp: missing file operand\n"
                      "Try 'my_cp --help' for more information.\n");
      exit(-1);
    }
}
//--------------------------------------------------------------
bool is_directory(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) != 0)
        return false;

    return S_ISDIR(path_stat.st_mode);
}
//--------------------------------------------------------------
void clear_stdin()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
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
ssize_t safe_write(int fd, const char* filename, char* buf, size_t n)
{
    ssize_t num_sym = 0;

    while(true)
    {
        num_sym = write(fd, buf + num_sym, n);
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
int copy_to_file(const char *filename_src, const char *filename_dest, char *buf,
                 struct opt_flags *opt_flags)
{
    if (strcmp(filename_src, filename_dest) == 0) {
        fprintf(stderr, "my_cp: '%s' and '%s' are the same file\n", filename_src,
            filename_dest);
       return -1;
    }

    int fd_src = safe_open(filename_src, O_RDONLY, 0);
    if (fd_src < 0)
        return fd_src;

    int o_flags = O_WRONLY | O_CREAT | O_EXCL;
    if (opt_flags->interactive == true)
    {
        fprintf(stdout, "my_cp: overwrite '%s'?\n", filename_dest);
        if (tolower(getchar()) == 'y')
        {
            o_flags = O_WRONLY | O_CREAT | O_TRUNC;
            clear_stdin();
        }
        else
        {
            clear_stdin();
            return -1;
        }
    }

    if (opt_flags->force == true)
        o_flags = O_WRONLY | O_CREAT | O_TRUNC;

    int fd_dest = safe_open(filename_dest, o_flags, 0666);
    if (fd_dest < 0)
        return fd_dest;

    int error = fd_write(fd_src, fd_dest, filename_src, filename_dest, buf);
    if (error < 0)
        return error;

    error = safe_close(fd_dest, filename_dest);
    if (error < 0)
        return error;

    error = safe_close(fd_src, filename_src);
    if (error < 0)
        return error;

    if (opt_flags->verbose == true)
        fprintf(stderr, "'%s' -> '%s'\n", filename_src, filename_dest);

    return 0;
}
//--------------------------------------------------------------
int copy_to_dir(int argc,char** argv, char* buf, struct opt_flags* opt_flags)
{
    char* dir = argv[argc - 1];
    if(strchr(dir, '/') == 0)
        dir = strncat(dir, "/", 1);
    size_t dir_length = strlen(dir);

    int error = 0;
    for (int i = optind; i < argc - 1; ++i)
    {   
        dir[dir_length] = '\0';
        const char* new_file_name = strncat(dir, argv[i], strlen(argv[i]));
        copy_to_file(argv[i], argv[argc - 1], buf, opt_flags);
    }

    return error;
}
//--------------------------------------------------------------
