#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "../include/constants.h"
#include "../include/socket.h"
#include "../include/logfile.h"

int isContinue = 1;
int endOfGame = 0;

int height = BOARD_SIZE;
int width = BOARD_SIZE;
int numberOfTargets;
struct Coordinates coordinatesTarget[10];
int numberOfObstacles;
struct Coordinates coordinatesObstacle[10];

int pipeID1, pipeID2, pipeID3, pipeID4;

struct Line
{
    int step;
    int type;
    int fd;
    void (*FuncPtr)(struct Line *line, char *buffer);
};

static void funcKeyboard(struct Line *line, char *buffer)
{
    write(pipeID1, buffer, sizeof(struct Sender));
}

static void funcHandle(struct Line *line, char *buffer)
{
    char out[64];
    int ret;
    int numberOfItems;
    struct Sender sender;
    printf("buffer=%lu %s %d tye=%d\n", strlen(buffer), buffer, line->step, line->type);
    switch (line->step)
    {

    case 0:
        if (!strcmp(buffer, "TI"))
        {
            writeToLogFile(logpath, "Connection Done with Target client\n", "SERVER");
            sprintf(out, "%d,%d\n", width, height);
            ret = send(line->fd, out, strlen(out), 0);
            printf("Send TI : %d,%d\n", width, height);
            line->type = TARGET;
            line->step = (ret == strlen(out)) ? 1 : -1;
        }
        else if (!strcmp(buffer, "OI"))
        {
            writeToLogFile(logpath, "Connection Done with Target client\n", "SERVER");
            sprintf(out, "%d,%d\n", width, height);
            ret = send(line->fd, out, strlen(out), 0);
            printf("Send : %d,%d\n", width, height);
            line->type = OBSTACLE;
            line->step = (ret > 0) ? 1 : -1;
        }
        break;

    case 1:
        if (line->type == TARGET)
        {
            sscanf(buffer, "T[%d]%s", &numberOfItems, out);
        }
        else if (line->type == OBSTACLE)
        {
            sscanf(buffer, "O[%d]%s", &numberOfItems, out);
        }
        printf("number item = %d\n", numberOfItems);
        writeToLogFile(logpath, buffer, (line->type == TARGET) ? "TARGET" : "OBSTACLE");
        line->step = 2;
        sender.source = line->type;
        for (int i = 0; i < numberOfItems; i++)
        {
            ret = sscanf(out, "%d,%d|%s", &sender.coordinates.x, &sender.coordinates.y, buffer);
            strcpy(out, buffer);
            printf("ret = %d, coord = %d, %d, buffer = [%s]\n", ret, sender.coordinates.x, sender.coordinates.y, out);
            if (ret >= 2)
            {
                sender.source = line->type;
                write(pipeID2, &sender, sizeof(sender));
                if (line->type == OBSTACLE)
                {
                    write(pipeID1, &sender, sizeof(sender));
                }
            }
            else
            {
                line->step = -1;
                writeToLogFile(logpath, "ERROR RECEIVING TARGETS", "SERVER");
            }
        }
        break;
    }
}

