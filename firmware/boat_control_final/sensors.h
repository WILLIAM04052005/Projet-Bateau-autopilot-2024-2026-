// =====================================================================
//  sensors.h — Capteurs : vitesse (LM393), courant (ACS758), cap (BMM150)
// =====================================================================
#pragma once
#include <Arduino.h>

// --- Vitesse ---
extern int currentRPM;
extern float currentSpeed;
void initSpeedSensor();
void calculateSpeed();         // à appeler dans loop() : met à jour toutes les SPEED_PERIOD_MS

// --- Courant ---
extern float currentAmperes;
void initCurrentSensor();
void updateCurrent();          // à appeler dans loop() : lecture toutes les CURRENT_PERIOD_MS

// --- Magnétomètre / cap ---
extern int magX, magY, magZ;
extern bool magInitialized;
extern float currentHeading;   // cap filtré (°)
bool initMagnetometer();
void updateMagnetometer();     // à appeler dans loop() : lecture à 20 Hz
float calculateHeading();      // cap brut corrigé de l'offset (0-360°)
float smoothHeading(float h);  // moyenne glissante
void calibrateNorth();         // le cap actuel devient 0°
