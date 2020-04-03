#ifndef _CONSTS_H
#define _CONSTS_H

enum ROOMS {KITCHEN, BEDROOM_1, BEDROOM_2, LIVING, DINING, BATHROOM, MAIN};
int LIGHTS[] = {24};
int DOORS[] = {23};

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



#endif
