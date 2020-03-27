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
  char serverOption;
  char input[5];
  fgets(input,5,stdin);
  serverOption = input[0];
  int serverIP;
  if (serverOption == '1') {
    serverIP = 1;
  } else if (serverOption == '2') {
    serverIP = 2;
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
  if (serverIP == 1) {
    if (inet_pton(AF_INET, SERVER_HERSCHEL_IP, &serverAddr.sin_addr) <= 0){
      printf("\nInvalid address/ Address not supported \n");
      return -1;
    }
    printf("[+]Convert the HERSCHEL IP...\n");
  } else {
    if (inet_pton(AF_INET, SERVER_RANGER_IP, &serverAddr.sin_addr) <= 0){
      printf("\nInvalid address/ Address not supported \n");
      return -1;
    }
    printf("[+]Convert the RANGER IP...\n");    
  } 

  
  if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("\nConnection Failed \n");
  }
  printf("[+]Connect to the sever...\n");


  //send the serverIP to server
  send(clientSocket,&serverIP,sizeof(serverIP),0);
  
  //check if the server is lock or not
  int lock;
  recv(clientSocket,&lock,sizeof(lock),0);
  if (lock == 0) {
    //login into the server
    int logInS = login(clientSocket,buffer);
    memset(buffer, 0, strlen(buffer));
    
    while (logInS) {
      int quit = mainUsageClient(clientSocket, buffer);
      if (quit == 1) break;
    }
  } else {
    printf("The server is locked right now, please try later!\n");
  }

  close(clientSocket);
    
  return 0;
}


