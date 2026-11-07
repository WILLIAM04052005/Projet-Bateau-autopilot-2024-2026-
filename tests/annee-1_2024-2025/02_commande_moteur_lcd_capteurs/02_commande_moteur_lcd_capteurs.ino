/*
 * 02_commande_moteur_lcd_capteurs
 * Projet Bateau autopiloté · Année 1 (2024-2025) · Domaine EEE
 *
 * Code final du domaine EEE au semestre 2 (simulé sous SimulIDE puis testé) :
 *   - vitesse du moteur réglée au potentiomètre (PWM sur ENA du pont en H) ;
 *   - mesure du courant (ACS712 5 A, 185 mV/A) ;
 *   - mesure des tr/min (capteur infrarouge, roue codeuse 20 trous) ;
 *   - affichage PWM / tr/min / courant sur écran LCD 16x2 (bus parallèle).
 * Carte : Arduino UNO.
 *
 * Source : rapport « EEE DOMAIN S2 Report » (code recopié du PDF). La fin du
 * programme (fin de calculateRPM() et readCurrent()), coupée dans le rapport,
 * est reprise à l'identique de la même version du code (dossier EEE-MIA).
 */
#include <LiquidCrystal.h>

// Initialisation de l'écran LCD (broches réaffectées)
LiquidCrystal lcd(3, 4, 5, 6, 10, 11);

// Définition des broches
#define POTENTIOMETER_PIN A0  // Potentiomètre
#define CURRENT_SENSOR_PIN A1 // Capteur ACS712
#define ENA 9                 // Broche PWM pour contrôle de vitesse
#define IN1 7                 // Direction du moteur
#define IN2 8                 // Direction du moteur
#define SPEED_SENSOR_PIN 2    // Capteur infrarouge

// Variables pour le capteur ACS712
const float ACS712_SENSITIVITY = 0.185; // Sensibilité pour 5A
const int ACS712_OFFSET = 512;          // Valeur moyenne à 0A

// Variables pour le capteur de vitesse
volatile unsigned int pulseCount = 0; // Nombre d'impulsions
float motorRPM = 0.0;                 // RPM du moteur

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

  // Initialisation de l'écran LCD
  lcd.begin(16, 2); // Écran de 16 colonnes et 2 lignes
  lcd.print("Controle moteur");
  delay(2000);
  lcd.clear();

  // Initialisation du moteur
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

void loop() {
  // Lecture du potentiomètre et contrôle du moteur
  int potValue = analogRead(POTENTIOMETER_PIN);    // Lecture du potentiomètre (0 à 1023)
  int motorSpeed = map(potValue, 0, 1023, 0, 255); // Conversion en 0-255
  analogWrite(ENA, motorSpeed);

  // Définir la direction du moteur
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Calcul de la vitesse du moteur
  motorRPM = calculateRPM();

  // Lecture de l'intensité du moteur
  float current = readCurrent();

  // Affichage sur l'écran LCD
  lcd.setCursor(0, 0);
  lcd.print("Vitesse: ");
  lcd.print(motorSpeed);
  lcd.print(" PWM");
  lcd.setCursor(0, 1);
  lcd.print("RPM: ");
  lcd.print(motorRPM, 1); // Une décimale
  lcd.print(" | I: ");
  lcd.print(current, 2);  // Deux décimales
  lcd.print(" A");

  delay(500); // Pause pour actualisation
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
    return (pulses / 20.0) * (60000.0 / timeElapsed); // Conversion en RPM (20 trous)
  } else {
    return 0.0; // Si aucune impulsion détectée
  }
}

// Fonction pour lire l'intensité avec le capteur ACS712
float readCurrent() {
  int sensorValue = analogRead(CURRENT_SENSOR_PIN);
  return (sensorValue - ACS712_OFFSET) * (5.0 / 1024.0) / ACS712_SENSITIVITY;
}
