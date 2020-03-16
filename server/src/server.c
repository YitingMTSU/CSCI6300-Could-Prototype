#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "server.h" //the server header file


int main() {

  int sockfd, newSocket;
  struct sockaddr_in serverAddr;
  int opt = 1;
  
  //int addrlen
  struct sockaddr_in newAddr;

  socklen_t addr_size;
  char buffer[BUFFER_LEN];
  memset(&buffer, 0, sizeof(buffer));
  
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  memset(&serverAddr, 0, sizeof(serverAddr));


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

  int pid;
  //start accept the connection
  while (1) {
    if ((newSocket = accept(sockfd, (struct sockaddr*)& newAddr,
			    &addr_size)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    printf("Connection accepted from %s Port:%d\n",
	   inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

    pid = fork();
    if (pid == 0) {
      close(sockfd);
      char userName[USERNAME_LEN]; 
      int logInS = login(newSocket,buffer,userName);
      memset(buffer, 0, strlen(buffer));
      
      while (logInS) {
	int quit = mainUsageServer(newSocket, buffer,userName);
	if (quit == 1) break;
      }//end while
    } else { //parent process
      close(newSocket);
    }//end if
  }//end while
    
  return 0;
}

int checkUser(char* userName, char* password) {
  FILE* fp;
  char userNameInFile[USERNAME_LEN];
  char passwordInFile[PASSWORD_LEN];
  char* filename = ACCOUNT_PATH;
  fp = fopen(filename,"r");

  if (fp == NULL) {
    printf("Could not open file %s\n",filename);
    return 0;
  }

  while (!feof(fp)) {
    fscanf(fp, "%s %s[\n]", userNameInFile,passwordInFile);
    //printf("UserName:%s ",userNameInFile);
    //printf("Password:%s\n",passwordInFile);
    //printf("UserName: %s UserNameInFile: %s\n",userName,userNameInFile);
    //printf("len: [%ld %ld]",strlen(userName),strlen(userNameInFile));
    if (strcmp(userName,userNameInFile) == 0) {
      strcpy(password,passwordInFile);
      fclose(fp);
      return 1;
    }
    memset(userNameInFile,0,USERNAME_LEN);
    memset(passwordInFile,0,PASSWORD_LEN);
  }
  //printf("out of the loop\n");
  fclose(fp);
  return 0;
}


int login(int socket, char* buffer, char* userName) {
  //sleep(1);
  //int messageLen = strlen(interfaceWelcome);
  //send(socket, &messageLen, sizeof(messageLen),0);
  send(socket,interfaceWelcome,strlen(interfaceWelcome),0);

  //sleep(10);
  //messageLen = strlen(interfaceUser);
  //send(socket, &messageLen, sizeof(messageLen),0);
  send(socket,interfaceUser,strlen(interfaceUser),0);
  printf("waiting for user name...\n");
  //get user name
  sleep(1);
  recv(socket,buffer,BUFFER_LEN,0);
  //printf("UserName:%s\n",buffer);
  char passwordInFile[PASSWORD_LEN];
  //char userName[USERNAME_LEN];
  strcpy(userName, buffer);
  int userCheck = checkUser(buffer, passwordInFile);
  memset(buffer, 0, strlen(buffer));
  //printf("userCheck: %d\n",userCheck);
  if (userCheck == 0) {
    char userExist = '0';
    send(socket,&userExist,sizeof(userExist),0);
    //sleep(0.5);
    send(socket,interfaceNewUser,strlen(interfaceNewUser),0);
    char option;
    recv(socket, &option, sizeof(option),0);
    if (option == '2') { //quit the connection
      send(socket, interfaceBye, strlen(interfaceBye), 0);
      exit(1);
    } else { //create the account and enter password
      
      send(socket, interfacePassword, strlen(interfacePassword), 0);
      char passwordFirst[PASSWORD_LEN];
      //receive the first password
      recv(socket, buffer, BUFFER_LEN, 0);
      //printf("after\n");
      strcpy(passwordFirst,buffer);
      //printf("The first password:%s\n",passwordFirst);
      memset(buffer, 0, strlen(buffer));
      send(socket, interfaceReEnterPassword, strlen(interfaceReEnterPassword), 0);
      char passwordSecond[PASSWORD_LEN];
      recv(socket, buffer, BUFFER_LEN, 0);
      strcpy(passwordSecond, buffer);
      //printf("The second password:%s\n",passwordSecond);
      memset(buffer, 0, strlen(buffer));
      if (strcmp(passwordFirst, passwordSecond) == 0) {
	char create = '1';
	send(socket, &create, sizeof(create), 0);
	writeNewUserToFile(userName,passwordFirst);
	return 1;
      } else {
	char create = '0';
	send(socket, &create, sizeof(create), 0);
	exit(1);
      }
            
    }      
    //memset(&buffer, '\0', sizeof(buffer));
  } else {
    char userExist = '1';
    send(socket,&userExist,sizeof(userExist),0);
    const int tryTimes = 3;
    int count = 1;
  COMPAREPASSWORD:
    send(socket, interfacePassword, strlen(interfacePassword), 0);
    //memset(&buffer, '\0', sizeof(buffer));
    
    recv(socket, buffer, BUFFER_LEN, 0);//receive password

    //printf("the password: %s\n %s\n", buffer, passwordInFile);
    if (strcmp(buffer, passwordInFile) == 0) {
      char pass = '1';
      send(socket, &pass, sizeof(pass), 0);
      return 1;
    } else {
      char pass = '0';
      send(socket, &pass, sizeof(pass), 0);
      if (count == tryTimes) {
	send(socket, interfacePasswordError, strlen(interfacePasswordError), 0);
	close(socket);
	return 0;
      } else {
	send(socket, interfacePasswordError, strlen(interfacePasswordError), 0);
	sleep(1);
	count++;
	goto COMPAREPASSWORD;
      }
    }
    
  }
  return 0;
}


void writeNewUserToFile(char* userName, char* password) {
  FILE *fp;
  char *path = ACCOUNT_PATH;
  fp = fopen(path,"a");

  fprintf(fp, "%s %s\n",userName,password);
  
  fclose(fp);
  //printf("append the info file!\n");
  
  char userDataPath[300];
  memset(userDataPath, 0, 300);
  strcat(userDataPath, DATA_PATH);
  strcat(userDataPath, userName);
  strcat(userDataPath, "Data.txt");

  //printf("userDataPath: %s\n",userDataPath);
  fp = fopen(userDataPath, "w");
  char content[50];
  memset(content, 0, 50);
  strcat(content, "Hello, this is ");
  strcat(content, userName);
  strcat(content, "'s file\n");
  //printf("content: %s\n",content);
  fputs(content,fp);
  fclose(fp);

  
}

int mainUsageServer(int socket, char* buffer, char* userName){
  send(socket, interfaceUsage, strlen(interfaceUsage), 0);
  printf("usage sent\n");
  sleep(1);
  send(socket, interfaceOption, strlen(interfaceOption), 0);
  printf("option sent\n");
  char option;
  recv(socket,&option,sizeof(option),0);
  printf("option : %c\n",option);

  char filename[FILE_LEN];
  int find;
  switch (option) {
  case '1': //read the file
    //send the list of files
    showFiles(socket,buffer);
    
    //get the file name the user want to read
    recv(socket,buffer,BUFFER_LEN,0);
    strcpy(filename,buffer);
    bzero(buffer,strlen(buffer));
      
    //send the information of the file
    printf("The file %s\n",filename);
    sendFileInfor(socket,filename);
    bzero(filename,strlen(filename));
    
    //return back to main interface
    printf("come back!\n");
    mainUsageServer(socket,buffer,userName);
    break;
  case '2': //wirte information
    //send the list of files
    showFiles(socket,buffer);

    //get the file name the user want to read
    recv(socket,buffer,BUFFER_LEN,0);
    printf("The file %s\n",buffer);
    strcpy(filename,buffer);
    bzero(buffer,strlen(buffer));
    
    //send the information of the file 
    find = sendFileInfor(socket,filename);

    //if find recv and create new tmp file,
    //otherwise return back to main usage interface
    if (find == 1) {
      createWriteFile(socket,filename);
      combineWriteFile(filename);
    }
    bzero(filename,strlen(filename));
    
    //return back to main interface
    printf("come back!\n");
    mainUsageServer(socket,buffer,userName);
    
    break;
  case '3': //delete information
    //send the list of files
    showFiles(socket,buffer);

    //get the file name the user want to read
    recv(socket,buffer,BUFFER_LEN,0);
    printf("The file %s\n",buffer);
    strcpy(filename,buffer);
    bzero(buffer,strlen(buffer));

    //send the information of the file
    find = sendFileInfor(socket,filename);

    //if find recv and create new tmp file,
    //otherwise return back to main usage interface
    if (find == 1) {
      createDeleteFile(socket,filename);
      combineDeleteFile(filename);
    }
    bzero(filename,strlen(filename));

    //return back to main interface
    printf("come back!\n");
    mainUsageServer(socket,buffer,userName);
    
    break;
  case '4': //quit the system
    send(socket,interfaceBye,strlen(interfaceBye),0);
    return 1;
    break;
  default:
    return 1;
  }
  return 1;
}


void readFile(char* filename, char* buffer){

  FILE* fp;
  fp = fopen(filename,"r");
  
  int strLen = 50;
  char str[strLen];
  while (fgets(str,strLen,fp) != NULL) {
    strcat(buffer,str);
  }
  fclose(fp);
  //printf("%s\n",buffer);
  
}

void showFiles(int socket, char* buffer){
  struct dirent *de;
  DIR *dr = opendir(DATA_PATH);

  if (dr == NULL) {
    printf("Could not open curent directory\n");
    exit(1);
  }
  
  int count = 1;
  // for readdir()
  while ((de = readdir(dr)) != NULL) {
    if(strcmp(de->d_name,"..")==0) break;
    char str[FILE_LEN];
    //memset(str,0,FILE_LEN);
    sprintf(str,"%d. %s\n", count,de->d_name);
    strcat(buffer,str);
    count++;
  }

  closedir(dr);
  //printf("files:%s\n",buffer);
  send(socket,buffer,BUFFER_LEN,0);
  bzero(buffer,BUFFER_LEN);
}


int sendFileInfor(int socket, char* filename) {
  struct dirent *de;
  DIR *dr = opendir(DATA_PATH);

  if (dr == NULL) {
    printf("Could not open curent directory\n");
    exit(1);
  }

  int find = 0;
  // for readdir()
  while ((de = readdir(dr)) != NULL) {
    if(strcmp(de->d_name,"..") == 0) break;
    if (strcmp(de->d_name,filename) == 0) {
      find = 1;
      break;
    }
  }

  closedir(dr);
  printf("find the file or not: %d\n",find);
  send(socket,&find,sizeof(find),0);
  if (find == 0) {//didn't find the file
    printf("didn't find the file\n");
    bzero(filename,strlen(filename));
    return -1;
  } else {//find the file, send the information
    printf("find the file\n");
    char filenameToRead[BUFFER_LEN];
    memset(filenameToRead,0,strlen(filenameToRead));
    //printf("file path:%s\n",filenameToRead);
    strcat(filenameToRead, DATA_PATH);
    //printf("file path:%s\n",filenameToRead);
    strcat(filenameToRead, filename);
    char buffer[BUFFER_LEN];
    memset(buffer,0,BUFFER_LEN);
    //printf("file path:%s\n",filenameToRead);
    readFile(filenameToRead,buffer);
    printf("send content: %s\n",buffer);
    send(socket,buffer,strlen(buffer),0);
    sleep(1);
  }
  return 1;
}


void createWriteFile(int socket, char* filename) {
  //read the new wirte information
  char buffer[BUFFER_LEN];
  bzero(buffer,BUFFER_LEN);
  recv(socket, buffer,BUFFER_LEN,0);

  //create new tmp write file
  FILE *fp;
  char writeFilePath[FILE_LEN];
  memset(writeFilePath,0,FILE_LEN);

  //get the path of write file
  strcat(writeFilePath,DATA_PATH);
  strcat(writeFilePath,WRITE);
  strcat(writeFilePath,filename);
  printf("the path of new write file: %s\n",writeFilePath);

  //put the buffer to new write file
  fp = fopen(writeFilePath,"w");
  fputs(buffer,fp);
  fclose(fp);
  bzero(buffer,strlen(buffer));
  bzero(writeFilePath,strlen(writeFilePath));
  
  return;
}


void createDeleteFile(int socket, char* filename) {
  //read the new wirte information
  char buffer[BUFFER_LEN];
  bzero(buffer,BUFFER_LEN);
  recv(socket, buffer,BUFFER_LEN,0);

  //create new tmp delete file
  FILE *fp;
  char deleteFilePath[FILE_LEN];
  memset(deleteFilePath,0,FILE_LEN);

  //get the path of delete file
  strcat(deleteFilePath,DATA_PATH);
  strcat(deleteFilePath,DELETE);
  strcat(deleteFilePath,filename);
  printf("the path of new write file: %s\n",deleteFilePath);

  //put the buffer to new delete file
  fp = fopen(deleteFilePath,"w");
  fputs(buffer,fp);
  fclose(fp);
  bzero(buffer,strlen(buffer));
  bzero(deleteFilePath,strlen(deleteFilePath));
  
  return;
}
