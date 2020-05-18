#ifndef _CONSTS_H
#define _CONSTS_H

enum ROOMS {KITCHEN, BEDROOM_1, BEDROOM_2, LIVING, DINING, BATHROOM};
enum ROOMS_DOOR {BEDROOM_1_DOOR, BEDROOM_2_DOOR, BATHROOM_DOOR, MAIN_DOOR, GARAGE_DOOR};
int LIGHTS[] = {14, 15, 18, 23, 24, 25};
int DOORS[] = {17, 27, 22, 13, 19};
//{17, 27, 22, 5, 6, 26, 16};
int ENCRYPTION_LENGTH = 256;

static const char * USER = "admin";
static const char * PASS = "embedded2020";

static const char * INPUT_FILE = "srv_input.txt";
static const char * LOG_FILE = "Log_server.txt";
static const char * PRIVATE_KEY_FILE = "home/root/private.pem";
static const char * PICTURE_1 = "/home/root/pictures/pic_1.jpg";
static const char * PICTURE_2 = "/home/root/pictures/pic_2.jpg";
static const char * PICTURE_3 = "/home/root/pictures/pic_3.jpg";

static const char * OK = "Ok";
static const char * VALUE = "Value";
static const char * ERROR = "Error";
static const char * MESSAGE = "Message";

static const char * LIGHT_OK = "Light turned ON/OFF :D";
static const char * LIGHT_ERR = "Problem with the light :/";
static const char * DOOR_OK = "Door OPEN/CLOSE :D";
static const char * DOOR_ERR = "Problem with the door :/";
static const char * LOGIN_OK = "Logic successful... Welcome!";
static const char * LOGIN_ERR = "Problem with login, provide user and password";
static const char * USER_ERR = "User doesn't exist";
static const char * PASS_ERR = "Incorrect password";
static const char * ROOM_ERR = "This room doesn't exist ¯\_(ツ)_/¯";
static const char * PIC_ERR = "Problem reading picture";



#endif
