#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define BUFSIZE 1024

int main(int argc, char* argv[]) {

    int sockfd, port, n;
    struct sockaddr_in serveraddr;
    struct hostent* server;

    char buf[BUFSIZE];
    
    if (argc < 3) {
        fprintf(stderr, "usage %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[2]);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error on socket");
        exit(1);
    }

    if (!(server = gethostbyname(argv[1]))) {
        perror("no such host");
        exit(1);
    }

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("error on connect");
        exit(1);
    }

    bzero(buf, BUFSIZE);
    fgets(buf, BUFSIZE-1, stdin);

    n = write(sockfd, buf, strlen(buf));
    if (n < 0) {
        perror("error on write");
        exit(1);
    }

    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE-1);

    if (n < 0) {
        perror("error on read");
        exit(1);
    }
    
    printf("%s\n", buf);
    
    close(sockfd);

    return 0;
}
