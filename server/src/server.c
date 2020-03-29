#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "server.h" //the server header file


//global variables
char anotherIP[IP_LEN];
int serverLock = 0;
char ACCOUNT_PATH[PATH_MAX];
char DATA_PATH[PATH_MAX];
struct FILELOCK fileLock[MAX_USER];
int curUser = 0;
int ACK = 0;

int main() {

  //get tmp path and set ACCOUNT_PATH DATA_PATH
  getPath();

  //initial the fileLock
  setFileLock();
  
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

      //receive the server IP and set another server IP
      int serverIP;
      recv(newSocket,&serverIP,sizeof(serverIP),0);
      
      if (serverIP == 1) {
	strcpy(anotherIP,SERVER_RANGER_IP);
      } else {
	strcpy(anotherIP,SERVER_HERSCHEL_IP);
      }
      printf("another IP: %s\n",anotherIP);
      
      send(newSocket,&serverLock,sizeof(serverLock),0);
      
      if (serverLock == 0){
	char userName[USERNAME_LEN]; 
	int logInS = login(newSocket,buffer,userName);
	memset(buffer, 0, strlen(buffer));

	//print the username to check
	printf("usernama: %s\n",userName);
	if (strcmp(userName,"root") == 0) {
	  serverLock = 1;
	  rootSyn(newSocket,buffer);
	  serverLock = 0;
	} else { //common users
	  //check if server synchronizing
   
	  int curLockInd = getCurLockInd(userName);
	  
	  while (logInS) {
	    int quit = mainUsageServer(newSocket, buffer,userName, curLockInd);
	    if (quit == 1) break;
	  }//end while
	}
	close(newSocket);
      } else {
	close(newSocket);
      }
      
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
  return -1;
}


