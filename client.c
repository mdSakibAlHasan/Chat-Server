/*
 * client.c -- a stream socket client demo
 */

/*
 * The steps involved in establishing a socket on the client side are as follows:
 *  1. Create a socket with the socket() system call
 *  2. Connect the socket to the address of the server using the connect() system call
 *  3. Send and receive data. There are a number of ways to do this, but the simplest 
 *     is to use the read() and write() system calls.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define PORT 5050       // the port client will be connecting to 
#define MAXDATASIZE 256 // max number of bytes we can get at once 

char client_name[20];

void *dostuff (void *socket_desc)
{
  char *str, buffer[MAXDATASIZE];
  //Get the socket descriptor
  int sock = *(int*)socket_desc,numbytes;
while(1){
   bzero(buffer,MAXDATASIZE);
    if ((numbytes=read(sock, buffer, MAXDATASIZE-1)) < 0) {
        perror("ERROR reading from socket");  exit(0);  }
        buffer[numbytes] = '\0';

    if(strlen(buffer) !=0)    
        printf("Recieve %s\n",buffer);
}


}


int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE],temp[MAXDATASIZE];
    struct hostent *server;
    struct sockaddr_in server_addr; // connector's address information 
    pthread_t thread_id;

    if (argc != 3) {
        printf("usage: client hostname\n"); exit(1);  
    }
    

    if ((server=gethostbyname(argv[1])) == NULL) {  // get the host info 
        printf("ERROR, no such host\n");  exit(0);  }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR opening socket");  exit(0);  }

    bzero((char *) &server_addr, sizeof(server_addr));  // zero the rest of the struct 
    server_addr.sin_family = AF_INET;    // host byte order 
    server_addr.sin_port = htons(PORT);  // short, network byte order 
    server_addr.sin_addr = *((struct in_addr *)server->h_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
        perror("ERROR connecting");  exit(0);  }

    if (write(sockfd,argv[2],strlen(argv[2])) < 0) {        //sent client name to server
        perror("ERROR writing to socket"); exit(0);  
    }


   if( pthread_create( &thread_id , NULL ,  dostuff , (void*) &sockfd) < 0) {           //create thread for read
        perror("ERROR create thread"); exit(1);  }

    while(1){
    bzero(buf,MAXDATASIZE);
    fgets(buf,MAXDATASIZE-1,stdin);

    // strcpy(temp,argv[2]);
    // int length = strlen(temp);
    // temp[length] = 32;          //add a space after client name
    // temp[length+1] = '\0';
    // strcat(temp,buf);

    if (write(sockfd,buf,strlen(buf)) < 0) {
        perror("ERROR writing to socket"); exit(0);  }

    }

   

    close(sockfd);

    return 0;
}
