//CMPSC 311 FINAL PROJECT
//Developers: Jaden Clay, Michael DeSalis, Ariana Sookoo
//TCP server program

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERV_TCP_PORT 5000 /* server's port number */
#define MAX_SIZE 80
#define Max_Client_Size 3

// Array to hold client socket file descriptors
int clientNum[Max_Client_Size] = {0}; 
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// Global variables
int sockfd, clilen, len, port;
struct sockaddr_in cli_addr, serv_addr;
char string[MAX_SIZE];

// Function to send message to all connected clients
void message_to_clients(char *message, int sender)
{
    /* This first takes the given message then pthread_mutex_lock
    is used to stop any race conditions then the message is written to each client. */
    pthread_mutex_lock(&mtx);
    for (int x = 0; x < Max_Client_Size; x++)
    {
        if (clientNum[x] != 0 && clientNum[i] != sender)
        {
        send(clientNum[x], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mtx);
}

// Function to read messages from clients, handle clients
void *read_for_clients(void *arg)
{
    int sockfd = *(int*)arg;
    free(arg); //frees up the memory that was malloc-ed

    char string[MAX_SIZE];

    for(;;)
    {
        int len = read(sockfd, string, sizeof(string)- 1);

        if (len == 0) {
            printf("Client disconnected.\n");
            close(sockfd);
            remove_client(sockfd);
            break; 
        } else if (len < 0) {
            perror("Error: Read failed.");
            close(sockfd);
            remove_client(sockfd);
            break; 
        }
        string[len] = '\0';
        printf("%s\n", string);

        message_to_clients(string, sockfd);
    }
    return NULL;
}

// Function to remove a client from the clientNum array
void remove_clients(int sockfd) {
    pthread_mutex_lock(&mtx);

    for (int i = 0; i < Max_Client_Size; ++i) {

        if (clientNum[i] == sockfd) { // found the client to remove
            clientNum[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&mtx);
}

int main(int argc, char *argv[]) 
{
    //command line: server [port_number]
    if (argc == 2)
        sscanf(argv[1], "%d", &port); /* read the port number if provided */
    else 
        port = SERV_TCP_PORT;

    /* open a TCP socket (an Internet stream socket) */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: Cannot open stream socket.");
        exit(1);
    }

    // bind the local address to the socket so that the client can send to the server
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) {
        perror("Error: Cannot bind local address.");
        exit(1);
    }

    //listen to the socket
    listen(sockfd, Max_Client_Size); // maximum number of clients
    
    for (;;) {
        //wait for a connection from a client; this is an iterative server
        clilen = sizeof(cli_addr);
        
        int *newsockfd = malloc(sizeof(int)); // allocate memory for each new socket file descriptor
        *newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (*newsockfd < 0) {
            perror("Error: Cannot accept connection.");
            free(newsockfd); // free allocated memory on error
            continue;
        }

        for (int x = 0; x < Max_Client_Size; x++)
        {
            if (clientNum[x] <= 0)
            {
                clientNum[x] = newsockfd;
                break;
            }
            
        }

        pthread_t temp;
        pthread_create(&temp, NULL, Read_For_clients, (void*)&newsockfd);
        pthread_detach(temp);
    }
    // close(newsockfd);
}

/*To run the server: 
gcc tcp_server.c -lpthread -o server
./tcp_server [port_number] */