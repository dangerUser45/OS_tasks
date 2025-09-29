#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    struct timeval start, end;
    gettimeofday(&start, 0);
    
    int pid = fork();
    if(pid == 0)
    {        
        execvp( argv[1], &(argv[1]));
        perror ("error");
        exit(0);
    }
    
    int status = 0;
    wait(&status);
    gettimeofday(&end, 0);

    double elapsed_time = (double)(end.tv_usec - start.tv_usec + (1000000)*(end.tv_sec - start.tv_sec));

    printf("Elapsed_time = %lf microseconds\n", elapsed_time);
}
