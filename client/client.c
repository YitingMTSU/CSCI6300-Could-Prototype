#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_LEN 1024
#define PORT 3303
//#define SERVER_IP "192.168.122.1"
#define SERVER_IP "161.45.162.78"

int main() {

  int clientSocket;
  struct sockaddr_in serverAddr;
  char buffer[BUFFER_LEN];

  if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&buffer, '\0', sizeof(buffer));

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);

  //Convert IPv4 and IPv6 address from text to binary form
  if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("\nConnection Failed \n");
  }


  char *hello = "Hello from client";
  send(clientSocket, hello, strlen(hello), 0);
  printf("Hello message sent\n");
  recv(clientSocket, buffer, BUFFER_LEN, 0);
  printf("The data rev : %s\n", buffer);
  
}
