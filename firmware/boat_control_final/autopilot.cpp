#include "autopilot.h"
#include "actuators.h"
#include "config.h"
#include "sensors.h"

bool autopilotActive = false;
int targetHeading = 0;

static float lastError = 0;
static float integralError = 0;
static unsigned long lastPIDTime = 0;

void resetPID() {
  lastError = 0;
  integralError = 0;
  lastPIDTime = millis();
}

void autopilotControl() {
  if (!autopilotActive || !magInitialized) return;

  unsigned long now = millis();
  float dt = (now - lastPIDTime) / 1000.0;
  if (dt < PID_PERIOD_S) return;  
  lastPIDTime = now;

  // Cap mesuré, filtré
  currentHeading = smoothHeading(calculateHeading());

  float error = targetHeading - currentHeading;
  if (error > 180) error -= 360;
  if (error < -180) error += 360;

  if (abs(error) < HEADING_DEADBAND) {
    applyServo(SERVO_CENTER);
    return;
  }

  //PID control
  float P = KP * error;

  integralError += error * dt;
  integralError = constrain(integralError, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);  // anti-windup
  float I = KI * integralError;

  float D = KD * (error - lastError) / dt;
  lastError = error;

  int servoAngle = SERVO_CENTER + P + I + D;
  servoAngle = constrain(servoAngle, AUTO_SERVO_MIN, AUTO_SERVO_MAX);
  if (abs(servoAngle - SERVO_CENTER) < AUTO_SERVO_STABLE) servoAngle = SERVO_CENTER;

  applyServo(servoAngle);

  Serial.printf("AUTO | Target:%d° Current:%.1f° Error:%.1f Servo:%d\n",
                targetHeading, currentHeading, error, servoAngle);
}
