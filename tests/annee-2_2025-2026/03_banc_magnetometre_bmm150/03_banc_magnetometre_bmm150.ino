/*
 * 03_banc_magnetometre_bmm150
 * Projet Bateau autopiloté · Année 2 (2025-2026) · Banc de test EEE & MIA
 * Auteur : William JUNIOR-ABO
 *
 * Test 3 — Magnétomètre BMM150 (I2C, SDA 22 / SCL 21) : lecture du champ et calcul du cap.
 * Résultat : valeurs de cap obtenues ; lectures encore instables (calibration à améliorer).
 *
 * Source : rapport individuel « Test bench — Report MIA & EEE » (capture du code).
 * La capture s'arrête au calcul du cap : la mise dans [0, 360[ et l'affichage sont complétés.
 */
#include <Wire.h>
#include <bmm150.h>

BMM150 bmm;

void setup() {
  Serial.begin(115200);
  Wire.begin(22, 21);  // SDA, SCL

  if (bmm.initialize() == BMM150_OK) {
    Serial.println("BMM150 OK");
  } else {
    Serial.println("BMM150 FAIL");
  }
}

void loop() {
  bmm.read_mag_data();

  int x = bmm.raw_mag_data.raw_datax;
  int y = bmm.raw_mag_data.raw_datay;

  float heading = atan2(y, x) * 180 / PI;

  // --- complété : cap dans [0, 360[ et affichage ---
  if (heading < 0) heading += 360;
  Serial.printf("X=%d  Y=%d  Cap=%.1f°\n", x, y, heading);
  delay(200);
}
