/*
 * 05_banc_capteur_courant
 * Projet Bateau autopiloté · Année 2 (2025-2026) · Banc de test EEE & MIA
 * Auteur : William JUNIOR-ABO
 *
 * Test 5 — Capteur de courant ACS758 50 A (40 mV/A) sur l'ADC 12 bits de l'ESP32.
 * I = (V − 1,65 V) / 0,04. Résultat : courant consommé par le moteur mesuré
 * (le calage du zéro a ensuite été affiné dans le code final).
 *
 * Source : rapport individuel « Test bench — Report MIA & EEE » (capture du code).
 */
#define PIN_MES 34

float Vref = 3.3;
int ADCmax = 4095;
float sensitivity = 0.04;  // V/A

void setup() {
  Serial.begin(115200);
}

void loop() {
  int raw = analogRead(PIN_MES);
  float voltage = (raw * Vref) / ADCmax;

  float current = (voltage - 1.65) / sensitivity;

  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V | Current: ");
  Serial.print(current);
  Serial.println(" A");

  delay(500);
}