int login(int socket, char* buffer) {
  //read the interface from server
  //int sig;
  //int messageLen;
  //recv(socket, &messageLen, sizeof(messageLen), 0);
  //printf("the messagelen : %d\n",messageLen);
  //sig = recv(socket, buffer, messageLen, 0);
  //recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s\n",interfaceWelcome);
  //bzero(buffer, BUFFER_LEN);
  //sleep(0.5);
  
  //recv(socket, &messageLen, sizeof(messageLen), 0);
  //recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s",interfaceUser);
  //memset(buffer, 0, BUFFER_LEN);
  
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
  //printf("userExist: %c\n",userExist);
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
  recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s\n",buffer);
  bzero(buffer,BUFFER_LEN);
  recv(socket, buffer, BUFFER_LEN, 0);
  printf("%s",buffer);
  char input[5];
  char option;
 RECHOOSE:
  fgets(input,5,stdin);
  option = input[0];
  if (option - '1' < 0 || option - '1' > 3) {
    printf("Error input!\n");
    printf("%s",buffer);
    goto RECHOOSE;
  }
  bzero(buffer,BUFFER_LEN);
  send(socket,&option,sizeof(option),0);

  char filename[FILE_LEN];
  int find;
  int ACK = 1;
  int fileLock;
  int permission;
  switch (option) {
  case '1': //read file
    //receive the list of files
    printf("%s\n",interfaceFiles);
    recv(socket,buffer,BUFFER_LEN,0);
    printf("%s",buffer);
    bzero(buffer,BUFFER_LEN);
    
    //get the file want to read
    printf("%s to read: ",interfaceChooseFile);
    fgets(filename,FILE_LEN,stdin);
    filename[strlen(filename)-1] = '\0';

    //send the file name
    printf("filename:%s\n",filename);
    send(socket,filename,strlen(filename),0);
    
    //recv the information from the file
    recv(socket,&find,sizeof(find),0);
    if (find == 1) { //find the file, print the information
      //check the file lock
      recv(socket,&fileLock,sizeof(fileLock),0);
      send(socket,&(ACK),sizeof(ACK),0);
      if (fileLock == 0) {
	recv(socket,buffer,BUFFER_LEN,0);
	printf("%s:\n%s\n\n",filename,buffer);
	bzero(buffer,BUFFER_LEN);
	sleep(1);
      } else {
	printf("%s\n\n",interfaceFileLock);
      }
      mainUsageClient(socket,buffer);
    } else { //didn't find the file, return back to interface
      printf("The file didn't exist!\n");
      
      mainUsageClient(socket,buffer);
    }
    break;
  case '2': //write information

    //receive the list of files
    printf("%s\n",interfaceFiles);
    recv(socket,buffer,BUFFER_LEN,0);
    printf("%s",buffer);
    bzero(buffer,BUFFER_LEN);
    
    //get the file want to write
    printf("%s to write: ",interfaceChooseFile);
    fgets(filename,FILE_LEN,stdin);
    filename[strlen(filename)-1] = '\0';
    
    //send the file name the user want to write
    send(socket,filename,strlen(filename),0);

    //receive the permission
    recv(socket,&permission,sizeof(permission),0);

    if (permission == 1) {
      //recv the information from the file
      recv(socket,&find,sizeof(find),0);
      if (find == 1) { //find the file, print the information
	//check the file lock
	recv(socket,&fileLock,sizeof(fileLock),0);
	send(socket,&(ACK),sizeof(ACK),0);
	if (fileLock == 0) {
	  //read the information of file
	  recv(socket,buffer,BUFFER_LEN,0);
	  printf("%s:\n%s\n\n",filename,buffer);
	  bzero(buffer,BUFFER_LEN);
	
	  //get the new information you want to wirte
	  printf("Enter the information you want to write:\n");
	  fgets(buffer,BUFFER_LEN,stdin);
	
	  //send information
	  send(socket,buffer,strlen(buffer),0);
	  printf("[+]New information to write sent.\n");
	  bzero(buffer,BUFFER_LEN);
	  
	  sleep(1);
	} else {
	  printf("%s\n",interfaceFileLock);
	}
      } else { //didn't find the file, return back to interface
	printf("The file didn't exist!\n");
      }
      
      mainUsageClient(socket,buffer);
    } else {
      printf("You You didn't have the permission to write.\n");
      mainUsageClient(socket,buffer);
    }
    break;
  case '3': //delete information
    //receive the list of files
    printf("%s\n",interfaceFiles);
    recv(socket,buffer,BUFFER_LEN,0);
    printf("%s",buffer);
    bzero(buffer,BUFFER_LEN);

    //get the file want to delete information
    printf("%s to delete: ",interfaceChooseFile);
    fgets(filename,FILE_LEN,stdin);
    filename[strlen(filename)-1] = '\0';

    //send the file name the user want to delete information
    send(socket,filename,strlen(filename),0);


    //receive the permission
    recv(socket,&permission,sizeof(permission),0);

    if (permission == 1) {
      //recv the information from the file
      recv(socket,&find,sizeof(find),0);
      if (find == 1) { //find the file, print the information
	//check the file lock
	recv(socket,&fileLock,sizeof(fileLock),0);
	send(socket,&(ACK),sizeof(ACK),0);
	if (fileLock == 0) {
	  //read the information of file
	  recv(socket,buffer,BUFFER_LEN,0);
	  printf("%s:\n%s\n\n",filename,buffer);
	  bzero(buffer,BUFFER_LEN);
	  
	  //get the new information you want to delete
	  printf("Enter the information you want to delete:\n");
	  fgets(buffer,BUFFER_LEN,stdin);
	  
	  //send information
	  send(socket,buffer,strlen(buffer),0);
	  printf("[+]Information to delete sent.\n");
	  bzero(buffer,strlen(buffer));
	  
	  sleep(1);
	} else {
	  printf("%s\n",interfaceFileLock);
	}
      } else { //didn't find the file, return back to interface
	printf("The file didn't exist!\n");
      }
      mainUsageClient(socket,buffer);
    } else {
      printf("You You didn't have the permission to delete.\n");
      mainUsageClient(socket,buffer);
    }
    break;
  case '4': //quit the system
    recv(socket,buffer,BUFFER_LEN,0);
    printf("%s\n",buffer);
    bzero(buffer,BUFFER_LEN);
    exit(1);
    break;
  default:
    return 1;
  }
  
  return 1;
}
