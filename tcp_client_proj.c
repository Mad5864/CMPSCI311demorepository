//Jaden's Rough Draft Code
// TCP client program
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define SERV_TCP_PORT 5000 /* server's port */
#define Message 80
void * Read_For_Clients(void *arg)
{
    int sockfd = *(int*)arg;
    char buf[Message];

    int len;
    for(;;)
    {
    len = read(sockfd, buf, sizeof(buf) - 1);
    if (len <= 0)
    {
        printf("Error:");
        exit(0);
    }
    buf[len] = 0;
    printf("\n%s\n ", buf);
    }
    return NULL;

}

int main(int argc, char *argv[])
{
    int sockfd;

    struct sockaddr_in serv_addr;

    char *serv_host = "localhost";

    struct hostent *host_ptr;

    int port;

    int buff_size = 0;
    /* command line: client [host [port]] */
    if(argc >= 2)
        serv_host = argv[1]; /* read the host if provided */
    
    if(strcmp(serv_host, "localhost") == 0)
        serv_host = "127.0.0.1";

    if(argc == 3)
        sscanf(argv[2], "%d", &port); /* read the port if provided */
    else
        port = SERV_TCP_PORT;
    /* get the address of the host */
    if((host_ptr = gethostbyname(serv_host)) == NULL) {
     perror("gethostbyname error");

     exit(1);

  }

 

  if(host_ptr->h_addrtype !=  AF_INET) {
     perror("unknown address type");

     exit(1);

  }

 

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;

  serv_addr.sin_addr.s_addr =

     ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;

  serv_addr.sin_port = htons(port);
  /* open a TCP socket */
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
     perror("can't open stream socket");

     exit(1);
  }
  /* connect to the server */
  if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     perror("can't connect to server");

     exit(1);
  }
  /* write a message to the server */
  pthread_t temp;
  pthread_create(&temp, NULL, Read_For_Clients, (void *)&sockfd);
  pthread_detach(temp);
  char buf[Message];
  for(;;)
  {
    
    fgets(buf, sizeof(buf), stdin);

    write(sockfd, buf, strlen(buf));
  }
  close(sockfd);
}