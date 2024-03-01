#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <../include/socket.h>


void my_close(int fd)
{
    if (fd >= 0)
    {
        shutdown(fd, 2);
        close(fd);
    }
}

int readUntilNewline(int sockfd, char *buffer, size_t bufferSize)
{
    size_t bytesRead = 0;
    char c;

    while (bytesRead < bufferSize - 1)
    {
        ssize_t bytesReceived = recv(sockfd, &c, sizeof(c), 0);
        if (bytesReceived == -1)
        {
            perror("recv failed");
            exit(EXIT_FAILURE);
        }
        else if (bytesReceived == 0)
        {
            break;
        }

        if (c == '\n')
        {
            buffer[bytesRead] = '\0'; // Reached newline, stop reading
            break;
        }
        else
        {
            buffer[bytesRead++] = c;
        }
    }
    return bytesRead;
}