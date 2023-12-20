#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
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
        // RIGHT
        case 'o':
        case 'r':
            direction = '0';
            break;
        case 'l':
        case 'f':
            direction = '1';
            break;

        case '.':
        case 'v':
            direction = '2';
            break;

        // LEFT
        case 'u':
        case 'w':
            direction = '3';
            break;
        case 'j':
        case 's':
            direction = '4';
            break;
        case 'm':
        case 'x':
            direction = '5';
            break;

        // UP
        case 'i':
        case 'e':
            direction = '6';
            break;

        // DOWN
        case ',':
        case 'c':
            direction = '7';
            break;

        default:
            isWrite = 0;
            break;
        }

        if (isWrite)
        {
            // Send the command force to drone.c
            int ret = write(pipeID, &direction, sizeof(char));

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
