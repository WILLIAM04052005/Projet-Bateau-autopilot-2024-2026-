/*
 * 03_mesure_courant_vitesse_lcd_i2c
 * Projet Bateau autopiloté · Année 1 (2024-2025) · Domaine EEE / MIA
 *
 * Banc de commande moteur : vitesse au potentiomètre, sens au bouton (anti-rebond),
 * mesure du courant (ACS712 5 A, calibration du zéro au démarrage, moyenne de 50 mesures),
 * tr/min (capteur IR, roue 20 trous) et affichage sur LCD I2C 16x2. Carte : Arduino UNO.
 *
 * Fichier d'origine : MeasureCurrent/MeasureCurrent.ino
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2); // Using 2 lines (not 4) for 16x2 display

#define POTENTIOMETER_PIN A0
#define CURRENT_SENSOR_PIN A1
#define IN1 10
#define IN2 9
#define SPEED_SENSOR_PIN 2
#define DIRECTION_BUTTON_PIN 4

const float ACS712_SENSITIVITY = 0.185; // 185 mV/A for 5A version
int acs712Offset = 512; // Will be calibrated at startup

volatile unsigned int pulseCount = 0;
float motorRPM = 0.0;

bool motorDirection = true;
bool lastButtonState = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 30;

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(SPEED_SENSOR_PIN, INPUT_PULLUP);
  pinMode(DIRECTION_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(SPEED_SENSOR_PIN), countPulse, FALLING);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Controle moteur");
  delay(2000);
  lcd.clear();

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // Calibrate ACS712 zero-current voltage
  long offsetSum = 0;
  for (int i = 0; i < 100; i++) {
    offsetSum += analogRead(CURRENT_SENSOR_PIN);
    delay(5);
  }
  acs712Offset = offsetSum / 100;
}

void loop() {
  // Debounce button logic
  bool reading = digitalRead(DIRECTION_BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        motorDirection = !motorDirection;
      }
    }
  }
  lastButtonState = reading;

  // Read potentiometer and control motor speed
  int potValue = analogRead(POTENTIOMETER_PIN);
  int motorSpeed = map(potValue, 0, 1023, 0, 255);

  if (motorDirection) {
    analogWrite(IN1, motorSpeed);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    analogWrite(IN2, motorSpeed);
  }

  motorRPM = calculateRPM();
  float current = readCurrentAverage(50); // 50-sample average

  // Update LCD
  lcd.setCursor(0, 0);
  lcd.print("PWM:");
  lcd.print(motorSpeed);
  lcd.print(" RPM:");
  lcd.print(motorRPM, 0);
  lcd.print("   "); // Clear leftover chars

  lcd.setCursor(0, 1);
  lcd.print("I:");
  lcd.print(current, 2);
  lcd.print(" A       "); // Padding

  delay(300); // Reasonable refresh delay
}

void countPulse() {
  pulseCount++;
}

float calculateRPM() {
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();
  unsigned int pulses;

  noInterrupts();
  pulses = pulseCount;
  pulseCount = 0;
  interrupts();

  unsigned long timeElapsed = currentTime - lastTime;
  lastTime = currentTime;

  if (timeElapsed > 0) {
    return (pulses / 20.0) * (60000.0 / timeElapsed); // Adjust 20 for your encoder
  } else {
    return 0.0;
  }
}

// Average several readings for better accuracy
float readCurrentAverage(int samples) {
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(CURRENT_SENSOR_PIN);
    delayMicroseconds(200);
  }
  float avgValue = total / (float)samples;
  float voltage = (avgValue * 5.0) / 1024.0;
  float offsetVoltage = (acs712Offset * 5.0) / 1024.0;
  float current = (voltage - offsetVoltage) / ACS712_SENSITIVITY;
  return current;
}
