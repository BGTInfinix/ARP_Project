#ifndef SOCKET_H
#define SOCKET_H

#define AX_TIMEUP -2
#define AX_ECONNABORTED -3
#define AX_ENOTSOCK -4

void my_close(int fd);
int readUntilNewline(int sockfd, char *buffer, size_t bufferSize);

#endif /* SOCKET_H */