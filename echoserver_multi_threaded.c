#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#define BUFSIZE 1024

void error(char* msg) {
    perror(msg);
    exit(1);
}

typedef struct request_data {
    int connfd;
    struct sockaddr_in clientaddr;
} request_data_t;

void* request_handler(void* arg) {
    request_data_t* r = (request_data_t*)arg;
    
    struct hostent* hostp;
    char* hostaddrp;
    int n;
    char buf[BUFSIZE];

    hostp = gethostbyaddr((const char *) &r->clientaddr.sin_addr.s_addr, sizeof(r->clientaddr.sin_addr.s_addr), AF_INET);
    if (!hostp) {
        error("error on gethostbyaddr");
    }

    hostaddrp = inet_ntoa(r->clientaddr.sin_addr);
    if (!hostaddrp) {
        error("error on inet_ntoa");
    }
    printf("connection with %s (%s)\n", hostp->h_name, hostaddrp);

    bzero(buf, BUFSIZE);
    n = read(r->connfd, buf, BUFSIZE);
    if (n < 0) {
        error("error on recv");
    }

    printf("received %d bytes %s", n, buf);

    n = write(r->connfd, buf, strlen(buf));
    if (n < 0) {
        error("error on send");
    }

    close(r->connfd);
    free(r);

    return NULL;
}

int main(int argc, char** argv) {

    int listenfd;
    int connfd;
    int port;
    unsigned int clientlen;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    request_data_t* r;
    pthread_t tid;
    int optval;

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
        r = (request_data_t *) malloc(sizeof(request_data_t));
        r->connfd = connfd;
        r->clientaddr = clientaddr;

        pthread_create(&tid, NULL, request_handler, (void *)r);
    }

    return 0;
}
