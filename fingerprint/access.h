#ifndef ACCESS_H
#define ACCESS_H

#include <Arduino.h>

void accessSetup();

void accessLoop();

void fingerprintgo();

uint8_t getFingerprintID();

void handleSuccess();

void lcdSuccess();

uint8_t imageHandler(uint8_t p);

void templateHandler(uint8_t p);

void searchHandler(uint8_t p);

void handleFailure();

void lcdFailure();

void relayLogic();

void printSuccess();

void printFailure();

void fingerprintSensorSetup();

void relaySetup();

void lcdSetup();

#endif