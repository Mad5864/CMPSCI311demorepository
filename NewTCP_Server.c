//Jaden's Rough Draft Code
// TCP server program
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

int clientNum[Max_Client_Size] = {0};
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
void Message_To_Clients(char *message)
{
    /* This first takes the given message then pthread_mutex_lock is used to stop any race conditions then the message is written to each client. */
    pthread_mutex_lock(&mtx);
    for (int x = 0; x < Max_Client_Size; x++)
    {
        if (clientNum[x] > 0)
        {
        write(clientNum[x], message, strlen(message));
        }
        
    }
    pthread_mutex_unlock(&mtx);

}
void * Read_For_clients(void *arg)
{
    int sockfd = *(int*)arg;
    char string[MAX_SIZE];

    int len;
    for(;;)
    {
    len = read(sockfd, string, MAX_SIZE - 1);

    string[len] = 0;

    Message_To_Clients(string);
    }
    return NULL;

}
int main(int argc, char *argv[]) 

{

    int sockfd, newsockfd, clilen;

    struct sockaddr_in cli_addr, serv_addr;

    int port;

    char string[MAX_SIZE];

    int len;

    /* command line: server [port_number] */

    if (argc == 2)

        sscanf(argv[1], "%d", &port); /* read the port number if provided */

    else 

        port = SERV_TCP_PORT;

    /* open a TCP socket (an Internet stream socket) */

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

        perror("can't open stream socket");

        exit(1);

    }




/* bind the local address, so that the client can send to the server */

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) {

        perror("can't bind local address");

        exit(1);

    }
 /* listen to the socket*/

    listen(sockfd, Max_Client_Size);

    for (;;) {

        /* wait for a connection from a client; this is an iterative server */

        clilen = sizeof(cli_addr);

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {

            perror("can't accept connection");

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
    close(newsockfd);

}