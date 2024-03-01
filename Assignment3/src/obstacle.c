#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include "../include/constants.h"
#include "../include/socket.h"
#include "../include/logfile.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int isContinue = 1;
int isNewRound = 1;

static void my_handler(int signum)
{
    if (signum == SIGINT)
        isContinue = 0;
    else if (signum == SIGUSR1)
    {
        isNewRound = 1;
    }
}

static int handleConnection(int fd)
{
    int step = 1;
    int width, height;
    size_t count;
    char buff[64];
    int ret;
    struct Sender sender;
    sender.source = OBSTACLE;

    do
    {
        printf("Step = %d\n", step);
        switch (step)
        {
        case 1:
            ret = send(fd, "OI\n", 3, 0);
            step = (ret == 3) ? 2 : -1;
            break;

        case 2:
            memset(buff, '\0', sizeof(buff));
            ret = readUntilNewline(fd, buff, sizeof(buff));
            printf("received :ret=%d %s\n", ret, buff);
            if (ret > 0)
            {
                ret = sscanf(buff, "%d,%d", &width, &height);
                printf("received ret :%d\n", ret);
                step = (ret == 2) ? 3 : -1;
            }
            else
                step = -1;
            break;

        case 3:
            srand(time(NULL));
            int numberOfObstacle = rand() % 5 + 5;
            int nn = sprintf(buff, "O[%d]", numberOfObstacle);
            for (int i = 0; i < numberOfObstacle; i++)
            {
                sender.coordinates.x = rand() % (width - 30);
                sender.coordinates.y = rand() % (height - 30);
                if ((sender.coordinates.x == width / 2) && (sender.coordinates.y == height / 2))
                {
                    i--; // In case Target coordinates are equal to drone initial position
                }
                else
                {
                    nn += sprintf(&buff[nn], "%d,%d|", sender.coordinates.x, sender.coordinates.y);
                }
            }
            buff[nn++] = '\n';
            ret = send(fd, buff, nn, 0);
            if (ret <= 0)
            {
                printf("OBSTACLES UNSUCCESSFULLY SEND\n");
                step = -1;
            }
            step = 4;
            break;

        case 4:
            ret = readUntilNewline(fd, buff, sizeof(buff));
            if (ret > 0)
            {
                if (strcmp(buff, "GE") == 0)
                {
                    step = 3;
                }

                else if (strcmp(buff, "STOP") == 0)
                {
                    step = -1; // STOP
                }
            }
            else
            {
                step = -1;
            }
            break;
        default:
            step = -1;
            break;
        }
    } while (step != -1);
    printf("Exit Step = %d\n", step);
}

int myConnect2(char *hostname, int port)
{
    struct sockaddr_in sa;
    struct hostent *hp;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fprintf(stderr, "ERROR opening socket\n");

    memset(&sa, '\0', sizeof(struct sockaddr_in));
    // resolve host adress to an IP address
    if ((hp = gethostbyname(hostname)) == NULL)
    {
        if ((sa.sin_addr.s_addr = inet_addr(hostname)) == INADDR_NONE)
        {
            fprintf(stderr, "ERROR, no such host\n");
            return -1;
        }
    }
    else
    {
        memcpy(&sa.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (connect(sockfd, (const struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "ERROR connecting\n");
        return -1;
    }
    return sockfd;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        return EXIT_FAILURE;
    }
    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }
    if (signal(SIGUSR1, my_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);
    struct Sender sender;
    sender.source = OBSTACLE;
    int fd = myConnect2(hostname, port);
    if (fd < 0)
    {
        printf("OBSTACLE error fd\n");
    }

    handleConnection(fd);

    my_close(port);
}
