/*
 * 07_uno_servo_potentiometre
 * Projet Bateau autopiloté · Année 2 (2025-2026) · EEE & MIA
 *
 * Test du servo du safran : l'angle (0-180°) suit le potentiomètre, valeurs affichées sur le moniteur série.
 *
 * Source : dossier « EEE MIA » (code recopié du PDF, indentation restaurée).
 */

#include <Servo.h>
Servo direction;         // Objet servo
const int servoPin = 9;  // Pin signal du servo
const int potPin = A0;   // Pin analogique du potentiomètre
void setup() {
  direction.attach(servoPin);
  Serial.begin(9600);
}
void loop() {
  int potValue = analogRead(potPin);           // Lire la valeur du potentiomètre (0-1023)
  int angle = map(potValue, 0, 1023, 0, 180);  // Convertir en angle servo (0-180)
  direction.write(angle);                      // Positionner le servo
  Serial.print("Potentiomètre: ");
  Serial.print(potValue);
  Serial.print(" -> Angle servo: ");
  Serial.println(angle);
  delay(15);  // Temps pour que le servo atteigne la position
}
