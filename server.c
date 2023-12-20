#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "include/constants.h"

#include <signal.h>

const int shared_seg_size = sizeof(struct Sender);

int isContinue = 1;
sem_t *sem_id;
void *shm_ptr;

void my_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        struct Sender sender;
        sem_wait(sem_id);
        memcpy(&sender, shm_ptr, shared_seg_size);
        sem_post(sem_id);
        printf("[%c] (%d, %d)\n", sender.sender, sender.coordinates.x, sender.coordinates.y);
    }
    else if (signum == SIGINT)
        isContinue = 0;
}

int main(int argc, char *argv[])
{
    // initialize semaphore
    sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (sem_id == SEM_FAILED)
    {
        fprintf(stderr, "sem_open() failed.  errno:%d\n", errno);
        sleep(15);
        exit(EXIT_FAILURE);
    }
    sem_init(sem_id, 1, 0); // initialized sem_id to 0 until shared memory is instantiated

    // SHARED MEMORY
    int shmfd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG); // create shared memory object
    if (shmfd < 0)
    {
        fprintf(stderr, "sem_open() failed.  errno:%d\n", errno);
        sleep(15);
        exit(EXIT_FAILURE);
    }
    ftruncate(shmfd, shared_seg_size);                                                   // truncate size of shared memory
    shm_ptr = mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0); // map pointer
    if (shm_ptr == MAP_FAILED)
    {
        fprintf(stderr, "shm_open() failed.  errno:%d\n", errno);
        sleep(5);
        exit(EXIT_FAILURE);
    }
    // post semaphore (shared memory is ready)
    sem_post(sem_id);
    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }
    if (signal(SIGUSR1, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }

    do
    {
        pause();
    } while (isContinue);

    // clean up
    shm_unlink(SHM_PATH);
    sem_close(sem_id);
    sem_unlink(SEM_PATH);
    munmap(shm_ptr, shared_seg_size);

    return 0;
}