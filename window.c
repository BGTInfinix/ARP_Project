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
    
    if ( signal(SIGINT, my_handler) == SIG_ERR) {
        printf("\ncan't catch SIGINT\n");
    }
    int pipeID1 = atoi(argv[1]);

    initscr();

    // refresh window
    struct Coordinates coordinates;

    // get the scale, to scale up the window to the desired size
    double scalex = (double)BOARD_SIZE / ((double)COLS * 0.98);
    double scaley = (double)BOARD_SIZE / ((double)LINES * 0.79);
    WINDOW *win, *score;
    ncursesSetup(&win, &score);
    curs_set(0); // don't show cursor
    do
    {
        ssize_t bytesRead = read(pipeID1, &coordinates, sizeof(coordinates));
              
        werase(win);
        box(win, 0, 0);
        werase(score);
        box(score, 0, 0);
        mvwprintw(win, (int) (coordinates.y / scaley), (int) (coordinates.x / scalex), "X");
        //mvwprintw(win, coordinates.y, coordinates.x, "X");
        mvwprintw(score, 1, 20, "%d,%d", coordinates.x, coordinates.y);
        wrefresh(win);
        wrefresh(score);
    } while(isContinue);

    close(pipeID1);
    delwin(win);
    delwin(score);
    endwin();

    return 0;
}