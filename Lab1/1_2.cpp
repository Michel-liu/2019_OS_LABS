#include<stdio.h>
#include<unistd.h>
int main(){
    pid_t pid_1,pid_2;
    asm volatile(
        "mov $20,%%eax\n\t"
        "int $0x80\n\t"
        "mov %%eax,%0\n\t"
        :"=m" (pid_1)
        );
    printf("%d\n",pid_1);
	pid_2 = getpid();
	printf("%d\n",pid_2);
    return 0;
}
