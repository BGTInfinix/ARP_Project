#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include "include/constants.h"

int main(int argc, char *argv[])
{
    // Check program arguments
    if (argc != 2)
    {
        printf("This program needs one argument\n");
        return EXIT_FAILURE;
    }
    // PIPE
    int pipeID = atoi(argv[1]);
    struct Sender sender;
    sender.source = KEYBOARD;

    static struct termios oldTermios, newTermios;
    tcgetattr(STDIN_FILENO, &oldTermios);
    newTermios = oldTermios;
    cfmakeraw(&newTermios);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
    static struct termios oldt, newt;
    int isContinue = 1;

    int isWrite = 1;
    char direction;
    do
    {
        isWrite = 1;
        switch (getchar())
        {
        case ' ': // enter space to exit
            isContinue = isWrite = 0;
            break;

        case '8': // up
        case 'e':
            direction = '0';
            break;

        case '9':
        case 'r': // up - right
            direction = '1';
            break;

        case '6':
        case 'f': // right
            direction = '2';
            break;

        case '3':
        case 'v': // bot - right
            direction = '3';
            break;

        case '2':
        case 'c': // bot
            direction = '4';
            break;

        case '1':
        case 'x': // bot - left
            direction = '5';
            break;

        case '4':
        case 's': // left
            direction = '6';
            break;

        case '7':
        case 'w': // top - left
            direction = '7';
            break;
        case '5':
        case 'd':
            direction = '8'; //stop
            break;

        default:
            isWrite = 0;
            break;
        }

        if (isWrite)
        {
            sender.coordinates.x = sender.coordinates.y = direction;
            // Send the command force to drone.c
            int ret = write(pipeID, &sender, sizeof(sender));

            if (ret < 0)
            {
                perror("writing error\n");
                sleep(10);
            }
        }
    } while (isContinue);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
    close(pipeID);
    return 0;
}
