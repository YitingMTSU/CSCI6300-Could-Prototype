#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_LEN 1024
#define PORT 3303
#define SERVER_IP "127.0.0.1"

void main() {

  int clientSocket;
  struct sockaddr_in serverAddr;
  char buffer[BUFFER_LEN];

  clientSocket = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serverAddr, '\0', sizeof(serverAddr));

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

  recv(clientSocket, buffer, BUFFER_LEN, 0);

  printf("The data rev : %s\n", buffer);
  
}
