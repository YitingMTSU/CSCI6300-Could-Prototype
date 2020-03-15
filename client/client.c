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
  memset(buffer, 0, sizeof(buffer));

  //choose the server you want to connect
 SERVERCHOOSE:
  printf("Which Server you want to connect? 1. HERSCHEL OR 2. RANGER\n");
  printf("Type 1 OR 2: ");
  char serverOption = getchar();
  char* serverIP;
  if (serverOption == '1') {
    strcpy(serverIP,SERVER_HERSCHEL_IP);
  } else if (serverOption == '2') {
    strcpy(serverIP,SERVER_RANGER_IP);
  } else {
    printf("Cannot reconginze, choose again!\n");
    goto SERVERCHOOSE;
  }
  
  if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }
  printf("[+]Create socket...\n");
  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);

  //Convert IPv4 and IPv6 address from text to binary form
  if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }
  printf("[+]Convert the IP...\n");

  if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("\nConnection Failed \n");
  }
  printf("[+]Connect to the sever...\n");

  //login into the server
  int logInS = login(clientSocket,buffer);
  memset(buffer, 0, strlen(buffer));
  
  while (logInS) {
    int quit = mainUsageClient(clientSocket, buffer);
    if (quit == 1) break;
  }

  return 0;
}


int login(int socket, char* buffer) {
  //read the interface from server
  //int sig;
  //int messageLen;
  //recv(socket, &messageLen, sizeof(messageLen), 0);
  //printf("the messagelen : %d\n",messageLen);
  //sig = recv(socket, buffer, messageLen, 0);
  recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s\n",buffer);
  bzero(buffer, BUFFER_LEN);
  //sleep(0.5);
  
  //recv(socket, &messageLen, sizeof(messageLen), 0);
  recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s",buffer);
  memset(buffer, 0, BUFFER_LEN);
  
  //get user name
  char userName[USERNAME_LEN];
  memset(userName,0,USERNAME_LEN);
  //int count = 0;
  fgets(userName,USERNAME_LEN,stdin);
  userName[strlen(userName)-1] = 0; //otherwise \n at the end
  send(socket,userName,strlen(userName),0);


  //check the account if exist
  char userExist;
  recv(socket,&userExist,sizeof(userExist),0);
  printf("userExist: %c\n",userExist);
  //memset(&buffer, '0', sizeof(buffer));

  
  if (userExist == '0') { //didn't find the user
    recv(socket,buffer,BUFFER_LEN,0);
    printf("%s\n",buffer);
    memset(buffer, 0, BUFFER_LEN);
    char option; //select option create interface or quit
    echo(false);
    char input[5];
    fgets(input,5,stdin);
    option = input[0];
    //printf("The option you choose %c\n",option);
    send(socket, &option, sizeof(option), 0);
    if (option == '1') { //create the account and enter the password
      //printf("here");
      //receive the first password
      echo(true);
      //printf("bfeore receive buffer %s\n",buffer);
      recv(socket, buffer, BUFFER_LEN, 0);
      printf("%s ",buffer);
      memset(buffer, 0, BUFFER_LEN);
      char passwordFirst[PASSWORD_LEN];
      memset(passwordFirst, 0, PASSWORD_LEN);
      echo(false);
      fgets(passwordFirst,PASSWORD_LEN,stdin);
      passwordFirst[strlen(passwordFirst) - 1] = 0;
      echo(true);
      send(socket, passwordFirst, strlen(passwordFirst), 0);
      printf("\n");

      
      //receive the second password
      recv(socket, buffer, BUFFER_LEN, 0);
      printf("%s\n", buffer);
      memset(buffer,0,BUFFER_LEN);
      char passwordSecond[PASSWORD_LEN];
      memset(passwordSecond, 0, PASSWORD_LEN);
      echo(false);
      fgets(passwordSecond,PASSWORD_LEN,stdin);
      passwordSecond[strlen(passwordSecond) - 1] = 0;
      echo(true);
      send(socket, passwordSecond, strlen(passwordSecond), 0);

      char create;
      recv(socket, &create, sizeof(create), 0);
      if (create == '0') { //create unsucessful
	printf("The password doesn't match. Exit.\n");
	exit(1);
      } else { //create sucessful
	printf("Create the new account: %s\n",userName);
	return 1;
      }
      
      
    } else {
      echo(true);
      recv(socket, buffer, BUFFER_LEN, 0);
      printf("%s\n",buffer);
      memset(buffer, 0, BUFFER_LEN);
      exit(1);//select quit
    }
  } else { //find the user, next input the password
    int count = 1;
  INPUTPASSWORD:
    recv(socket, buffer, BUFFER_LEN, 0);
    printf("%s\n", buffer);
    memset(buffer, 0, BUFFER_LEN);
    
    echo(false);
    char password[PASSWORD_LEN];
    fgets(password,PASSWORD_LEN,stdin);
    password[strlen(password)-1] = '\0';
    echo(true);
    send(socket,password,strlen(password),0);

    char pass;
    recv(socket, &pass, sizeof(pass), 0);
    
    if (pass == '0') {
      recv(socket,buffer,BUFFER_LEN,0);
      printf("%s\n",buffer);
      memset(buffer, 0, strlen(buffer));
      if(count == 3) exit(1);
      count++;
      goto INPUTPASSWORD;
      exit(1);
    } else {
      return 1;
    }
    
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

int mainUsageClient(int socket, char* buffer){
  return 1;
}
