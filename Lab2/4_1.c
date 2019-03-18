#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>

int main(){
    pid_t pid_1,pid_root;
    pid_root = getpid();
    for(int i = 0; i<= 1; i++){
        pid_1 = fork();
        if (i==0) {
            if (pid_1 > 0){
                // root father
                printf("The root father %d create the child1 %d\n",pid_root,pid_1);
            } else if (pid_1 == 0)
            {
                // child 1
                printf("hello, I'm the child1. And I will create the child 3 soon!\n");
            } else
            {
                // error
                perror("fork!\n");
            }
        } else
        {
            if (pid_1 > 0)
            {
                if(getpid() == pid_root)
                {
                    // root father
                    printf("The root father %d create the child2 %d\n",pid_root,pid_1);
                } else
                {
                    printf("The child1 %d create the child3 %d\n",getpid(),pid_1);
                    pid_1 = fork();
                    if(pid_1 == 0){
                        // child 4
                        printf("Hello, I'm child4, I finish my task.\n");
                    } else
                    {
                        // child 1
                        printf("The child1 %d create the child4 %d\n",getpid(),pid_1);
                    }                
                }
            } else if (pid_1 == 0)
            {
                if(getppid()!=pid_root){
                    // child 3
                    printf("Hello, I'm child3, I finish my task.\n");
                } else
                {
                    // child 2
                    printf("Hello, I'm child2, I finish my task.\n");
                }
            } else
            {
                // error
                perror("fork!");
            }
        }
        
    }
    while(1){
		sleep(3);
		printf("(%d):My father pid is %d\n",getpid(),getppid());
	}
    
}
