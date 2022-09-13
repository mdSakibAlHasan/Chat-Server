/*
 * server.c -- a stream socket server demo
 */

/*
 * The steps involved in establishing a socket on the server side are as follows:
 *  1. Create a socket with the socket() system call
 *  2. Bind the socket to an address using the bind() system call. For a server socket
 *     on the Internet, an address consists of a port number on the host machine.
 *  3. Listen for connections with the listen() system call
 *  4. Accept a connection with the accept() system call. This call typically blocks
 *     until a client connects with the server.
 *  5. Send and receive data
 *
 *  Ref:
 *   1. https://www.linuxhowtos.org/C_C++/socket.htm
 *   2. http://www.mario-konrad.ch/blog/programming/multithread/tutorial-04.html
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MYPORT 5050      // the port users will be connecting to
#define BACKLOG 10       // how many pending connections queue will hold
#define MAXDATASIZE 256  // max number of bytes we can get at once 
#define MAXCLIENT 20

void *dostuff(void *); //the thread function

struct client_struct{
    char client_name[8];
    int socket_number;
};

struct client_struct client_info[MAXCLIENT];
int number_of_client=0;

int main(void)
{
  int sockfd, new_sockfd;        // listen on sockfd, new connection on new_sockfd
  struct sockaddr_in my_addr;    // my address information
  struct sockaddr_in their_addr; // connector's address information
  int sin_size;
  pthread_t thread_id;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("ERROR opening socket"); exit(1);  }

  bzero((char *) &my_addr, sizeof(my_addr)); // zero the rest of the struct

  my_addr.sin_family = AF_INET;         // host byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
  my_addr.sin_port = htons(MYPORT);     // short, network byte order

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0) {
      perror("ERROR on binding"); exit(1);  }

  if (listen(sockfd, BACKLOG) < 0) {
      perror("ERROR on listen"); exit(1); }

  sin_size = sizeof(struct sockaddr_in);

  while (1) {
    if ((new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) < 0) {
        perror("ERROR on accept"); exit(1);  }
    printf("server: got connection from %s and port %d\n",inet_ntoa(their_addr.sin_addr),their_addr.sin_port);

    // start child thread
    if( pthread_create( &thread_id , NULL ,  dostuff , (void*) &new_sockfd) < 0) {
        perror("ERROR create thread"); exit(1);  }

    pthread_detach(thread_id);  // don't track it

  } /* end of while */

  close(sockfd);
  return 0;
}


int extract_name_message(char  str[])
{
  printf("%s\n",str);
  if(str[0] != '@'){
    printf("Error client name message\n");
    return -1;
  }

  char client_name[8],index=0,i=1;
  while(str[i] != 32){
      client_name[index++] = str[i++];
  }
  client_name[index] = '\0';

  for(index=0;str[i];i++){
    str[index++] = str[i];
  }
  str[index] = '\0';

  for(int k=0;k<number_of_client;k++){
    if(strcmp(client_info[k].client_name,client_name) == 0){
      return k;//client_info[k].socket_number;
    }
  }

  if(strcmp(client_name,"all") == 0)
      return -2;

  return -1;
  //send_specific(client_name,str);
  //printf("%s\t%s\n",client_name,str);
}


void send_all(char str[])
{
    for(int i=0;i<number_of_client;i++){
        if (write(client_info[i].socket_number, str, strlen(str)) < 0) {
              printf("ERROR writing to %s\n",client_info[i].client_name);
        }

    }
}



void sent_message_client(int client_index,char str[])
{
    int ci = extract_name_message(str);
    if(ci == -1){
        char warning_message[]={"Client not found\n"};
        write(client_info[client_index].socket_number, warning_message, strlen(warning_message));
    }
    else if(ci == -2)
        send_all(str);
    //char str[MAXDATASIZE];

    if (write(client_info[ci].socket_number, str, strlen(str)) < 0) {
      perror("ERROR writing to socket"); exit(1);  }

      return ;

}


// void save_name(int sock_number,char str_name[])
// {
//     strcpy(client_info[number_of_client].client_name,str_name);
//     client_info[number_of_client++].socket_number = sock_number;

//     return ;
// }




void *dostuff (void *socket_desc)
{
  int n = number_of_client;
  char *str, buffer[MAXDATASIZE];
  //Get the socket descriptor
  int sock = *(int*)socket_desc;
  printf("socket number %d\n",sock);

  bzero(buffer,MAXDATASIZE);
  if (read(sock,buffer,MAXDATASIZE-1) < 0) {                //recieve name from client and save it struct
      perror("ERROR reading from socket"); exit(1);  }
  printf("Here is the connection : %s\n",buffer);
  strcpy(client_info[number_of_client].client_name,buffer);
    client_info[number_of_client++].socket_number = sock;

    while(1){
  str = "Please enter the message: ";
  if (write(sock, str, strlen(str)) < 0) {
      perror("ERROR writing to socket"); exit(1);  }



  bzero(buffer,MAXDATASIZE);
  if (read(sock,buffer,MAXDATASIZE-1) < 0) {
      perror("ERROR reading from socket"); exit(1);  }
  printf("Here is the message: %s\n",buffer);

    sent_message_client(n,buffer);      //sent mesage other client
    }

//   str = "I got your message \n";
//   if (write(sock, str, strlen(str)) < 0) {
     // perror("ERROR writing to socket"); exit(1);  }

  close(sock);
  pthread_exit(NULL);
}



