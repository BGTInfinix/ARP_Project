#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include "include/constants.h"
#include "include/logfile.c"

const int MAX_PIPES = 3;     // Number of pipes per process
const int MAX_PROCESSES = 6; // Number of process runing in project

int GetPIDbyName(const char *pidName)
{
    FILE *fp;
    char pidofCmd[50] = {0};
    int pidValue = -1;

    sprintf(pidofCmd, "pidof %s > /tmp/pidof", pidName);
    system("pidof server > /tmp/pidof");
    fp = fopen("/tmp/pidof", "r");
    fscanf(fp, "%d", &pidValue);
    fclose(fp);

    return pidValue;
}

static int createPipes(int pipes[][2])
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        pipes[i][0] = pipes[i][1] = -1;
    }

    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipe(pipes[i]))
        {
            fprintf(stderr, "Pipe failed.\n");
            return -1;
        }
    }
    return 0;
}

static void closePipes(int pipes[][2])
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipes[i][0] >= 0)
        {
            close(pipes[i][0]);
        }
        if (pipes[i][1] >= 0)
        {
            close(pipes[i][1]);
        }
    }
}

static void stopProcess(pid_t processes[], int numberOfProcesses)
{
    for (int i = 0; i < numberOfProcesses; i++)
    {
        kill(processes[i], 9);
    }
}

int launchKeyboard(int pipes[][2])
{
    // Use only Pipe[0][1] for writing to Drone
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipes[i][0] >= 0)
        {
            close(pipes[i][0]);
        }
        if ((i != 0) && (pipes[i][1] >= 0))
        {
            close(pipes[i][1]);
        }
    }
    char argument1[12];
    sprintf(argument1, "%d", pipes[0][1]);
    // Launch process window
    char *argsWindow[] = {"konsole", "-e", "./build/keyboard", argument1, NULL};
    execvp("konsole", argsWindow);

    return EXIT_SUCCESS;
}

int launchDrone(int pipes[][2])
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if ((i != 0) && (i != 2) && (pipes[i][0] >= 0))
        {
            close(pipes[i][0]);
        }
        if ((i != 1) && (i != 2) && (pipes[i][1] >= 0))
        {
            close(pipes[i][1]);
        }
    }
    char argument1[12];
    sprintf(argument1, "%d", pipes[0][0]);
    char argument2[12];
    sprintf(argument2, "%d", pipes[1][1]);
    char argument3[12];
    sprintf(argument3, "%d", pipes[2][1]);

    // Launch process Drone
    char *argsWindow[] = {"konsole", "-e", "./build/drone", argument1, argument2, argument3, NULL};
    execvp("konsole", argsWindow);

    return EXIT_SUCCESS;
}

int launchObstacle(int pipes[][2])
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipes[i][0] >= 0)
        {
            close(pipes[i][0]);
        }
        if ((i != 1) && (i != 2) && (i != 0) && (pipes[i][1] >= 0))
        {
            close(pipes[i][1]);
        }
    }
    char argument1[12];
    sprintf(argument1, "%d", pipes[1][1]);
    char argument2[12];
    sprintf(argument2, "%d", pipes[2][1]);
    char argument3[12];
    sprintf(argument3, "%d", pipes[0][1]);
    char *argsWindow[] = {"konsole", "-e", "./build/obstacle", argument1, argument2, argument3, NULL};
    return execvp("konsole", argsWindow);
}

int launchTarget(int pipes[][2])
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipes[i][0] >= 0)
        {
            close(pipes[i][0]);
        }
        if ((i != 1) && (i != 2) && (pipes[i][1] >= 0))
        {
            close(pipes[i][1]);
        }
    }
    char argument1[12];
    sprintf(argument1, "%d", pipes[1][1]);
    char argument2[12];
    sprintf(argument2, "%d", pipes[2][1]);
    char *argsWindow[] = {"konsole", "-e", "./build/target", argument1, argument2, NULL};
    return execvp("konsole", argsWindow);
}