int login(int socket, char* buffer, char* userName) {
  
  printf("waiting for user name...\n");

  //get user name
  recv(socket, buffer, BUFFER_LEN, 0);
  //printf("UserName:%s\n",buffer);
  char passwordInFile[PASSWORD_LEN];
  //char userName[USERNAME_LEN];
  strcpy(userName, buffer);
  int userCheck = checkUser(buffer, passwordInFile);
  memset(buffer, 0, strlen(buffer));
  send(socket, &userCheck, sizeof(userCheck), 0);
  
  if (userCheck == -1) {//didn't find the user
    char option;
    recv(socket, &option, sizeof(option), 0);
    send(socket, &ACK, sizeof(ACK), 0);
    
    if (option == '1') { //create the account and enter password
      //receive the first password
      char passwordFirst[PASSWORD_LEN];
      recv(socket, buffer, BUFFER_LEN, 0);
      send(socket, &ACK, sizeof(ACK), 0);
      strcpy(passwordFirst,buffer);
      memset(buffer, 0, strlen(buffer));
      
      //reveive the second password
      char passwordSecond[PASSWORD_LEN];
      recv(socket, buffer, BUFFER_LEN, 0);
      //send(socket, &ACK, sizeof(ACK), 0);
      strcpy(passwordSecond, buffer);
      memset(buffer, 0, strlen(buffer));

      //compare the password
      char create;
      if (strcmp(passwordFirst, passwordSecond) == 0) {
	create = '1';
      } else {
	create = '0';
      }

      //send if create the new user
      send(socket, &create, sizeof(create), 0);

      if (create == '1') {
	//write new user information to file
	writeNewUserToFile(userName,passwordFirst);

	//add the new user to filelock
	strcpy(fileLock[curUser].filename, userName);
	strcat(fileLock[curUser].filename,"Data.txt");
	fileLock[curUser].lock = 0;
	fileLock[curUser].write = 0;
	fileLock[curUser].delete = 0;
	curUser++;
	
	//send the information to another server
	sendUserToAnotherServer(anotherIP,userName,passwordFirst);
	
	return 1;
      } else {//the password didn't match
	return 0;
      }           
    } else { //user choose quit
      return 0;
    }
  } else { //find the user
    
    const int tryTimes = 3;
    int count = 1;
  COMPAREPASSWORD:

    //receive password
    recv(socket, buffer, BUFFER_LEN, 0);

    //compare the password
    if (strcmp(buffer, passwordInFile) == 0) {
      char pass = '1';
      send(socket, &pass, sizeof(pass), 0);
      return 1;
    } else {
      char pass = '0';
      send(socket, &pass, sizeof(pass), 0);
      if (count == tryTimes) {
	close(socket);
	return 0;
      } else {
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
  return;  
}

int mainUsageServer(int socket, char* buffer, char* userName, int lockInd){
  //receive the option user choose
  char option;
  recv(socket, &option, sizeof(option), 0);

  char filename[FILE_LEN];
  bzero(filename, FILE_LEN);
  char fileInfo[BUFFER_LEN];
  bzero(fileInfo, BUFFER_LEN);
  int find;
  int readInd;
  int permission;
  
  switch (option) {
  case '1': //read the file
    //send the list of files
    showFiles(socket,buffer);
    
    //get the file name the user want to read
    recv(socket,buffer,BUFFER_LEN,0);
    strcpy(filename,buffer);
    bzero(buffer,strlen(buffer));

    //check if find the file, if find send information
    find = findFile(socket, filename, fileInfo);
    send(socket, &find, sizeof(find), 0);

    //if find the file, send the information
    if (find == 1) {
      recv(socket, &ACK, sizeof(ACK), 0);

      //check the file lock
      readInd = getCurLockIndByFileName(filename);
      send(socket, &fileLock[readInd].lock, sizeof(int), 0);

      //send the file information if file not lock
      if (fileLock[readInd].lock == 0) {
	recv(socket, &ACK, sizeof(ACK), 0);
	send(socket, fileInfo, strlen(fileInfo), 0);
      }
    }

    //reset filename and fileInfo
    bzero(filename,strlen(filename));
    bzero(fileInfo,strlen(fileInfo));
    
    //return back to main interface
    printf("come back!\n");
    mainUsageServer(socket,buffer,userName,lockInd);
    break;
  case '2': //wirte information
    //send the list of files
    showFiles(socket,buffer);

    //get the file name the user want to read
    recv(socket,buffer,BUFFER_LEN,0);
    strcpy(filename,buffer);
    bzero(buffer,strlen(buffer));

    //check the permission
    permission = checkPermission(filename, userName);
    send(socket, &permission, sizeof(permission), 0);

    if (permission == 1) {
      recv(socket, &(ACK), sizeof(ACK), 0);

      //check if find the file, if find send information
      find = findFile(socket, filename, fileInfo);
      send(socket, &find, sizeof(find), 0);
      
      //if find recv and create new tmp file,
      //otherwise return back to main usage interface
      if (find == 1) {
	recv(socket, &(ACK), sizeof(ACK), 0);
	//check the file lock
	send(socket, &fileLock[lockInd].lock, sizeof(int), 0);

	if (fileLock[lockInd].lock == 0) {
	  recv(socket, &(ACK), sizeof(ACK), 0);

	  //send the file information
	  send(socket, fileInfo, strlen(fileInfo), 0);
	  
	  //lock the file
	  fileLock[lockInd].lock = 1;
	  fileLock[lockInd].write = 1;

	  //receive the new information
	  recv(socket, buffer, BUFFER_LEN, 0);
	  
	  //create write file
	  createWriteFile(socket,filename,buffer);
	  bzero(buffer,strlen(buffer));
	  send(socket, &ACK, sizeof(ACK), 0);
	}
      }
      bzero(filename,strlen(filename));
    }
    
    //return back to main interface
    printf("come back!\n");
    mainUsageServer(socket,buffer,userName,lockInd);
    
    break;
  case '3': //delete information
    //send the list of files
    showFiles(socket,buffer);

    //get the file name the user want to read
    recv(socket,buffer,BUFFER_LEN,0);
    printf("The file %s\n",buffer);
    strcpy(filename,buffer);
    bzero(buffer,strlen(buffer));

    //check the permission
    permission = checkPermission(filename, userName);
    send(socket, &permission, sizeof(permission), 0);

    if (permission == 1) {
      recv(socket, &ACK, sizeof(ACK), 0);
      
      //check if find the file, if find, send the information
      find = findFile(socket,filename,fileInfo);
      send(socket, &find, sizeof(find), 0);
      
      //if find recv and create new tmp file,
      //otherwise return back to main usage interface
      if (find == 1) {
	recv(socket, &ACK, sizeof(ACK), 0);
	
	//check the file lock
	send(socket, &fileLock[lockInd].lock, sizeof(int), 0);

	if (fileLock[lockInd].lock == 0) {
	  recv(socket, &ACK, sizeof(ACK), 0);

	  //send the file information
	  send(socket, fileInfo, strlen(fileInfo), 0);
	  
	  //lock the file
	  fileLock[lockInd].lock = 1;
	  fileLock[lockInd].delete = 1;

	  //receive the new (delete) information
	  recv(socket, buffer, BUFFER_LEN, 0);
	  
	  //create delete file
	  createDeleteFile(socket, filename, buffer);
	  bzero(buffer, strlen(buffer));
	  send(socket, &ACK, sizeof(ACK), 0);
	}
      }
      bzero(filename,strlen(filename));
    }
    //return back to main interface
    printf("come back!\n");
    mainUsageServer(socket,buffer,userName,lockInd);
    
    break;
  case '4': //quit the system
    //check if write or delete
    checkWD(socket, lockInd, userName);
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
    if(strcmp(de->d_name,"..")==0 || strcmp(de->d_name,".")==0) continue;
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


int findFile(int socket, char* filename, char* fileInfo) {
  struct dirent *de;
  DIR *dr = opendir(DATA_PATH);

  if (dr == NULL) {
    printf("Could not open curent directory\n");
    exit(1);
  }

  int find = -1;
 
  while ((de = readdir(dr)) != NULL) {
    if(strcmp(de->d_name,"..") == 0 || strcmp(de->d_name,".")==0) continue;
    if (strcmp(de->d_name,filename) == 0) {
      find = 1;
      break;
    }
  }

  closedir(dr);

  //send if the file find or not
  printf("find the file or not: %d\n",find);
  
  if (find == 1) {
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
    strcpy(fileInfo, buffer);
  }
  
  return find;
}


void createWriteFile(int socket, char* filename, char* buffer) {
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
  bzero(writeFilePath,strlen(writeFilePath));
  
  return;
}


void createDeleteFile(int socket, char* filename, char* buffer) {
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

void combineWriteFile(char* filename) {
  FILE *fin,*fout;
  char writeFilePath[FILE_LEN];
  memset(writeFilePath,0,FILE_LEN);

  //get the path of write file
  strcat(writeFilePath,DATA_PATH);
  strcat(writeFilePath,WRITE);
  strcat(writeFilePath,filename);

  char dataFile[FILE_LEN];
  memset(dataFile,0,FILE_LEN);

  //get the path of data file
  strcat(dataFile,DATA_PATH);
  strcat(dataFile,filename);

  fin = fopen(writeFilePath,"r");
  fout = fopen(dataFile,"a");

  char line[BUFFER_LEN];
  bzero(line,BUFFER_LEN);
  while (fgets(line,sizeof(line),fin)) {
    printf("%s",line);
    fprintf(fout,"%s",line);
    bzero(line,strlen(line));
  }

  fclose(fin);
  fclose(fout);

  //delete the file
  remove(writeFilePath); 
}

void combineDeleteFile(char* filename) {
  FILE *fin,*fout,*fnew;
  char fileDelPath[FILE_LEN];
  bzero(fileDelPath,FILE_LEN);

  //get the path of delete file
  strcat(fileDelPath,DATA_PATH);
  strcat(fileDelPath,DELETE);
  strcat(fileDelPath,filename);

  fin = fopen(fileDelPath,"r");
  char lineDel[BUFFER_LEN];
  bzero(lineDel,BUFFER_LEN);
  while (fgets(lineDel,sizeof(lineDel),fin)) {
    printf("%s",lineDel);
  }

  //get the path of original data
  char fileOriPath[FILE_LEN];
  bzero(fileOriPath,FILE_LEN);
  strcat(fileOriPath,DATA_PATH);
  strcat(fileOriPath,filename);

  fout = fopen(fileOriPath,"r");
  
  //get the path of tmp data file
  char fileTmpPath[FILE_LEN];
  bzero(fileTmpPath,FILE_LEN);
  strcat(fileTmpPath,DATA_PATH);
  strcat(fileTmpPath,"Tmp");
  strcat(fileTmpPath,filename);

  fnew = fopen(fileTmpPath,"a");

  char lineKeep[BUFFER_LEN];
  bzero(lineKeep,BUFFER_LEN);
  while (fgets(lineKeep,sizeof(lineKeep),fout)) {
    if (strcmp(lineDel,lineKeep) == 0) {
      continue;
    }
    fprintf(fnew,"%s",lineKeep);
  }

  fclose(fin);
  fclose(fout);
  fclose(fnew);

  remove(fileDelPath);
  remove(fileOriPath);
  rename(fileTmpPath,fileOriPath);
}
				    
int sendUserToAnotherServer(char* IP, char* username, char* password) {
  //it works as client side
  int clientSocket;
  struct sockaddr_in serverAddr;
  char buffer[BUFFER_LEN];
  memset(buffer, 0, sizeof(buffer));

  if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }
  printf("[+]Create socket...\n");

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);

  printf("IP: %s\n",IP);
  if (inet_pton(AF_INET, IP, &serverAddr.sin_addr) <= 0){
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }
  printf("[+]Convert the SERVER IP...\n");

  if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("\nConnection Failed \n");
  }
  printf("[+]Connect to the sever...\n");

  
  int serverIP; 
  if (IP == SERVER_HERSCHEL_IP) {
    serverIP = 2;
  } else {
    serverIP = 1;
  }
  
  send(clientSocket,&serverIP,sizeof(serverIP),0);
  
  //check if the server is lock or not
  int lock;
  recv(clientSocket,&lock,sizeof(lock),0);

  if (lock == 0) {
    //login into the server as root
    send(clientSocket,ROOT,strlen(ROOT),0);

    //check the account if exist
    int userExist;
    recv(clientSocket,&userExist,sizeof(userExist),0);

    //send root's password
    send(clientSocket,ROOT_PASSWORD,strlen(ROOT_PASSWORD),0);

    //check the password
    char pass;
    recv(clientSocket, &pass, sizeof(pass), 0);

    //OPTION 1: ADD USER
    //OPTION 2: WRITE
    //OPTION 3: DELETE

    int option = 1;
    send(clientSocket, &option, sizeof(option), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

    //send new user name
    send(clientSocket, username, strlen(username), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

    //send new password
    send(clientSocket, password, strlen(password), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

    close(clientSocket);
    return 1;
    
  } else {
    printf("The server is locked right now, please try later!\n");
  }
  close(clientSocket);
  return 1;
  
}


void rootSyn(int socket, char* buffer) {

  //When another server login as the root
  //Here are three option
  //OPTION 1: add the new user and its passwrod
  //OPTION 2: WRITE
  //OPTION 3: DELETE
  int option;
  recv(socket, &option, sizeof(option), 0);
  send(socket, &ACK, sizeof(ACK), 0);

  char userName[USERNAME_LEN];
  char password[PASSWORD_LEN];
  char fileName[FILE_LEN];
  //initialize
  bzero(userName,USERNAME_LEN);
  bzero(password,PASSWORD_LEN);
  bzero(fileName,FILE_LEN);

  switch (option) {
  case 1: // add the new user
    //get new user name
    recv(socket, userName, USERNAME_LEN, 0);
    send(socket, &ACK, sizeof(ACK), 0);

    //get new password
    recv(socket, password, PASSWORD_LEN, 0);
    send(socket, &ACK, sizeof(ACK), 0);

    writeNewUserToFile(userName,password);
    break;
  case 2: // synchronize the write file
    //receive the file name
    recv(socket, fileName, FILE_LEN, 0);
    send(socket, &ACK, sizeof(ACK), 0);

    //receive the information to write
    recv(socket, buffer, BUFFER_LEN, 0);
    send(socket, &ACK, sizeof(ACK), 0);
    createWriteFile(socket,fileName,buffer);
    bzero(buffer,strlen(buffer));

    //combine the write file
    combineWriteFile(fileName);
    break;
  case 3: // synchronize the delete file
    //receive the file name
    recv(socket, fileName, FILE_LEN, 0);
    send(socket, &ACK, sizeof(ACK), 0);

    //receive the information to delete
    recv(socket, buffer, BUFFER_LEN, 0);
    send(socket, &ACK, sizeof(ACK), 0);
    createDeleteFile(socket,fileName,buffer);
    bzero(buffer,strlen(buffer));

    //combine the delete file
    combineDeleteFile(fileName);
    break;
  default:
    return;
  }
  return;
}

void getPath() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    //printf("Current working dir: %s\n", cwd);
   } else {
       perror("getcwd() error");
       return;
   }

   //delete last three elements(src) in path
   int count = 0;
   while(cwd[count] != '\0'){
     count++;
   }

   for(int i=0;i<SRC_LEN;i++){
     cwd[count--] = '\0';
   }

   strcpy(ACCOUNT_PATH,cwd);
   strcat(ACCOUNT_PATH,"data/userAccount/userInfo.txt");
   strcpy(DATA_PATH,cwd);
   strcat(DATA_PATH,"data/userData/");

   return; 
}

int checkPermission(char* filename, char* username) {

  int len = strlen(username);
  int i;
  printf("filename: %s\n",filename);
  printf("username: %s\n",username);
  for (i=0;i<len;i++) {
    if (filename[i] != username[i]) break;
  }
  printf("i = %d, len = %d\n",i,len);
  
  if (i == len) return 1;
  return -1;
}

void setFileLock() {
  struct dirent *de;
  DIR *dr = opendir(DATA_PATH);

  if (dr == NULL) {
    printf("Could not open curent directory\n");
    exit(1);
  }

  // for readdir()
  while ((de = readdir(dr)) != NULL) {
    if(strcmp(de->d_name,"..") == 0 || strcmp(de->d_name,".")==0) continue;
    
    strcpy(fileLock[curUser].filename,de->d_name);
    fileLock[curUser].lock = 0;
    fileLock[curUser].write = 0;
    fileLock[curUser].delete = 0;
    curUser++;
  }

  closedir(dr);

  return;
}

int getCurLockInd(char* username) {
  char curFileName[FILE_LEN];
  strcpy(curFileName,username);
  strcat(curFileName,"Data.txt");
  int ind = getCurLockIndByFileName(curFileName);
  return ind;
}

int getCurLockIndByFileName(char* filename) {
  for (int i=0;i<curUser;i++) {
    if (strcmp(fileLock[i].filename,filename) == 0) {
      return i;
    }
  }
  printf("error user!\n");
  exit(1);  
}

void checkWD(int socket, int lockInd, char* username) {
  if (fileLock[lockInd].write == 1) {
    char filename[FILE_LEN];
    strcpy(filename,username);
    strcat(filename,"Data.txt");

    //int pid = fork();
    //if (pid != 0) {
    //  close(socket);
    printf("filename in checkWD:%s\n",filename);
    sendWriteFile(anotherIP,filename);
    //  exit(1);
    //}
    
    //combine wirte file
    combineWriteFile(filename);
  }

  if (fileLock[lockInd].delete == 1) {
    char filename[FILE_LEN];
    strcpy(filename,username);
    strcat(filename,"Data.txt");

    // int pid = fork();
    //if (pid != 0) {
    //close(socket);
    sendDeleteFile(anotherIP,filename);
    //exit(1);
    //}
    //wait(NULL);
    
    //combine the delete file
    combineDeleteFile(filename);
  }


  //unlock the file
  fileLock[lockInd].lock = 0;
  fileLock[lockInd].write = 0;
  fileLock[lockInd].delete = 0;
   
}

int sendWriteFile(char* IP, char* filename) {
  //it works as client side
  int clientSocket;
  struct sockaddr_in serverAddr;
  char buffer[BUFFER_LEN];
  memset(buffer, 0, sizeof(buffer));

  if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }
  printf("[+]Create socket...\n");

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);

  printf("IP: %s\n",IP);
  if (inet_pton(AF_INET, IP, &serverAddr.sin_addr) <= 0){
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }
  printf("[+]Convert the SERVER IP...\n");

  if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("\nConnection Failed \n");
  }
  printf("[+]Connect to the sever...\n");

  int serverIP;
  if (IP == SERVER_HERSCHEL_IP) {
    serverIP = 2;
  } else {
    serverIP = 1;
  }
  
  send(clientSocket,&serverIP,sizeof(serverIP),0);

  //check if the server is lock or not
  int lock;
  recv(clientSocket,&lock,sizeof(lock),0);

  if (lock == 0) {
    //login into the server as root
    send(clientSocket,ROOT,strlen(ROOT),0);

    //check the account if exist
    int userExist;
    recv(clientSocket,&userExist,sizeof(userExist),0);

    //send root's password
    send(clientSocket,ROOT_PASSWORD,strlen(ROOT_PASSWORD),0);

    //check the password
    char pass;
    recv(clientSocket, &pass, sizeof(pass), 0);


    //OPTION 1: ADD USER
    //OPTION 2: WRITE
    //OPTION 3: DELETE

    int option = 2;
    send(clientSocket, &option, sizeof(option), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

     //send write information
    char writeFilePath[FILE_LEN];
    bzero(writeFilePath,FILE_LEN);
    //get the path of write file
    strcat(writeFilePath,DATA_PATH);
    strcat(writeFilePath,WRITE);
    strcat(writeFilePath,filename);

    printf("the write file:%s\n",writeFilePath);
    //read buffer from the file
    FILE *fin;
    fin = fopen(writeFilePath,"r");

    char lineWrite[BUFFER_LEN];
    bzero(lineWrite,BUFFER_LEN);
    while (fgets(lineWrite,sizeof(lineWrite),fin)) {
      strcat(buffer,lineWrite);
      bzero(lineWrite,BUFFER_LEN);
    }
    //send file name
    send(clientSocket, filename, strlen(filename), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

    //send wirte information
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

    close(clientSocket);
    return 1;

  } else {
    printf("The server is locked right now, please try later!\n");
  }
  close(clientSocket);
  return 1;
}

int sendDeleteFile(char* IP, char* filename) {
  //it works as client side
  int clientSocket;
  struct sockaddr_in serverAddr;
  char buffer[BUFFER_LEN];
  memset(buffer, 0, sizeof(buffer));

  if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }
  printf("[+]Create socket...\n");

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);

  printf("IP: %s\n",IP);
  if (inet_pton(AF_INET, IP, &serverAddr.sin_addr) <= 0){
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }
  printf("[+]Convert the SERVER IP...\n");

  if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("\nConnection Failed \n");
  }
  printf("[+]Connect to the sever...\n");

  int serverIP;
  if (IP == SERVER_HERSCHEL_IP) {
    serverIP = 2;
  } else {
    serverIP = 1;
  }

  send(clientSocket,&serverIP,sizeof(serverIP),0);

  //check if the server is lock or not
  int lock;
  recv(clientSocket,&lock,sizeof(lock),0);

  if (lock == 0) {
    //login into the server as root
    send(clientSocket,ROOT,strlen(ROOT),0);

    //check the account if exist
    int userExist;
    recv(clientSocket,&userExist,sizeof(userExist),0);

    //send root's password
    send(clientSocket,ROOT_PASSWORD,strlen(ROOT_PASSWORD),0);

    //check the password
    char pass;
    recv(clientSocket, &pass, sizeof(pass), 0);

    //OPTION 1: ADD USER
    //OPTION 2: WRITE
    //OPTION 3: DELETE

    int option = 3;
    send(clientSocket, &option, sizeof(option), 0);
    recv(clientSocket, &ACK, sizeof(ACK), 0);

    //send delete information
    char delFilePath[FILE_LEN];
    bzero(delFilePath,FILE_LEN);
    //get the path of delete file
    strcat(delFilePath,DATA_PATH);
    strcat(delFilePath,DELETE);
    strcat(delFilePath,filename);

    //read buffer from the file
    FILE *fin;
    fin = fopen(delFilePath,"r");

    char lineDel[BUFFER_LEN];
    bzero(lineDel,BUFFER_LEN);
    while (fgets(lineDel,sizeof(lineDel),fin)) {
      strcat(buffer,lineDel);
      bzero(lineDel,BUFFER_LEN);
    }
    //send user name
    send(clientSocket, filename, strlen(filename), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

    //send the delete buffer
    send(clientSocket, buffer, strlen(buffer), 0);
    recv(clientSocket, &ACK, sizeof(ACK),0);

    close(clientSocket);
    return 1;

  } else {
    printf("The server is locked right now, please try later!\n");
  }
  close(clientSocket);
  return 1;
}
