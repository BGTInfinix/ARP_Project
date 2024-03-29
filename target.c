#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include "include/constants.h"

int isContinue = 1;
int isNewRound = 1;

void my_handler(int signum)
{
    if (signum == SIGINT)
        isContinue = 0;
    else if (signum == SIGUSR1)
    {
        isNewRound = 1;
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        return EXIT_FAILURE;
    }
    int pipeID1 = atoi(argv[1]);
    int pipeID2 = atoi(argv[2]);

    struct Sender sender;
    sender.source = TARGET;

    srand(time(NULL));

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
        if (isNewRound == 1)
        {
            isNewRound = 0;
            int numberOfTarget = rand() % 5 + 5;
            for (int i = 0; i < numberOfTarget; i++)
            {
                sender.coordinates.x = rand() % (BOARD_SIZE - 30);
                sender.coordinates.y = rand() % (BOARD_SIZE - 30);
                if ((sender.coordinates.x == BOARD_SIZE / 2) && (sender.coordinates.y == BOARD_SIZE / 2))
                {
                    i--; // In case Target coordinates are equal to drone initial position
                }
                else
                {
                    write(pipeID1, &sender, sizeof(sender));
                    write(pipeID2, &sender, sizeof(sender));
                }
            }
        }
        pause();
    } while (isContinue);
    close(pipeID1);
    close(pipeID2);
}