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
    char client_name[20];
    int socket_number;
    int active_status;
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
      perror("ERROR opening socket"); 
      exit(1);  
  }

  bzero((char *) &my_addr, sizeof(my_addr)); // zero the rest of the struct

  my_addr.sin_family = AF_INET;         // host byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
  my_addr.sin_port = htons(MYPORT);     // short, network byte order

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0) {
      perror("ERROR on binding"); 
      exit(1);
  }

  if (listen(sockfd, BACKLOG) < 0) {
      perror("ERROR on listen"); 
      exit(1); 
  }

  sin_size = sizeof(struct sockaddr_in);

  while (1) {
    if ((new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) < 0) {
        perror("ERROR on accept"); 
        exit(1);  
    }
    printf("server: got connection from %s, port %d and ",inet_ntoa(their_addr.sin_addr),their_addr.sin_port);

    // start child thread
    if( pthread_create( &thread_id , NULL ,  dostuff , (void*) &new_sockfd) < 0) {
        perror("ERROR create thread");
        exit(1);  
    }

    pthread_detach(thread_id);  // don't track it

  } /* end of while */

  close(sockfd);
  return 0;
}


int extract_name_message(char  str[], int client_index)
{
  if(str[0] != '@'){                    //message must be start with @
    printf("Error message syntax\n");
    return -3;
  }

  char client_name[MAXCLIENT],sending_mesage[MAXDATASIZE],index=0,i=1;

  while(str[i] != 32 && str[i] != '\n'){                      //separate name and message
      client_name[index++] = str[i++];
  }
  client_name[index] = '\0';

  for(index=0;str[i];i++){              //aline message
    str[index++] = str[i];
  }
  str[index] = '\0';

  strcpy(sending_mesage,client_info[client_index].client_name);     //aline message format
  strcat(sending_mesage,"-> ");
  strcat(sending_mesage,str);
  strcpy(str,sending_mesage);



  for(int k=0;k<number_of_client;k++){                              //serach sender index and socket number
    if(strcmp(client_info[k].client_name,client_name) == 0 && client_info[k].active_status == 1){
      return k;//client_info[k].socket_number;
    }
  }

  if(strcmp(client_name,"all") == 0)
      return -2;

  return -1;
}


void send_all(char str[],int client_index)
{
    for(int i=0;i<number_of_client;i++){
      if(client_index == i || client_info[i].active_status == 0)                   //self message not send
        continue;

      if (write(client_info[i].socket_number, str, strlen(str)) < 0) {        //send mesage all client listed
          printf("ERROR writing to %s\n",client_info[i].client_name);
      }
    }
}



void sent_message_client(int client_index,char str[])
{
    int sock_index = extract_name_message(str,client_index);
    if(sock_index == -1){
        char warning_message[]={"Server-> Client not found\n"};
        write(client_info[client_index].socket_number, warning_message, strlen(warning_message));
    }
    if(sock_index == -3){
        char warning_message[]={"Server-> Syntex error\n"};
        if(strcmp(str,"exit\n")==0)
            write(client_info[client_index].socket_number, "exit",5);
        else
            write(client_info[client_index].socket_number, warning_message, strlen(warning_message));
    }
    else if(sock_index == -2)
        send_all(str,client_index);
    
    if (write(client_info[sock_index].socket_number, str, strlen(str)) < 0) {
      perror("ERROR writing to socket"); 
      exit(1);  
    }

      return ;
}



void *dostuff (void *socket_desc)
{
  int n = number_of_client;
  char *str, buffer[MAXDATASIZE];
  int sock = *(int*)socket_desc;


  printf("socket number %d\n",sock);
  bzero(buffer,MAXDATASIZE);

  if (read(sock,buffer,MAXDATASIZE-1) < 0) {                //recieve name from client and save it struct
      perror("ERROR reading from socket"); 
      exit(1);  
  }
  
  strcpy(client_info[number_of_client].client_name,buffer);       //list client information
  client_info[number_of_client].active_status = 1;
  client_info[number_of_client++].socket_number = sock;

  str = "Server-> Connection established\n";
  if (write(sock, str, strlen(str)) < 0) {
      perror("ERROR writing to socket"); 
      exit(1);  
  }


  while(1){
    bzero(buffer,MAXDATASIZE);
    if (read(sock,buffer,MAXDATASIZE-1) < 0) {
      perror("ERROR reading from socket"); 
      exit(1);  
    }
    printf("Here is the message: %s \t %ld\n",buffer,strlen(buffer));
    sent_message_client(n,buffer);      //sent mesage other client

    if(strcmp(buffer,"exit\n")==0){
        int i;
        for(i=0;i<number_of_client;i++)
          if(client_info[i].socket_number == sock)  break;
        
        client_info[i].active_status = 0;
        client_info[i].socket_number = -1;
        printf("Connection closed %s socket %d",client_info[i].client_name,client_info[i].socket_number);
        write(sock, buffer, strlen(buffer));
        pthread_exit(NULL);
        //continue;
    }

    
  }

  close(sock);
  pthread_exit(NULL);
}

