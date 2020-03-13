#ifndef _GPIO_H
#define _GPIO_H

#define GPIO_DIR "/sys/class/gpio/"
#define EXPORT_FILE "/sys/class/gpio/export"
#define UNEXPORT_FILE "/sys/class/gpio/unexport"
#define DIRECTION_DIR "/direction"
#define VALUE_DIR "/value"

#define INPUT "in"
#define OUTPUT "out"
#define HIGH 1
#define LOW 0


int pinMode(const int pPin, char const * pMode);
int digitalWrite(const int pPin, const int pValue);
int digitalRead(const int pPin);
int blink(const int pPin, const int pFreq, const int pDuration);
int releasePin(const int pPin);


#endif
