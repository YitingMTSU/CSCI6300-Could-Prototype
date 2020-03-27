#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

#define BUFFER_LEN 1024
#define PORT 3303
//#define SERVER_IP "192.168.122.1"
#define SERVER_HERSCHEL_IP "161.45.162.78"
#define SERVER_RANGER_IP "161.45.162.71"
#define USERNAME_LEN 20
#define PASSWORD_LEN 50
#define FILE_LEN 300

const char* interfaceFiles = "The following are the files:";
const char* interfaceChooseFile = "Choose your File";
const char* interfaceWelcome = "Welcome To Ranger/Herschel Cloud!";
const char* interfaceFileLock = "The file is locked!";
const char* interfaceUser = "Please enter your username: ";



//log into the account
int login(int socket, char* buffer);

//show echo or not
void echo(bool on);

//enter into the main menu client side
int mainUsageClient(int socket, char* buffer);



#endif

