#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
// #include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include "include/constants.h"

int isContinue = 1;



static void update_pos(struct Forces *currentForces, struct Forces *previousForces, struct Coordinates *currentCoordinates, struct Coordinates *previousCoordinates)
{
    int new_posx, new_posy;
    new_posx = ((currentForces->forcex * T * T) - M * previousForces->forcex + (2 * M + T) * currentForces->forcex) / (M + K * T);
    new_posy = ((currentForces->forcey * T * T) - M * previousForces->forcey + (2 * M + T) * currentForces->forcey) / (M + K * T);

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
    int pipeID3 = atoi(argv[3]);

    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }

    struct Coordinates currentCoordinates;
    struct Coordinates previousCoordinates;
    struct Sender sender;
    struct Forces currentForces;
    struct Forces previousForces;

    currentCoordinates.x = currentCoordinates.y = BOARD_SIZE / 2;
    previousCoordinates.x = previousCoordinates.y = BOARD_SIZE / 2;

    sender.sender = DRONE;
    sender.coordinates.x = currentCoordinates.x;
    sender.coordinates.y = currentCoordinates.y;

    write(pipeID2, &sender, sizeof(sender));
    write(pipeID3, &sender, sizeof(sender));
    // logPosition(&sender, sem_id, shm_ptr, pidServer);
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
            currentForces.forcex--;
            currentForces.forcey++;
            break;
        case '1':
            currentForces.forcex++;
            break;
        case '2':
            currentForces.forcex++;
            currentForces.forcey++;
            break;
        // LEFT
        case '3':
            currentForces.forcex--;
            currentForces.forcey--;
            break;
        case '4':
            currentForces.forcex--;
            break;
        case '5':
            currentForces.forcex--;
            currentForces.forcey++;
            break;
        // UP
        case '6':
            currentForces.forcey--;
            break;
        // DOWN
        case '7':
            currentForces.forcey++;
            break;
        default:
            isMoving = 0;
            break;
        }

        if (isMoving)
        {
            update_pos(&currentForces, &previousForces, &currentCoordinates, &previousCoordinates);
            // update_pos(&currentCoordinates, &previousForces);
            sender.coordinates.x = currentCoordinates.x;
            sender.coordinates.y = currentCoordinates.y;
            write(pipeID2, &sender, sizeof(sender));
            write(pipeID3, &sender, sizeof(sender));
            // logPosition(&sender, sem_id, shm_ptr, pidServer);
        }
    } while (isContinue);
    close(pipeID1);
    close(pipeID2);
    close(pipeID3);
    return 0;
}
