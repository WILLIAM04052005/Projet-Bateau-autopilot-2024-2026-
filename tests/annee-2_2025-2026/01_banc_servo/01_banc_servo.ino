/*
 * 01_banc_servo
 * Projet Bateau autopiloté · Année 2 (2025-2026) · Banc de test EEE & MIA
 * Auteur : William JUNIOR-ABO
 *
 * Test 1 — Servo de direction (safran) : balayage 0° → 90° → 180°.
 * Résultat attendu : débattement complet 0-180°. Carte : ESP32 · Bibliothèque : ESP32Servo.
 *
 * Source : rapport individuel « Test bench — Report MIA & EEE » (capture du code).
 */
#include <ESP32Servo.h>

Servo testServo;
const int servoPin = 18;

void setup() {
  Serial.begin(115200);
  testServo.attach(servoPin, 500, 2400);  // impulsions 500-2400 µs
}

void loop() {
  Serial.println("0°");
  testServo.write(0);
  delay(1000);

  Serial.println("90°");
  testServo.write(90);
  delay(1000);

  Serial.println("180°");
  testServo.write(180);
  delay(1000);
}
