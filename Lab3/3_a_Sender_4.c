#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>

int main(int argc, char *argv[])
{
    key_t  key;
    int shm_id;
    int sem_id;

    //1.Product the key
    key = ftok(".", 0xFF);
    //2. Creat the shared memory(1K bytes)
    shm_id = shmget(key, 1024, IPC_CREAT|0644);
    if(-1 == shm_id)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    //3. attach the shm_id to this process
    char *shm_ptr;
    shm_ptr = shmat(shm_id, (void*)0x7f95ddc35000, 0);
    if(NULL == shm_ptr)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    printf("Sender共享内存地址:%p\n",shm_ptr);
    shmdt(shm_ptr);
    return 0;
}