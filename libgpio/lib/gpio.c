#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <gpio.h>

/**
 * Sets the mode (in/out) of a gpio pin 
 * 
 * Error codes:
 *      0 = Successful
 *      -1 = Undefined pin 
 *      -2 = Undefined pin mde
 *      -3 = Unable to open file
 *      -4 = Unable to write
 */
int pinMode(const int pPin, char const * pMode){
    //Check pin number
    if(pPin < 1 || pPin > 40) return -1;
    if((strcmp(pMode, "in") != 0) && (strcmp(pMode, "out") != 0))
        return -2;
    
    //Export the pin
    int fd = open(EXPORT_FILE, O_WRONLY);
    if(fd == -1) return -3;
    
    char pin[2];
    sprintf(pin, "%d", pPin);
    if(write(fd, pin, 2) != 2) return -4;
    
    //Set pin mode
    
    //First create the path /sys/class/gpio/<gpioPIN>/direction
    char path[100] = "";
    char pinDir[7] = "gpio";
    strcat(pinDir, pin);
    strcat(path, GPIO_DIR);
    strcat(path, pinDir);
    strcat(path, DIRECTION_DIR);
    
    fd = open(path, O_WRONLY);
    if(fd == -1) return -2;
    
    if(write(fd, pMode, 3) != 3) return -3;
    close(fd);
    return 0;    
}


/**
 * Writes a value of 1 (HIGH) or 0 (LOW) to the specified pinDir
 * 
 * Error codes: 
 *      0 = Successful
 *      -1 = Undefined pin
 *      -2 = Undefined value (only 1 or 0)
 *      -3 = Unable to open value directory for the specified pin
 *      -4 = Error writing the value of the pin
 */
int digitalWrite(const int pPin, const int pValue){
    //Check pin number
    if(pPin < 1 || pPin > 40) return -1;
    if(pValue < 0 || pValue > 1) return -2;
    
    char pin[2];
    sprintf(pin, "%d", pPin);
    
    //Construction of the gpio value directory
    char path[100] = "";
    char pinDir[7] = "gpio";
    strcat(pinDir, pin);
    strcat(path, GPIO_DIR);
    strcat(path, pinDir);
    strcat(path, VALUE_DIR);
    
    int fd = open(path, O_WRONLY);
    if(fd == -1) return -3;
    
    char value[1];
    sprintf(value, "%d", pValue);
    
    if(write(fd, value, 1) != 1) return -4;
    close(fd);  
    
    return 0;
}


/**
 * Reads the value of a pin configured as input
 * 
 * Error codes:
 *      -1 = Undefined pin
 *      -2 = Unable to open value file of the pin
 *      -3 = Error reading the value file
 *      -4 = Value readed is undefined
 */
int digitalRead(const int pPin){
    //Check pin number
    if(pPin < 1 || pPin > 40) return -1;
    
    char pin[2];
    sprintf(pin, "%d", pPin);
    
    //Construction of the gpio value directory
    char path[100] = "";
    char pinDir[7] = "gpio";
    strcat(pinDir, pin);
    strcat(path, GPIO_DIR);
    strcat(path, pinDir);
    strcat(path, VALUE_DIR);
    
    int fd = open(path, O_RDONLY);
    if(fd == -1) return -2;
    
    char valueStr[1];
    if(read(fd, valueStr, 1) != 1) return -3;
    
    close(fd);
    
    if(valueStr[0] == '0') return 0;
    else if(valueStr[0] == '1') return 1;
    else return -4;
}


/**
 * Cycles the output of a pin between HIGH and LOW with the specified
 * frequency and during the specified duration 
 * 
 * Error codes:
 *      0 = Successful
 *      -1 = Undefined pin 
 *      -2 = Invalid frequency or duration
 *      -3 = Unable to open value file of the pin 
 *          (may be due because the pin mode has not been specified)
 */
int blink(const int pPin, const int pFreq, const int pDuration){
    //Check parameters
    if(pPin < 1 || pPin > 40) return -1;
    if(pFreq < 0 || pDuration < 0) return -2;
    
    char pin[2];
    sprintf(pin, "%d", pPin);
    
    int nCycles = pDuration * pFreq;
    int halfPeriod = (((float)1/pFreq) * 1000000) / 2;
    
    //Construction of the gpio value directory
    char path[100] = "";
    char pinDir[7] = "gpio";
    strcat(pinDir, pin);
    strcat(path, GPIO_DIR);
    strcat(path, pinDir);
    strcat(path, VALUE_DIR);
    
    int fd = open(path, O_WRONLY);
    if(fd == -1) return -3;
    else{
        close(fd);
        for(int i=0; i<nCycles; i++){
            digitalWrite(pPin, HIGH);
            usleep(halfPeriod);
            digitalWrite(pPin, LOW);
            usleep(halfPeriod);
        }
    } 
    
    return 0;
}


/*
 * Releases a pin
 * 
 * Error codes:
 *      0 = Successful
 *      -1 = Undefined pin
 *      -2 = Unable to open unexport file
 *      -3 = Error writin to unexport file
 */
int releasePin(const int pPin){
    
    //Check pin number
    if(pPin < 1 || pPin > 40) return -1;
    
    char pin[2];
    sprintf(pin, "%d", pPin);
    
    int fd = open(UNEXPORT_FILE, O_WRONLY);
    if(fd == -1) return -2;
    
    if(write(fd, pin, 2) != 2) return -3;
    close(fd);
    
    return 0;   
}

