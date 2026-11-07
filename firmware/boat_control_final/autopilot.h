#pragma once
#include <Arduino.h>

extern bool autopilotActive;
extern int targetHeading;    

void resetPID();           
void autopilotControl();   
