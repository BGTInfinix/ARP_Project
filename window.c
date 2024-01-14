#include <curses.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include "include/constants.h"
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

int isContinue = 1;

static int isPositionReached(struct Coordinates DronePos, struct Coordinates ListPos[], int PosNumber)
{
    for (int i = 0; i < PosNumber; i++)
    {
        if ((DronePos.x == ListPos[i].x) && (DronePos.y == ListPos[i].y))
        {
            return 1;
        }
    }
    return 0;
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
    if (argc != 2)
    {
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }
    int pipeID1 = atoi(argv[1]);

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
    int ObstacleNumber = 0;
    int TargetNumber = 0;
    int IndexTarget = 0;

    do
    {
        ssize_t bytesRead = read(pipeID1, &sender, sizeof(sender));
        werase(win);
        box(win, 0, 0);
        werase(score);
        box(score, 0, 0);

        switch (sender.sender)
        {
        case DRONE:
            DronePos.x = sender.coordinates.x;
            DronePos.y = sender.coordinates.y;
            if (isPositionReached(&DronePos, ObstaclePos, ObstacleNumber) == 1)
            {
            }
            if ((DronePos.x == TargetPos[IndexTarget].x) && (DronePos.y == TargetPos[IndexTarget].y))
            {
                IndexTarget ++;
            }
            break;
        case OBSTACLE:
            ObstaclePos[ObstacleNumber].x = sender.coordinates.x;
            ObstaclePos[ObstacleNumber].y = sender.coordinates.y;
            ObstacleNumber++;
            break;
        case TARGET:
            TargetPos[TargetNumber].x = sender.coordinates.x;
            TargetPos[TargetNumber].y = sender.coordinates.y;
            TargetNumber++;
            break;
        default:
            break;
        }
        if (IndexTarget == TargetNumber) isContinue = 0;

        mvwprintw(win, (int)(DronePos.y / scaley), (int)(DronePos.x / scalex), "X");
        for (int i = 0; i < ObstacleNumber; i++)
        {
            mvwprintw(win, (int)(ObstaclePos[i].y / scaley), (int)(ObstaclePos[i].x / scalex), "-");
        }
        for (int i = 0; i < TargetNumber; i++)
        {
            mvwprintw(win, (int)(TargetPos[i].y / scaley), (int)(TargetPos[i].x / scalex), "%d", i+1);
        }
        mvwprintw(score, 1, 20, "%d,%d", DronePos.x, DronePos.y);
        wrefresh(win);
        wrefresh(score);
    } while (isContinue);

    close(pipeID1);
    delwin(win);
    delwin(score);
    endwin();

    return 0;
}