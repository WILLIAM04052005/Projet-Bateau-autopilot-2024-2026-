//  actuators.cpp — Propulsion (BTS7960) et direction (servo du safran)
#include "actuators.h"
#include "config.h"
#include <ESP32Servo.h>

String direction = "stop";
int speedVal = 0;
bool motorOn = false;

static Servo rudderServo;

// Moteur 
void initMotor() {
  ledcAttach(RPWM, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(LPWM, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(RPWM, 0);
  ledcWrite(LPWM, 0);
}

void applyMotor() {
  if (!motorOn) {
    ledcWrite(RPWM, 0);
    ledcWrite(LPWM, 0);
    return;
  }
  if (direction == "fwd") {          // marche avant : PWM sur RPWM
    ledcWrite(LPWM, 0);
    ledcWrite(RPWM, speedVal);
  } else if (direction == "rev") {   // marche arrière : PWM sur LPWM
    ledcWrite(RPWM, 0);
    ledcWrite(LPWM, speedVal);
  } else {                           // stop
    ledcWrite(RPWM, 0);
    ledcWrite(LPWM, 0);
  }
}

// Safran
void initRudder() {
  rudderServo.attach(SERVO_PIN);
  rudderServo.write(SERVO_CENTER);
}

void applyServo(int angle) {
  static unsigned long lastMove = 0;
  if (millis() - lastMove < SERVO_UPDATE_MS) return;
  lastMove = millis();

  angle = constrain(angle, SERVO_MIN, SERVO_MAX);

  static int lastAngle = SERVO_CENTER;
  if (abs(angle - lastAngle) > SERVO_STEP) {
    if (angle > lastAngle) lastAngle += SERVO_STEP;
    else lastAngle -= SERVO_STEP;
  } else {
    lastAngle = angle;
  }

  rudderServo.write(lastAngle);
}