int launchBlackboard(int pipes[][2])
{
    // Use pipes[1][0] for reading from drone
    // Use Pipe[2][1] for writing to drone
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if ((i != 1) && (pipes[i][0] >= 0))
        {
            close(pipes[i][0]);
        }
        if ((pipes[i][1] >= 0))
        {
            close(pipes[i][1]);
        }
    }
    char argument1[12];
    sprintf(argument1, "%d", pipes[1][0]);

    // Launch process window
    char *argsWindow[] = {"konsole", "-e", "./build/window", argument1, NULL};
    return execvp("konsole", argsWindow);
}

int launchServer(int pipes[][2])
{
    // Launch process Server (Shared Memory)
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if ((i != 2) && pipes[i][0] >= 0)
        {
            close(pipes[i][0]);
        }
        if (pipes[i][1] >= 0)
        {
            close(pipes[i][1]);
        }
    }
    char argument[12];
    sprintf(argument, "%d", pipes[2][0]);
    char *argsWindow[] = {"konsole", "-e", "./build/server", argument, NULL};
    return execvp("konsole", argsWindow);
}

int launchWatchDog(pid_t processes[], int numberOfProcesses)
{
    // Launch process Server

    char *argsWindow[7 + 3];
    argsWindow[0] = "konsole";
    argsWindow[1] = "-e";
    argsWindow[2] = "./build/watchdog";
    int totalLength = 0;
    int nextArgment = 3;
    char buffer[64];

    for (int i = 0; i < numberOfProcesses; i++)
    {
        int j = sprintf(&buffer[totalLength], "%d", processes[i]);
        argsWindow[nextArgment] = &buffer[totalLength];
        totalLength += j + 1;
        nextArgment++;
    }
    argsWindow[nextArgment] = NULL;
    return execvp("konsole", argsWindow);
}

int main(int argc, char **argv)
{
    int pipes[MAX_PIPES][2];
    pid_t processes[MAX_PROCESSES];

    if (createPipes(pipes))
    {
        return EXIT_FAILURE;
    }
    int ret = EXIT_SUCCESS;
    clearLogFile(logpath);
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        processes[i] = fork();
        if (processes[i] == (pid_t)0)
        {
            // This is the child process. Close other end first.
            switch (i)
            {
            case 0: // logger server process
                launchServer(pipes);
                break;
            case 1: // Window Process
                launchBlackboard(pipes);
                break;
            case 2: // Drone Process
                launchDrone(pipes);
                break;
            case 3: // Obstacle Process
                launchObstacle(pipes);
            case 4: // Target Process
                launchTarget(pipes);
            case 5: // Keyboard process
                launchKeyboard(pipes);
                break;
            case 6: // watchdog process, last one to be started
                    //   closePipes(pipes);
                    //   launchWatchDog(processes, i);
                break;
            default:
                break;
            }
        }
        else if (processes[i] < (pid_t)0)
        {
            // The fork failed.
            stopProcess(processes, i);
            ret = EXIT_FAILURE;
        }
        else
            sleep(1);
    }
    closePipes(pipes);
    do
    {
        printf("Press the Q key to Quit :");
        if (toupper(getchar()) == 'Q')
            break;
    } while (1);
    stopProcess(processes, MAX_PROCESSES);

    pid_t pid;
    time_t t;
    int status;
    do
    {
        if ((pid = waitpid(-1, &status, WUNTRACED | WCONTINUED)) == -1)
        {
            perror("wait() error");
        }
        else if (pid == 0)
        {
            time(&t);
            printf("child is still running at %s", ctime(&t));
            sleep(1);
        }
        else if (WIFEXITED(status))
        {
            printf("child exited with status of %d pid=%d\n", WEXITSTATUS(status), pid);
        }
        else
        {
            puts("child did not exit successfully");
        }
    } while (pid == 0);
    return ret;
}