#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sched.h>

sem_t* mySem = NULL;
int tickets = 1000;
void*sold(){
    int i = 100;
    while(i--){
        // sem_wait(mySem);
        printf("Current tickes number is %d.\n",tickets);
        int temp = tickets;
        // sched_yield();
        temp = temp - 1;
        // sched_yield();
        tickets = temp;
        // sem_post(mySem);
    }
}

void*returnT(){
    int i = 150;
    while(i--){
        // sem_wait(mySem);
        printf("Current tickes number is %d.\n",tickets);
        int temp = tickets;
        // sched_yield();
        temp = temp + 1;
        // sched_yield();
        tickets = temp;
        // sem_post(mySem);
    }
}

int main(){
    pthread_t p1,p2;

    mySem = sem_open("Ticket", O_CREAT, 0666, 1);

    pthread_create(&p1,NULL,sold,NULL);
    pthread_create(&p2,NULL,returnT,NULL);

    pthread_join(p1,NULL);
    pthread_join(p2,NULL);

    sem_close(mySem);
    sem_unlink("Ticket");

    printf("The finall value is %d.\n",tickets);

    return 0;
}