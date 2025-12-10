//CMPSC 311 FINAL PROJECT
//Main Developers: Jaden Clay, Michael DeSalis, Ariana Sookoo
// TCP server program

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERV_TCP_PORT 5000
#define MAX_SIZE 200
#define Max_Client_Size 3

int clientNum[Max_Client_Size] = {0};
char usernames[Max_Client_Size][50] = {0};

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void Broadcast(char *message)
{
    pthread_mutex_lock(&mtx);

    for (int i = 0; i < Max_Client_Size; i++)
    {
        if (clientNum[i] > 0)
        {
            write(clientNum[i], message, strlen(message));
        }
    }

    pthread_mutex_unlock(&mtx);
}

void remove_client(int sockfd)
{
    pthread_mutex_lock(&mtx);
    for (int i = 0; i < Max_Client_Size; i++)
    {
        if (clientNum[i] == sockfd)
        {
            printf("User left: %s\n", usernames[i]);

            char msg[200];
            snprintf(msg, sizeof(msg), "%s has left the chat.\n", usernames[i]);
            Broadcast(msg);

            clientNum[i] = 0;
            usernames[i][0] = '\0';
            break;
        }
    }
    pthread_mutex_unlock(&mtx);
}

void *Read_For_clients(void *arg)
{
    int sockfd = *(int*)arg;
    char buf[MAX_SIZE];
    int len;

    int index = -1;

    // Find client index
    pthread_mutex_lock(&mtx);
    for (int i = 0; i < Max_Client_Size; i++)
    {
        if (clientNum[i] == sockfd)
            index = i;
    }
    pthread_mutex_unlock(&mtx);

    for (;;)
    {
        len = read(sockfd, buf, MAX_SIZE - 1);
        if (len <= 0)
        {
            remove_client(sockfd);
            close(sockfd);
            pthread_exit(NULL);
        }

        buf[len] = 0;

        // Username setup: "__username__:alice"
        if (strncmp(buf, "__username__:", 13) == 0)
        {
            strcpy(usernames[index], buf + 13);
            printf("User joined: %s\n", usernames[index]);

            char joined[200];
            snprintf(joined, sizeof(joined), "%s has joined the chat.\n", usernames[index]);
            Broadcast(joined);

            continue;
        }

        // Normal message
        Broadcast(buf);
    }

    return NULL;
}

int main()
{
    int sockfd, newsockfd, clilen;
    struct sockaddr_in cli_addr, serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("cannot open socket");
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind error");
        exit(1);
    }

    listen(sockfd, Max_Client_Size);

    printf("Server running on port %d...\n", SERV_TCP_PORT);

    for (;;)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

        if (newsockfd < 0)
        {
            perror("accept error");
            continue;
        }

        // Add to client list
        pthread_mutex_lock(&mtx);
        int placed = 0;
        for (int i = 0; i < Max_Client_Size; i++)
        {
            if (clientNum[i] == 0)
            {
                clientNum[i] = newsockfd;
                usernames[i][0] = '\0';
                placed = 1;
                break;
            }
        }
        pthread_mutex_unlock(&mtx);

        if (!placed)
        {
            char *full = "Server full.\n";
            write(newsockfd, full, strlen(full));
            close(newsockfd);
            continue;
        }

        pthread_t t;
        pthread_create(&t, NULL, Read_For_clients, (void*)&newsockfd);
        pthread_detach(t);
    }

    close(sockfd);
    return 0;
}
