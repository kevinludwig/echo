#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/errno.h>
#include <fcntl.h>

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

    int i;
    int ready;
    int running = 1;
    int maxfd;
    int conn_close;
    fd_set master_set, working_set;

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

    /* set socket to nonblocking: child sockets inherit this */
    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

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

    /* initialize fd set with listenfd */
    FD_ZERO(&master_set);
    maxfd = listenfd;
    FD_SET(listenfd, &master_set);

    /* accept loop */
    while (running) {
        memcpy(&working_set, &master_set, sizeof(master_set));

        /* wait for a readable socket */
        if ((ready = select(maxfd + 1, &working_set, NULL, NULL, NULL)) < 0) {
            perror("error in select");
            break;
        }

        /* find sockets with data in fdset */
        for (i = 0; i <= maxfd && ready > 0; i++) {
            if (FD_ISSET(i, &working_set)) {
                ready -= 1;

                /* handle accept socket readable */
                if (i == listenfd) {
                    do {
                        if ((connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen)) < 0) {
                            if (errno != EWOULDBLOCK) {
                                perror("error on accept");
                                running = 0;
                            }
                            break;
                        }

                        hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
                        if (!hostp) {
                            perror("error on gethostbyaddr");
                            running = 0;
                            break;
                        }

                        hostaddrp = inet_ntoa(clientaddr.sin_addr);
                        if (!hostaddrp) {
                            perror("error on inet_ntoa");
                            running = 0;
                            break;
                        }
                        printf("connection with %s (%s)\n", hostp->h_name, hostaddrp);

                        FD_SET(connfd, &master_set);
                        if (connfd > maxfd) {
                            maxfd = connfd;
                        }
                    } while (connfd > 0);
                } 
                /* handle connection socket readable */
                else {
                    int conn_close = 0;
                    do {
                        bzero(buf, BUFSIZE);
                        n = read(i, buf, sizeof(buf));
                        if (n < 0) {
                            if (errno != EWOULDBLOCK) {
                                perror("error in read");
                                conn_close = 1;
                            }
                            break;
                        }

                        if (n == 0) {
                            conn_close = 1;
                            break;
                        }

                        printf("received %d bytes %s", n, buf);

                        n = write(i, buf, strlen(buf));
                        if (n < 0) {
                            perror("error on write");
                            conn_close = 1;
                            break;
                        }
                    } while (1);

                    if (conn_close) {
                        close(i);
                        FD_CLR(i, &master_set);
                        if (i == maxfd) {
                            while (FD_ISSET(maxfd, &master_set) == 0) {
                                maxfd -= 1;
                            }
                        }
                    }
                }
            }
        }
    } 

    for (i = 0; i < maxfd; i++) {
        if (FD_ISSET(i, &master_set)) {
            close(i);
        }
    }

    return 0;
}
