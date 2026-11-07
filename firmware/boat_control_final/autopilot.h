// =====================================================================
//  autopilot.h — Maintien de cap par régulateur PID sur le safran
// =====================================================================
#pragma once
#include <Arduino.h>

extern bool autopilotActive;
extern int targetHeading;    // cap de consigne (°)

void resetPID();             // remet à zéro intégrale et dérivée (à l'activation)
void autopilotControl();     // à appeler dans loop() : boucle à 20 Hz
