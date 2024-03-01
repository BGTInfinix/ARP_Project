#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include "../include/constants.h"
#include "../include/logfile.h"

int isContinue = 1;

static int calc_drone_pos(int force, int firstCoordinate, int secondCoordinate)
{
    double x;
    x = (force * T * T - M * secondCoordinate + 2 * M * firstCoordinate + K * T * firstCoordinate) / (M + K * T); // Eulers method

    if (x < -20)
    {
        return 5;
    }
    else if (x > BOARD_SIZE + 20)
    {
        return BOARD_SIZE - 5;
    }
    return (int)x;
}

// Calculate the angle at which the drone is coming at the obstacle
static double calculateAngle(struct Coordinates *currentCoordinates, struct Coordinates *obstaclePos)
{
    double angleRadians = atan2(obstaclePos->y - currentCoordinates->y, obstaclePos->x - currentCoordinates->x);
    return angleRadians;
}

static struct Coordinates calc_repulsive(struct Coordinates obstaclePos[], int numberOfObstacles, struct Coordinates *currentCoordinates, struct Forces *Forces)
{
    double sumx = 0, sumy = 0;
    for (int i = 0; i < numberOfObstacles; i++)
    {
        double p_q = sqrt(pow(obstaclePos[i].x - currentCoordinates->x, 2) + pow(obstaclePos[i].y - currentCoordinates->y, 2));
        double p_wall[] = {currentCoordinates->x, currentCoordinates->y, BOARD_SIZE - currentCoordinates->x, BOARD_SIZE - currentCoordinates->y};
        if (p_q <= p)
        {
            double angle = calculateAngle(currentCoordinates, &obstaclePos[i]);
            if (obstaclePos[i].x > currentCoordinates->x && Forces->forcex > 0)
            {
                sumx -= pow((1 / p_q) - (1 / p), 2) * cos(angle);
            }
            else if (obstaclePos[i].x < currentCoordinates->x && Forces->forcex < 0)
            {
                sumx += pow((1 / p_q) - (1 / p), 2) * cos(angle);
            }
            else if (obstaclePos[i].y > currentCoordinates->y && Forces->forcey > 0)
            {
                sumy -= pow((1 / p_q) - (1 / p), 2) * sin(angle);
            }
            else if (obstaclePos[i].y < currentCoordinates->y && Forces->forcey < 0)
            {
                sumy += pow((1 / p_q) - (1 / p), 2) * sin(angle);
            }
        }

        // Repulsive force for the walls
        if (p_wall[0] <= p && Forces->forcex < 0)
        {
            sumx -= pow((1 / p_wall[0]) - (1 / p), 2);
        }
        else if (p_wall[1] <= p && Forces->forcey < 0)
        {
            sumy -= pow((1 / p_wall[1]) - (1 / p), 2);
        }
        else if (p_wall[2] <= p && Forces->forcex > 0)
        {
            sumx -= pow((1 / p_wall[2]) - (1 / p), 2);
        }
        else if (p_wall[3] <= p && Forces->forcey > 0)
        {
            sumy -= pow((1 / p_wall[3]) - (1 / p), 2);
        }
    }
    struct Coordinates repulsion;
    repulsion.x = -0.5 * n * sumx;
    repulsion.y = -0.5 * n * sumy;
    return repulsion;
}

