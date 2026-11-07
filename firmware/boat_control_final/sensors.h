#pragma once
#include <Arduino.h>

// --- Vitesse ---
extern int currentRPM;
extern float currentSpeed;
void initSpeedSensor();
void calculateSpeed();       

// --- Courant ---
extern float currentAmperes;
void initCurrentSensor();
void updateCurrent();         

// --- Magnétomètre / cap ---
extern int magX, magY, magZ;
extern bool magInitialized;
extern float currentHeading;  
bool initMagnetometer();
void updateMagnetometer();    
float calculateHeading();     
float smoothHeading(float h); 
void calibrateNorth();         