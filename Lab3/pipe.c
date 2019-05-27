#include <stdio.h>
#include <unistd.h>     //for pipe()
#include <string.h>     //for memset()
#include <stdlib.h>     //for exit()

int main()
{
    int filedes[2];
    char buf[80];
    pid_t pid_1;
    if(-1 == pipe(filedes))
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_1 = fork();
    if(pid_1 > 0){
        // father thread
        sleep(2);
        printf("This is the father thread.\n");
        char s1[] = "Hello, this is written by father.\n";
        write(filedes[1],s1,sizeof(s1));
        close(filedes[0]);
        close(filedes[1]);
    } else if(pid_1 == 0)
    {
        // child thread
        printf("This is the child thread.\n");
        read(filedes[0],buf,sizeof(buf));
        printf("%s\n",buf);
        close(filedes[0]);
        close(filedes[1]);
    }
    return 0;
}