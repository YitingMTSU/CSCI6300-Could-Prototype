#ifndef SERVER_H
#define SERVER_H

#define BUFFER_LEN 1024
#define PORT 3303
#define SERVER_HERSCHEL_IP "161.45.162.78"
#define SERVER_RANGER_IP "161.XX.XX.XXX"
#define USERNAME_LEN 20
#define PASSWORD_LEN 50


const char* interfaceWelcome = "Welcome To Ranger/Herschel Cloud!";
const char* interfaceUser = "Please enter your username:";
const char* interfacePassword = "Password:";
const char* interfaceUsage = "Enter The Option You Want: \n1. Read Own File      2. Rewrite Own File\n 3. Read Others' Files 4. EXIT";
const char* interfaceNewUser = "The account doesn't exist. Do you want to create it?\n 1. Yes 2. Quit";
const char* interfaceReEnterPassword = "Re-Enter Password:";
const char* interfaceBye = "Bye!";
const char* interfacePasswordError = "The password error.";



//log into the account
int login(int socket, char* buffer);

//check if the user exit inside
int checkUser(char* userName, char* password);


#endif



