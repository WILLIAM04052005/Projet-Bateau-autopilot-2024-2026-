/*
 * 04_banc_capteur_vitesse
 * Projet Bateau autopiloté · Année 2 (2025-2026) · Banc de test EEE & MIA
 * Auteur : William JUNIOR-ABO
 *
 * Test 4 — Capteur de vitesse (fourche optique LM393) : comptage des impulsions par interruption,
 * calcul des tr/min sur 1 s. Résultat : vitesse de rotation du moteur mesurée.
 *
 * Source : rapport individuel « Test bench — Report MIA & EEE » (capture du code).
 * La capture s'arrête après l'affichage : la remise à zéro du compteur est complétée.
 */
#define SENSOR_PIN 27

volatile int pulses = 0;

void IRAM_ATTR countPulse() {
  pulses++;
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), countPulse, FALLING);
}

void loop() {
  delay(1000);

  int rpm = pulses * 60;  // 1 pulse per rotation assumed

  Serial.print("Pulses: ");
  Serial.print(pulses);
  Serial.print(" | RPM: ");
  Serial.println(rpm);

  pulses = 0;  // --- complété : nouvelle fenêtre de mesure ---
}
