/*
 * 02_banc_pont_h_bts7960
 * Projet Bateau autopiloté · Année 2 (2025-2026) · Banc de test EEE & MIA
 * Auteur : William JUNIOR-ABO
 *
 * Test 2 — Pont en H BTS7960 : commande du moteur 775 en vitesse (PWM 20 kHz, 8 bits) et en sens.
 * Résultat : le moteur tourne en avant et en arrière, vitesse réglable de 0 à 255.
 *
 * Source : rapport individuel « Test bench — Report MIA & EEE » (capture du code).
 * La capture s'arrête après « Stop » : la marche arrière et la fin de loop() sont complétées
 * (marquées « complété »).
 */
#define RPWM 25  // marche avant
#define LPWM 26  // marche arrière

#define CHANNEL_R 0
#define CHANNEL_L 1

void setup() {
  Serial.begin(115200);

  ledcAttach(RPWM, 20000, 8);  // ESP32 core 3.x : 20 kHz, 8 bits
  ledcAttach(LPWM, 20000, 8);
}

void loop() {
  Serial.println("Forward");
  ledcWrite(RPWM, 150);
  ledcWrite(LPWM, 0);
  delay(2000);

  Serial.println("Stop");
  ledcWrite(RPWM, 0);
  ledcWrite(LPWM, 0);
  delay(1000);

  // --- complété : marche arrière puis arrêt ---
  Serial.println("Reverse");
  ledcWrite(RPWM, 0);
  ledcWrite(LPWM, 150);
  delay(2000);

  Serial.println("Stop");
  ledcWrite(RPWM, 0);
  ledcWrite(LPWM, 0);
  delay(1000);
}
