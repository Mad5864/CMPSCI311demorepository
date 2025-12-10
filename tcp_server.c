//CMPSC 311 FINAL PROJECT
//Developers: Jaden Clay, Michael DeSalis, Ariana Sookoo
// TCP server program

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERV_TCP_PORT 5000
#define MAX_SIZE 80
#define MAX_CLIENT_SIZE 3

// Connected client sockets
int clientNum[MAX_CLIENT_SIZE] = {0};
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// Broadcast a message to all clients except sender
void message_to_clients(char *message, int sender)
{
    pthread_mutex_lock(&mtx);
    for (int i = 0; i < MAX_CLIENT_SIZE; i++)
    {
        if (clientNum[i] != 0 && clientNum[i] != sender)
        {
            send(clientNum[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mtx);
}

// Remove a disconnected client
void remove_client(int sockfd)
{
    pthread_mutex_lock(&mtx);
    for (int i = 0; i < MAX_CLIENT_SIZE; i++)
    {
        if (clientNum[i] == sockfd)
        {
            clientNum[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&mtx);
}

// Thread function: read from a specific client
void *read_for_clients(void *arg)
{
    int sockfd = *(int *)arg;
    free(arg);

    char msg[MAX_SIZE];

    while (1)
    {
        int len = read(sockfd, msg, sizeof(msg) - 1);

        if (len <= 0)
        {
            printf("Client disconnected.\n");
            close(sockfd);
            remove_client(sockfd);
            return NULL;
        }

        msg[len] = '\0';
        printf("%s\n", msg);
        message_to_clients(msg, sockfd);
    }
}

int main(int argc, char *argv[])
{
    int sockfd, port;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc == 2)
        port = atoi(argv[1]);
    else
        port = SERV_TCP_PORT;

    // Create server socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Configure server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // Bind
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    // Listen
    listen(sockfd, MAX_CLIENT_SIZE);
    printf("Server listening on port %d\n", port);

    while (1)
    {
        socklen_t clilen = sizeof(cli_addr);

        // Allocate new socket per client
        int *newsockfd = malloc(sizeof(int));

        *newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (*newsockfd < 0)
        {
            perror("Accept failed");
            free(newsockfd);
            continue;
        }

        // Add to client list
        pthread_mutex_lock(&mtx);
        for (int i = 0; i < MAX_CLIENT_SIZE; i++)
        {
            if (clientNum[i] == 0)
            {
                clientNum[i] = *newsockfd;
                break;
            }
        }
        pthread_mutex_unlock(&mtx);

        // Create thread for the new client
        pthread_t thread;
        pthread_create(&thread, NULL, read_for_clients, newsockfd);
        pthread_detach(thread);
    }

    close(sockfd);
    return 0;
}
