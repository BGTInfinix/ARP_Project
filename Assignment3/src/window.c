#include <curses.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "../include/constants.h"
#include "../include/logfile.h"
#include "../include/socket.h"

int isContinue = 1;
int endOfGame = 0;

static int isPositionReached(struct Coordinates *DronePos, struct Coordinates ListPos[], int numberOfObjects)
{
    for (int i = 0; i < numberOfObjects; i++)
    {
        if ((abs(DronePos->x - ListPos[i].x) < THRESHOLD) && (abs(DronePos->y - ListPos[i].y) < THRESHOLD))
        {
            return i;
        }
    }
    return -1;
}

void my_handler(int signum)
{
    if (signum == SIGINT)
        isContinue = 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    wrefresh(local_win);

    return local_win;
}

void ncursesSetup(WINDOW **display, WINDOW **score)
{
    int initPos[4] = {// Start position of the window
                      LINES / 200,
                      COLS / 200,
                      (LINES / 200) + LINES - (LINES / 5),
                      COLS / 200};

    *score = create_newwin(LINES * 0.2, COLS * 0.99, initPos[2], initPos[3]);
    *display = create_newwin(LINES * 0.8, COLS * 0.99, initPos[0], initPos[1]);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }
    int pipeID1 = atoi(argv[1]);
    int pipeID2 = atoi(argv[2]);

    initscr();

    // refresh window
    struct Sender sender;

    // get the scale, to scale up the window to the desired size
    double scalex = (double)BOARD_SIZE / ((double)COLS * 0.98);
    double scaley = (double)BOARD_SIZE / ((double)LINES * 0.79);
    WINDOW *win, *score;
    ncursesSetup(&win, &score);
    curs_set(0); // don't show cursor

    struct Coordinates DronePos;
    struct Coordinates ObstaclePos[MAX_OBSTACLE];
    struct Coordinates TargetPos[MAX_TARGET];

    int indexOfTargetReached = -1;
    int numberOfObstacles = 0;
    int numberOfTargets = 0;
    int isOneTargetReceived = 0;
    int indexTargetToReach = 0;
    int isObstacleReach = 0;

    do
    {
        ssize_t bytesRead = read(pipeID1, &sender, sizeof(sender));
        werase(win);
        box(win, 0, 0);
        werase(score);
        box(score, 0, 0);

        switch (sender.source)
        {
        case DRONE:
            DronePos.x = sender.coordinates.x;
            DronePos.y = sender.coordinates.y;
            indexOfTargetReached = isPositionReached(&DronePos, TargetPos, numberOfTargets);
            if (indexOfTargetReached == indexTargetToReach)
            {
                numberOfTargets--;
                if (indexOfTargetReached != numberOfTargets)
                {
                    for (int i = indexTargetToReach; i < numberOfTargets; i++)
                        TargetPos[i] = TargetPos[i + 1];
                }
                indexTargetToReach = 0;
                writeToLogFile(logpath, "A TARGET HAS BEEN REACHED", "WINDOW");
            }
            break;
        case OBSTACLE:
            if (numberOfObstacles < MAX_OBSTACLE)
            {
                ObstaclePos[numberOfObstacles].x = sender.coordinates.x;
                ObstaclePos[numberOfObstacles].y = sender.coordinates.y;
                numberOfObstacles++;
            }
            writeToLogFile(logpath, "A OBSTACLE HAS BEEN PRINTED", "WINDOW");
            break;
        case TARGET:
            isOneTargetReceived = 1;
            if (numberOfTargets < MAX_TARGET)
            {
                TargetPos[numberOfTargets].x = sender.coordinates.x;
                TargetPos[numberOfTargets].y = sender.coordinates.y;
                numberOfTargets++;
            }
            writeToLogFile(logpath, "A TARGET HAS BEEN PRINTED", "WINDOW");
            break;
        case SERVER:
            if (sender.coordinates.x == -1)
            {
                isContinue = 0;
            }
            else
            {
                numberOfObstacles = numberOfTargets = 0;
                endOfGame = 0;
            }
        default:
            break;
        }

        if (isOneTargetReceived && (numberOfTargets == 0))
        {
            endOfGame = 1;
            write(pipeID2, "*", 1);
            writeToLogFile(logpath, "END OF THE GAME", "WINDOW");
        }

        mvwprintw(win, (int)(DronePos.y / scaley), (int)(DronePos.x / scalex), "X");
        for (int i = 0; i < numberOfObstacles; i++)
        {
            mvwprintw(win, (int)(ObstaclePos[i].y / scaley), (int)(ObstaclePos[i].x / scalex), "/");
        }
        for (int i = 0; i < numberOfTargets; i++)
        {
            mvwprintw(win, (int)(TargetPos[i].y / scaley), (int)(TargetPos[i].x / scalex), "%d", i + 1);
        }
        mvwprintw(score, 1, 20, "%d,%d", DronePos.x, DronePos.y);
        wrefresh(win);
        wrefresh(score);
    } while (isContinue);

    close(pipeID1);
    close(pipeID2);
    delwin(win);
    delwin(score);
    endwin();

    return 0;
}