static int myRead(int port)
{
    int opt = 1;
    int addrlen, new_socket, activity, i;
    int fd_server;
    char buffer[64];
    struct Line lines[4];
    int max_clients = sizeof(lines) / sizeof(struct Line);
    struct sockaddr_in address;

    // initialise all client_socket[] to 0 so not checked
    lines[0].fd = pipeID3;
    lines[0].FuncPtr = funcKeyboard;
    lines[0].type = KEYBOARD;
    lines[0].step = 0;

    lines[1].fd = pipeID4;
    lines[1].FuncPtr = NULL;
    lines[1].type = SERVER;
    lines[1].step = 0;

    for (i = 2; i < max_clients; i++)
    {
        lines[i].fd = 0;
        lines[i].FuncPtr = funcHandle;
        lines[i].step = 0;
    }
    // create a master socket
    if ((fd_server = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        return EXIT_FAILURE;
    }
    // set master socket to allow multiple connections ,
    // this is just a good habit, it will work without this
    if (setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        close(fd_server);
        perror("setsockopt");
        return EXIT_FAILURE;
    }
    // type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    // bind the socket to localhost port 8888
    if (bind(fd_server, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        return EXIT_FAILURE;
    }
    printf("Listener on port %d \n", port);
    // try to specify maximum of 3 pending connections for the master socket
    if (listen(fd_server, 3) < 0)
    {
        perror("listen");
        return EXIT_FAILURE;
    }
    // accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    int j = 0;
    fd_set readfds;
    do
    {
        // clear the socket set
        FD_ZERO(&readfds);
        // add master socket to set
        FD_SET(fd_server, &readfds);
        int maxfd = fd_server;
        // add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            // if valid socket descriptor then add to read list
            if (lines[i].fd > 0)
                FD_SET(lines[i].fd, &readfds);

            // highest file descriptor number, need it for the select function
            if (lines[i].fd > maxfd)
                maxfd = lines[i].fd;
        }
        activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error\n");
        }
        else if ((activity < 0) && (errno == EINTR) && !isContinue)
        {
            return 0;
        }
        // If something happened on the master socket ,
        // then its an incoming connection
        if (FD_ISSET(fd_server, &readfds))
        {
            if ((new_socket = accept(fd_server, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            // add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                // if position is empty
                if (lines[i].fd == 0)
                {
                    lines[i].fd = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }
        // else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            if (FD_ISSET(lines[i].fd, &readfds))
            {
                if (i == 0)
                {
                    int ret;
                    struct Sender message;
                    message.source = lines[i].type;
                    ssize_t bytesRead = read(lines[i].fd, &message, sizeof(message)); // Receive command force from keyboard_manager
                    printf("%lu, message = %d %d\n", bytesRead, message.coordinates.x, message.coordinates.y);
                    if (message.coordinates.x == -1)
                    {
                        message.source = KEYBOARD;
                        write(pipeID1, &message, sizeof(message));
                        message.source = SERVER;
                        write(pipeID2, &message, sizeof(message));
                        sleep(1);
                        if (lines[2].fd > 0)
                            ret = send(lines[2].fd, "STOP\n", 5, 0);
                        if (lines[3].fd > 0)
                            ret = send(lines[3].fd, "STOP\n", 5, 0);
                        isContinue = 0;
                        break;
                    }

                    if (message.coordinates.x == -2)
                    {
                        if (endOfGame)
                        {
                            message.source = KEYBOARD;
                            write(pipeID1, &message, sizeof(message));
                            message.source = SERVER;
                            write(pipeID2, &message, sizeof(message));
                            sleep(1);
                            if (lines[2].fd > 0)
                            {
                                ret = send(lines[2].fd, "GE\n", 3, 0);
                                lines[2].step = 1;
                            }

                            if (lines[3].fd > 0)
                            {
                                ret = send(lines[3].fd, "GE\n", 3, 0);
                                lines[3].step = 1;
                            }
                            endOfGame = 0;
                        }
                    }
                    lines[i].FuncPtr(&lines[i], (char *)&message);
                }

                else if (i == 1)
                {
                    char c;
                    ssize_t bytesRead = read(lines[i].fd, &c, sizeof(char));
                    if ((bytesRead == 1) && (c == '*'))
                    {
                        endOfGame = 1;
                    }
                }

                else
                {
                    memset(buffer, 0, sizeof(buffer));
                    readUntilNewline(lines[i].fd, buffer, sizeof(buffer));
                    lines[i].FuncPtr(&lines[i], buffer);
                }
            }
        }
    } while (isContinue);
    // add child sockets to set
    for (i = 0; i < max_clients; i++)
    {
        // if valid socket descriptor then add to read list
        if (lines[i].fd > 0)
            close(lines[i].fd);
    }
    shutdown(fd_server, 2);
    close(fd_server);
    return 0;
}

void my_handler(int signum)
{
    printf("signal =%d\n", signum);
    if (signum == SIGINT)
        isContinue = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        return EXIT_FAILURE;
    }
    pipeID1 = atoi(argv[1]);
    pipeID2 = atoi(argv[2]);
    pipeID3 = atoi(argv[3]);
    pipeID4 = atoi(argv[4]);
    int port = atoi(argv[5]);

    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }
    if (signal(SIGUSR1, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }
    myRead(port);

    printf("END PROCESS SERVER\n");
    return 0;
}
