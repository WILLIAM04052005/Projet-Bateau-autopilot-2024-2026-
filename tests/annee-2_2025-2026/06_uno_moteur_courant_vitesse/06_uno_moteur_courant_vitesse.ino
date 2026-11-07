/*
 * 06_uno_moteur_courant_vitesse
 * Projet Bateau autopiloté · Année 2 (2025-2026) · EEE & MIA
 *
 * Prototype (début d'année) sur Arduino : vitesse au potentiomètre, courant (ACS712 5 A), tr/min (IR, 20 trous).
 * Version sans LCD de la commande moteur de l'année 1, préparée pour la carte embarquée.
 *
 * Source : dossier « EEE MIA » (code recopié du PDF, indentation restaurée).
 */

// Définition des broches
#define POTENTIOMETER_PIN A0   // Potentiomètre
#define CURRENT_SENSOR_PIN A1  // Capteur ACS712
#define ENA 4                  // Broche PWM pour contrôle de vitesse
#define IN1 3                  // Direction du moteur
#define IN2 11                 // Direction du moteur
#define SPEED_SENSOR_PIN 12    // Capteur infrarouge
// Variables pour le capteur ACS712
const float ACS712_SENSITIVITY = 0.185;  // Sensibilité pour 5A
const int ACS712_OFFSET = 512;           // Valeur moyenne à 0A
// Variables pour le capteur de vitesse
volatile unsigned int pulseCount = 0;  // Nombre d'impulsions
float motorRPM = 0.0;                  // RPM du moteur
#include <Arduino.h>

void countPulse();
float calculateRPM();
float readCurrent();

void setup() {
  // Configuration des broches
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(SPEED_SENSOR_PIN, INPUT_PULLUP);
  // Configuration de l'interruption pour le capteur de vitesse
  attachInterrupt(digitalPinToInterrupt(SPEED_SENSOR_PIN), countPulse, FALLING);
  // Initialisation du moteur
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}
void loop() {
  // Lecture du potentiomètre et contrôle du moteur
  int potValue = analogRead(POTENTIOMETER_PIN);     // Lecture du potentiomètre (0 à 1023)
  int motorSpeed = map(potValue, 0, 1023, 0, 255);  // Conversion en 0-255
  analogWrite(ENA, motorSpeed);
  // Définir la direction du moteur
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  // Calcul de la vitesse du moteur
  motorRPM = calculateRPM();
  // Lecture de l'intensité du moteur
  float current = readCurrent();
  // (LCD supprimé — aucune sortie visuelle ici)
  // Vous pouvez ajouter Serial.print() si vous voulez surveiller via le moniteur série
  delay(500);  // Pause pour actualisation
}
// Fonction pour compter les impulsions du capteur de vitesse
void countPulse() {
  pulseCount++;
}
// Fonction pour calculer la vitesse en RPM
float calculateRPM() {
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();
  unsigned int pulses;
  // Calculer le temps écoulé et les impulsions détectées
  noInterrupts();
  pulses = pulseCount;
  pulseCount = 0;
  interrupts();
  unsigned long timeElapsed = currentTime - lastTime;
  lastTime = currentTime;
  if (timeElapsed > 0) {
    return (pulses / 20.0) * (60000.0 / timeElapsed);  // Conversion en RPM
  } else {
    return 0.0;  // Si aucune impulsion détectée
  }
}
// Fonction pour lire l'intensité avec le capteur ACS712
float readCurrent() {
  int sensorValue = analogRead(CURRENT_SENSOR_PIN);
  return (sensorValue - ACS712_OFFSET) * (5.0 / 1024.0) / ACS712_SENSITIVITY;
}