// Get the new drone_pos using calc_function and store the previous drone_poss
static void update_pos(struct Forces *Forces, struct Coordinates *currentCoordinates, struct Coordinates *previousCoordinates, struct Coordinates *obstaclePos, int numberOfObstacles)
{
    struct Coordinates repulsion;
    double new_posx, new_posy;
    repulsion = calc_repulsive(obstaclePos, numberOfObstacles, currentCoordinates, Forces);

    if (Forces->forcex > 0)
    {
        new_posx = calc_drone_pos(Forces->forcex - repulsion.x, currentCoordinates->x, previousCoordinates->x);
    }
    else if (Forces->forcex < 0)
    {
        new_posx = calc_drone_pos(Forces->forcex + repulsion.x, currentCoordinates->x, previousCoordinates->x);
    }
    else
    {
        new_posx = calc_drone_pos(Forces->forcex, currentCoordinates->x, previousCoordinates->x);
    }

    if (Forces->forcey > 0)
    {
        new_posy = calc_drone_pos(Forces->forcey - repulsion.y, currentCoordinates->y, previousCoordinates->y);
    }
    else if (Forces->forcey < 0)
    {
        new_posy = calc_drone_pos(Forces->forcey + repulsion.y, currentCoordinates->y, previousCoordinates->y);
    }
    else
    {
        new_posy = calc_drone_pos(Forces->forcey, currentCoordinates->y, previousCoordinates->y);
    }

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

    if (argc != 3)
    {
        return EXIT_FAILURE;
    }
    int pipeID1 = atoi(argv[1]);
    int pipeID2 = atoi(argv[2]);
    printf("pipes %d %d\n", pipeID1, pipeID2);

    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }

    int numberOfObstacles = 0;
    int isMoving = 1;

    struct Coordinates Obstacles[MAX_OBSTACLE];
    struct Coordinates currentCoordinates;
    struct Coordinates previousCoordinates;
    currentCoordinates.x = currentCoordinates.y = BOARD_SIZE / 2;
    previousCoordinates.x = previousCoordinates.y = BOARD_SIZE / 2;

    struct Sender sender;
    sender.source = DRONE;
    sender.coordinates.x = currentCoordinates.x;
    sender.coordinates.y = currentCoordinates.y;

    struct Sender message;
    struct Forces forces;
    forces.forcex = 0;
    forces.forcey = 0;

    write(pipeID2, &sender, sizeof(sender));
    do
    {
        char direction = '9';
        isMoving = 1;
        int bytesRead = read(pipeID1, &message, sizeof(message)); // Receive command force from keyboard_manager
        printf("%c / %d : %d, %d\n", message.source, bytesRead, message.coordinates.x, message.coordinates.y);

        if (message.source == OBSTACLE)
        {
            if (numberOfObstacles < MAX_OBSTACLE)
            {
                Obstacles[numberOfObstacles].x = message.coordinates.x;
                Obstacles[numberOfObstacles].y = message.coordinates.y;
                printf("Obstacle coord %d,%d\n", message.coordinates.x, message.coordinates.y);
                numberOfObstacles++;
            }
        }

        else if (message.source == KEYBOARD)
        {
            switch (message.coordinates.x)
            {
            case -2:
                currentCoordinates.x = currentCoordinates.y = BOARD_SIZE / 2;
                previousCoordinates.x = previousCoordinates.y = BOARD_SIZE / 2;
                forces.forcex = 0;
                forces.forcey = 0;
                break;
            case -1:
                isMoving = 0;
                isContinue = 0;
                break;
            case '0': // E - TOP
                forces.forcey--;
                writeToLogFile(logpath, "User input 8", "DRONE");
                break;
            case '1': // R - TOP RIGHT
                forces.forcex++;
                forces.forcey--;
                writeToLogFile(logpath, "User input 9", "DRONE");
                break;
            case '2': // F - RIGHT
                forces.forcex++;
                writeToLogFile(logpath, "User input 6", "DRONE");
                break;
            case '3': // V - BOTTOM RIGHT
                forces.forcex++;
                forces.forcey++;
                writeToLogFile(logpath, "User input 3", "DRONE");
                break;
            case '4': // C - BOTTOM
                forces.forcey++;
                writeToLogFile(logpath, "User input 2", "DRONE");
                break;
            case '5': // X - BOTTOM LEFT
                forces.forcex--;
                forces.forcey++;
                writeToLogFile(logpath, "User input 1", "DRONE");
                break;
            case '6': // S - LEFT
                forces.forcex--;
                writeToLogFile(logpath, "User input 4", "DRONE");
                break;
            case '7': // W - TOP LEFT
                forces.forcex--;
                forces.forcey--;
                writeToLogFile(logpath, "User input 7", "DRONE");
                break;
            case '8': // Stop / reset
                forces.forcex = 0;
                forces.forcey = 0;
                writeToLogFile(logpath, "User input 5", "DRONE");
            default:
                isMoving = 0;
                break;
            }
            if (isMoving)
            {
                update_pos(&forces, &currentCoordinates, &previousCoordinates, Obstacles, numberOfObstacles);
                writeToLogFile(logpath, "Position has been updated", "DRONE");
                sender.coordinates.x = currentCoordinates.x;
                sender.coordinates.y = currentCoordinates.y;
                write(pipeID2, &sender, sizeof(sender));
            }
        }

    } while (isContinue);
    close(pipeID1);
    close(pipeID2);
    return 0;
}
