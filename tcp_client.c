//CMPSC 311 FINAL PROJECT
//Developers: Jaden Clay, Michael DeSalis, Ariana Sookoo
// TCP client program

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define SERV_TCP_PORT 5000
#define Message 80

// Thread function: read messages sent by server
void *read_for_clients(void *arg)
{
    int sockfd = *(int *)arg;
    char buf[Message];

    while (1)
    {
        int len = read(sockfd, buf, sizeof(buf) - 1);
        if (len <= 0)
        {
            printf("\nServer disconnected.\n");
            exit(0);
        }
        buf[len] = 0;
        printf("\n%s\n", buf);
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;
    char *serv_host = "127.0.0.1";
    int port = SERV_TCP_PORT;

    if (argc >= 2)
        serv_host = argv[1];
    if (argc == 3)
        port = atoi(argv[2]);

    // Resolve host
    struct hostent *host_ptr = gethostbyname(serv_host);
    if (!host_ptr)
    {
        perror("gethostbyname error");
        exit(1);
    }

    // Build server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr =
        ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
    serv_addr.sin_port = htons(port);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Cannot open socket");
        exit(1);
    }

    // Connect
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Cannot connect to server");
        exit(1);
    }

    printf("Connected to server!\n");

    // Start read thread
    pthread_t rthread;
    pthread_create(&rthread, NULL, read_for_clients, &sockfd);
    pthread_detach(rthread);

    // Username
    char username[20];
    char buf[Message];
    char formatted[Message + sizeof(username) + 4];     

    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    // Message loop
    while (1)
    {
        printf("Type message: ");
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = 0;

        char formatted[Message + 20];
        snprintf(formatted, sizeof(formatted), "%s: %s", username, buf);

        write(sockfd, formatted, strlen(formatted));
    }

    close(sockfd);
}
