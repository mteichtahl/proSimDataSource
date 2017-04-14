#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <zlog.h>

#include <signal.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "PoKeysLib.h"

int main()
{
    printf("Starting\n");
    const char *hostname = "192.168.2.2";
    const char *portname = "8091";
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;
    struct addrinfo *res = 0;
    int err = getaddrinfo(hostname, portname, &hints, &res);
    if (err != 0)
    {
        printf("failed to resolve remote socket address (err=%d)", err);
        return 1;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1)
    {
        printf("%s", strerror(errno));
        return 1;
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        printf("%s", strerror(errno));
        return 1;
    }

    char buffer[256];
    for (;;)
    {
        ssize_t count = read(fd, buffer, sizeof(buffer));
        if (count < 0)
        {
            if (errno != EINTR)
            {
                printf("%s", strerror(errno));
                return 1;
            }
        }
        else if (count == 0)
        {
            break;
        }
        else
        {
            write(STDOUT_FILENO, buffer, count);
        }
    }
    close(fd);
}