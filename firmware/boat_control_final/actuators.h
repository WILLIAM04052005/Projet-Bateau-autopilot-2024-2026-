//  actuators.h — Propulsion (BTS7960) et direction (servo du safran)
#pragma once
#include <Arduino.h>

// État de la propulsion, piloté depuis l'interface web
extern String direction;   // "fwd", "rev" ou "stop"
extern int speedVal;       
extern bool motorOn;

void initMotor();
void applyMotor();         

void initRudder();
void applyServo(int angle);
