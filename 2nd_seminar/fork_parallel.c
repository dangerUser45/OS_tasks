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
        
        if(pid != 0)
        {
            printf("%3d) I am parent proccess, my pid = %d\n", i + 1, getpid());
            int status = 0;
            wait(&status);
        }        
        
        else
        {
            printf("     I am child  proccess, my pid = %d\n\n", getpid());
            exit(0);
        }
    }

    // int status = 0;
    // for (int i = 0; i < num_iter; ++i)
    //     wait(&status);
}
