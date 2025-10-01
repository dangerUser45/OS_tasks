#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int num_iter = 0;
    if (argc == 1)
        num_iter = 10;

    else
        num_iter = atoi(argv[1]);

    //setvbuf(stdout, NULL, 0, 0);
    for(int i = 0; i < num_iter; ++i)
    {
        pid_t pid = fork();
        if (pid != 0)
        {
            printf("I am parent proccess, my pid = %d\n", getpid());
            int status = 0;
            wait(&status);
        }

        else
        {
            printf("I am child  process,  my pid = %d, pid of my parent = %d\n", getpid(), getppid());
        }
    }

    for (int i = 0; i < num_iter; i++)
        exit(0);               
}
