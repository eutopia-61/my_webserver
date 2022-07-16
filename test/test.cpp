#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("input error");
        exit(-1);
    }

    char *serverIP = argv[1];
    int port = atoi(argv[2]);

    char sendbuffer[1024] = {"hello, i am client!\n"};
    char recvbuffer[1024];

    int sockfd = -1;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
        exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(serverIP);
    
    if (connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "Connect error:%s\n", strerror(errno));
        exit(1);
    }
    while (1)
    {
        int len = strlen(sendbuffer);
        send(sockfd, sendbuffer, len, 0);
        printf("send %s", sendbuffer);

        // len = recv(sockfd, recvbuffer, 1024, 0);
        // if (len < 0)
        // {
        //     fprintf(stderr, "recv Error:%s\a\n", strerror(errno));
        // }
        // else if (len > 0)
        // {
        //     recvbuffer[len] = 0;
        //     printf("recv: %s\n", recvbuffer);
        // }
        sleep(5);
    }

    close(sockfd);
    exit(0);
}
