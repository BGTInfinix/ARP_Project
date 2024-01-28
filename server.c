#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "include/constants.h"

int isContinue = 1;

void my_handler(int signum)
{
    if (signum == SIGINT)
        isContinue = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return EXIT_FAILURE;
    }
    int pipeID1 = atoi(argv[1]);
    struct Sender sender;

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
        ssize_t bytesRead = read(pipeID1, &sender, sizeof(sender));
        if (bytesRead == sizeof(sender))
        {
            printf("[%c] (%d, %d)\n", sender.source, sender.coordinates.x, sender.coordinates.y);
        }
    } while (isContinue);
    return 0;
}