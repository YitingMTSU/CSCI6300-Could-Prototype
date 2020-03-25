#ifndef SERVER_H
#define SERVER_H

#define BUFFER_LEN 1024
#define IP_LEN 20
#define PORT 3303
#define SERVER_HERSCHEL_IP "161.45.162.78"
#define SERVER_RANGER_IP "161.45.162.71"
#define USERNAME_LEN 20
#define PASSWORD_LEN 50
#define FILE_LEN 300
#define WRITE "write"
#define DELETE "delete"
#define ROOT "root"
#define ROOT_PASSWORD "12345"
#define SRC_LEN 4

const char* interfaceWelcome = "Welcome To Ranger/Herschel Cloud!";
const char* interfaceUser = "Please enter your username: ";
const char* interfacePassword = "Password:";
const char* interfaceUsage = "Enter The Option You Want: \n1. Read Files         2. Write Information\n3. Delete information 4. EXIT";
const char* interfaceNewUser = "The account doesn't exist. Do you want to create it?\n1. Yes    2. Quit";
const char* interfaceReEnterPassword = "Re-Enter Password:";
const char* interfaceBye = "Bye!";
const char* interfacePasswordError = "The password error.";
const char* interfaceOption = "Please Enter Your Option: ";


//log into the account
int login(int socket, char* buffer, char* userName);

//check if the user exit inside
int checkUser(char* userName, char* password);

//write the new user infomation to accountInfo.txt file and creat new data file
void writeNewUserToFile(char* userName, char* password);

//enter into the main menu server side
int mainUsageServer(int socket, char* buffer, char* userName);

//read from a file
void readFile(char* filename, char* buffer);

//show all files in data directory
void showFiles(int socket, char* buffer);

//send the information of the file to the user
int sendFileInfor(int socket, char* filename);

//create tmp write file
void createWriteFile(int socket, char* filename);

//create tmp delete file
void createDeleteFile(int socket, char* filename);

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

#endif



