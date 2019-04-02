#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sched.h>

sem_t* read_mySem = NULL;
sem_t* print_mySem = NULL;

char buf[10];
short i = 0;
short j = 0;

void*read_char(){
    while(1){
        sem_wait(read_mySem);
        char *t = &buf[i++%10];
        scanf("%c",t);
        sem_post(print_mySem);
        if(*t == '\n') break;
    }
}

void*print_char(){
    while(1){
        sem_wait(print_mySem);
        char *t = &buf[j++%10];
        if(*t == '\n') break;
        printf("%d:%c\n",j,*t);
        sem_post(read_mySem);
    }
}

int main(){
    pthread_t p1,p2;

    read_mySem = sem_open("read_chars", O_CREAT, 0666, 10);
    print_mySem = sem_open("print_chars", O_CREAT, 0666, 0);

    pthread_create(&p1,NULL,read_char,NULL);
    pthread_create(&p2,NULL,print_char,NULL);

    pthread_join(p1,NULL);
    pthread_join(p2,NULL);

    sem_close(read_mySem);
    sem_close(print_mySem);

    sem_unlink("read_chars");
    sem_unlink("print_chars");
    return 0;
}