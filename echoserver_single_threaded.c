#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

void error(char* msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {

    int listenfd;
    int connfd;
    int port;
    unsigned int clientlen;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    char buf[BUFSIZE];
    struct hostent* hostp;
    char* hostaddrp;
    int optval;
    int n;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("error opening socket");
    }

    optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port);

    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        error("error on bind");
    }

    if (listen(listenfd, 10) < 0) {
        error("error on listen");
    }

    clientlen = sizeof(clientaddr);

    printf("listening on %d\n", port);

    /* accept loop */
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen)) < 0) {
            error("error on accept");
        }

        hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (!hostp) {
            error("error on gethostbyaddr");
        }

        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (!hostaddrp) {
            error("error on inet_ntoa");
        }
        printf("connection with %s (%s)\n", hostp->h_name, hostaddrp);

        bzero(buf, BUFSIZE);
        n = read(connfd, buf, BUFSIZE);
        if (n < 0) {
            error("error on recv");
        }

        printf("received %d bytes %s", n, buf);

        n = write(connfd, buf, strlen(buf));
        if (n < 0) {
            error("error on send");
        }

        close(connfd);
    }

    return 0;
}
