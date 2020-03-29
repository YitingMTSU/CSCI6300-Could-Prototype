#ifndef SERVER_H
#define SERVER_H

#define BUFFER_LEN 1024
#define IP_LEN 20
#define PORT 3303
#define SERVER_HERSCHEL_IP "161.45.162.78"
#define SERVER_RANGER_IP "161.45.162.69"
#define USERNAME_LEN 20
#define PASSWORD_LEN 50
#define FILE_LEN 300
#define WRITE "write"
#define DELETE "delete"
#define ROOT "root"
#define ROOT_PASSWORD "12345"
#define SRC_LEN 4
#define MAX_USER 100

struct FILELOCK{
  char filename[FILE_LEN];
  int lock;
  int write;
  int delete;
};

//log into the account
int login(int socket, char* buffer, char* userName);

//check if the user exit inside
int checkUser(char* userName, char* password);

//write the new user infomation to accountInfo.txt file and creat new data file
void writeNewUserToFile(char* userName, char* password);

//enter into the main menu server side
int mainUsageServer(int socket, char* buffer, char* userName, int lockInd);

//read from a file
void readFile(char* filename, char* buffer);

//show all files in data directory
void showFiles(int socket, char* buffer);

//check if find the file and get the file inforamtion
int findFile(int socket, char* filename, char* fileInfor);

//create tmp write file
void createWriteFile(int socket, char* filename, char* fileInfo);

//create tmp delete file
void createDeleteFile(int socket, char* filename, char* fileInfo);

//combine the write file and original data file
void combineWriteFile(char* filename);

//combine the delete file and the original data file
void combineDeleteFile(char* filename);

//send the information of new user to antoher sever
int sendUserToAnotherServer(char* IP, char* username, char* password);

//synchronizing the information
void rootSyn(int socket, char* buffer);

//set the ACCOUNT_PATH and DATA_PATH
void getPath();

//check the permission for the user
//user call read all the data file
//but only can write/delete for its own file
int checkPermission(char* filename, char* username);

//set fileLock intially
void setFileLock();

//get current file lock index by username
int getCurLockInd(char* username);

//get current file lock index by filename
int getCurLockIndByFileName(char* filename);

//check the if write or delete and send if yes
void checkWD(int socket, int lockInd, char* username);

//send the write file to another server
int sendWriteFile(char* IP, char* filename);

//send the delete file to another server
int sendDeleteFile(char* IP, char* filename);

#endif



