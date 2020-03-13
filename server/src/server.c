#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 3303
#define SERVER_IP "192.168.122.1"

int checkUser(char* userName);

int main() {

  int sockfd, newSocket;
  struct sockaddr_in serverAddr;
  int opt = 1;
  
  //int addrlen
  struct sockaddr_in newAddr;

  socklen_t addr_size;
  char buffer[BUFFER_SIZE];

  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  memset(&serverAddr, '\0', sizeof(serverAddr));


  // Forcefully attaching socket to the port 3303 
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		 &opt, sizeof(opt))) { 
    perror("setsockopt"); 
    exit(EXIT_FAILURE); 
  } 
    

  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  //attaching socket to the port
  if (bind(sockfd, (struct sockaddr*)&serverAddr,
	   sizeof(serverAddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  
  if (listen(sockfd,5) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  addr_size = sizeof(newAddr);
  if ((newSocket = accept(sockfd, (struct sockaddr*)& newAddr,
			  &addr_size)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  memset(&buffer, '\0', sizeof(buffer));
  char* interfaceWelcome = "Welcome To Ranger/Herschel Cloud!";
  char* interfaceUser = "Please enter your username:";
  char* interfacePassword = "Passport:";
  char* interfaceUsage = "Enter The Option You Want: \n1. Read Own File      2. Rewrite Own File\n 3. Read Others' Files 4. EXIT";

  send(newSocket,interfaceWelcome,strlen(interfaceWelcome),0);
  send(newSocket,interfaceUser,strlen(interfaceUser),0);
  read(newSocket,buffer,1024);
  char* userName;
  strcpy(userName,buffer);
  memset(&buffer, '\0', sizeof(buffer));
  int userIn = checkUser(userName);
  while (1) {
    
    read(newSocket,buffer,1024);
    printf("%s\n",buffer);
    strcpy(buffer, "Hello");
    send(newSocket, buffer, strlen(buffer), 0);
    printf("Hello sent\n");
    break;
  }
  return 0;
}

int checkUser(char* userName) {
  return 1;
}
