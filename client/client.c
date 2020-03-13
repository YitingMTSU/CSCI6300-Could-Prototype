#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <unistd.h>
#include "client.h" //the client header file

int main() {

  int clientSocket;
  struct sockaddr_in serverAddr;
  char buffer[BUFFER_LEN];
  memset(&buffer, '\0', sizeof(buffer));
  
  if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);

  //Convert IPv4 and IPv6 address from text to binary form
  if (inet_pton(AF_INET, SERVER_HERSCHEL_IP, &serverAddr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("\nConnection Failed \n");
  }

  //login into the server
  int logInS = login(clientSocket,buffer);

  while (logInS) {
    char *hello = "Hello from client";
    send(clientSocket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    recv(clientSocket, buffer, BUFFER_LEN, 0);
    printf("The data rev : %s\n", buffer);
    free(hello);
    break;
  }

  return 0;
}


int login(int socket, char* buffer) {
  recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s\n",buffer);
  memset(&buffer, '\0', sizeof(buffer));
  recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s\n",buffer);
  memset(&buffer, '\0', sizeof(buffer));

  //get user name
  char userName[USERNAME_LEN];
  memset(&userName,'\0',sizeof(userName));
  int count = 0;
  fgets(userName,USERNAME_LEN,stdin);
  send(socket,userName,strlen(userName),0);

  int flag;
  recv(socket,buffer,BUFFER_LEN,0);
  flag = buffer[0] - '0';
  memset(&buffer, '\0', sizeof(buffer));

  
  if (flag == 0) { //didn't find the user
    recv(socket,buffer,BUFFER_LEN,0);
    printf("%s\n",buffer);
  } else { //find the user, next input the password
    echo(false);
    char password[PASSWORD_LEN];
    fgets(password,PASSWORD_LEN,stdin);
    echo(true);
    send(socket,password,strlen(password),0);
    recv(socket,buffer,BUFFER_LEN,0);
    int pass = buffer[0] - '0';
    
  }
  return 0;
}
  

void echo(bool on) {
  struct termios settings;
  tcgetattr( STDIN_FILENO, &settings);
  settings.c_lflag = on
    ? (settings.c_lflag |   ECHO)
    : (settings.c_lflag & ~(ECHO));
  tcsetattr( STDIN_FILENO, TCSANOW, &settings);
}
