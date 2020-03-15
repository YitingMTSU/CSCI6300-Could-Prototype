#ifndef SERVER_H
#define SERVER_H

#define BUFFER_LEN 1024
#define PORT 3303
#define SERVER_HERSCHEL_IP "161.45.162.78"
#define SERVER_RANGER_IP "161.45.162.71"
#define USERNAME_LEN 20
#define PASSWORD_LEN 50
#define FILE_LEN 300
#define ACCOUNT_PATH "/nfshome/yw3c/CSCI6430/CSCI6300-Could-Prototype/server/data/userAccount/userInfo.txt"
#define DATA_PATH "/nfshome/yw3c/CSCI6430/CSCI6300-Could-Prototyp\
e/server/data/userData/"

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

#endif



