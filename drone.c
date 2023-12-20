#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

#include <signal.h>
#include "include/constants.h"

// array to store the position of drone
const int shared_seg_size = sizeof(struct Sender);
int isContinue = 1;

int calc_position(double force, int x1, int x2)
{
    double x;
    x = (force * T * T - M * x2 + 2 * M * x1 + K * T * x1) / (M + K * T); // Eulers method

    // Dont let it go outside of the window
    if (x < 0)
    {
        return 0;
    }
    else if (x > BOARD_SIZE)
    {
        return BOARD_SIZE;
    }
    return x;
}

static void update_pos(struct Coordinates *currentCoordinates, struct Coordinates *previousCoordinates)
{
    int new_posx, new_posy;
    new_posx = ((currentCoordinates->forcex * T * T) - M * previousCoordinates->x + (2 * M + T) * currentCoordinates->x) / (M + K * T);
    new_posy = ((currentCoordinates->forcey * T * T) - M * previousCoordinates->y + (2 * M + T) * currentCoordinates->y) / (M + K * T);

    // check if the drone reach the limit
    new_posx = ((new_posx < 0) ? 0 : new_posx);
    new_posy = ((new_posy < 0) ? 0 : new_posy);
    new_posx = ((new_posx > BOARD_SIZE) ? BOARD_SIZE : new_posx);
    new_posy = ((new_posy > BOARD_SIZE) ? BOARD_SIZE : new_posy);

    // update position
    previousCoordinates->x = currentCoordinates->x;
    previousCoordinates->y = currentCoordinates->y;
    currentCoordinates->x = new_posx;
    currentCoordinates->y = new_posy;
}

void logPosition(struct Coordinates *coordinates, sem_t *sem_id, void *shm_ptr, int pidServer)
{
    struct Sender sender;
    sender.sender = 'D';
    memcpy(&sender.coordinates, coordinates, sizeof(struct Coordinates));
    printf("%d %d\n", sender.coordinates.x, sender.coordinates.y);
    sem_wait(sem_id);
    memcpy(shm_ptr, &sender, shared_seg_size);
    sem_post(sem_id);
    kill(pidServer, SIGUSR1);
    usleep(200000);
}

void my_handler(int signum)
{
    if (signum == SIGINT)
        isContinue = 0;
}

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        return EXIT_FAILURE;
    }
    int pipeID1 = atoi(argv[1]);
    int pipeID2 = atoi(argv[2]);
    int pidServer = atoi(argv[3]);

    // SHARED MEMORY PART
    sem_t *sem_id = sem_open(SEM_PATH, 0);
    if (sem_id == SEM_FAILED)
    {
        fprintf(stderr, "sem_open() failed.  errno:%d\n", errno);
        sleep(15);
        exit(EXIT_FAILURE);
    }
    int shmfd = shm_open(SHM_PATH, O_RDWR, S_IRWXU | S_IRWXG);
    if (shmfd < 0)
    {
        fprintf(stderr, "shm_open() failed.  errno:%d\n", errno);
        sem_close(sem_id);
        sleep(5);
        exit(EXIT_FAILURE);
    }
    void *shm_ptr = mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        fprintf(stderr, "mmap() failed.  errno:%d\n", errno);
        sem_close(sem_id);
        shm_unlink(SHM_PATH);
        sleep(5);
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }

    // int force[2] = {0, 0};
    struct Coordinates currentCoordinates;
    struct Coordinates previousCoordinates;
    currentCoordinates.x = currentCoordinates.y = BOARD_SIZE / 2;
    previousCoordinates.x = previousCoordinates.y = BOARD_SIZE / 2;

    write(pipeID2, &currentCoordinates, sizeof(currentCoordinates));
    logPosition(&currentCoordinates, sem_id, shm_ptr, pidServer);
    int isMoving = 1;
    do
    {
        // 3
        char direction = '9';
        isMoving = 1;
        ssize_t bytesRead = read(pipeID1, &direction, sizeof(char)); // Receive command force from keyboard_manager
        // printf("bytesRead=%ld %d\n", bytesRead, errno);
        switch (direction)
        {
        // RIGHT
        case '0': // East
            currentCoordinates.forcex--;
            currentCoordinates.forcey++;
            break;
        case '1':
            currentCoordinates.forcex++;
            break;
        case '2':
            currentCoordinates.forcex++;
            currentCoordinates.forcey++;
            break;
        // LEFT
        case '3':
            currentCoordinates.forcex--;
            currentCoordinates.forcey--;
            break;
        case '4':
            currentCoordinates.forcex--;
            break;
        case '5':
            currentCoordinates.forcex--;
            currentCoordinates.forcey++;
            break;
        // UP
        case '6':
            currentCoordinates.forcey--;
            break;
        // DOWN
        case '7':
            currentCoordinates.forcey++;
            break;
        default:
            isMoving = 0;
            break;
        }

        if (isMoving)
        {
            update_pos(&currentCoordinates, &previousCoordinates);
            write(pipeID2, &currentCoordinates, sizeof(currentCoordinates));
            logPosition(&currentCoordinates, sem_id, shm_ptr, pidServer);
        }
    } while (isContinue);
    close(pipeID1);
    close(pipeID2);
    shm_unlink(SHM_PATH);
    sem_close(sem_id);
    sem_unlink(SEM_PATH);
    return 0;
}
