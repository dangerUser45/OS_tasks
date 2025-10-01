#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        perror("Error: too few arguments\n");
        return -1;
    }

    for (int i = 1; i < argc + 1; ++i)
    {
        int p = fork();
        if(p == 0)
        {
            int value = atoi(argv[i]);
            usleep(value * 1000);
            printf("%d ", value);
            exit(0);
        }
    }
}

