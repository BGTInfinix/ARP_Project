#ifndef CONSTANTS_H
#define CONSTANTS_H

#define KEY_POS 777

#define SEM_PATH "/my_sem_path" // Customize the path
#define SHM_PATH "/my_shm_path"

#define M 1.0
#define K 1.0
#define T 1

#define BOARD_SIZE 100

struct Coordinates
{
    int x;
    int y;
    int forcex;
    int forcey;
};

struct Forces
{
    int forcex;
    int forcey;
};

struct Sender
{
    char sender;
    struct Coordinates coordinates;
};

#endif