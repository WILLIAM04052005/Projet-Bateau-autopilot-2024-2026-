// =====================================================================
//  actuators.h — Propulsion (BTS7960) et direction (servo du safran)
// =====================================================================
#pragma once
#include <Arduino.h>

// État de la propulsion, piloté depuis l'interface web
extern String direction;   // "fwd", "rev" ou "stop"
extern int speedVal;       // rapport cyclique PWM 0-255
extern bool motorOn;

void initMotor();
void applyMotor();         // applique direction / vitesse / marche-arrêt

void initRudder();
void applyServo(int angle);// déplacement progressif et borné du safran
