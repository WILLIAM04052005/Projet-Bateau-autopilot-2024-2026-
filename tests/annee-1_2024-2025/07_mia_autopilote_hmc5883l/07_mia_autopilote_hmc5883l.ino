/*
 * 07_mia_autopilote_hmc5883l
 * Projet Bateau autopiloté · Année 1 (2024-2025) · Domaine MIA, semestre 2
 *
 * Autopilote simple : le potentiomètre fixe le cap de consigne, le
 * magnétomètre HMC5883L (GY-271) mesure le cap réel, et le safran corrige
 * proportionnellement à l'erreur :
 *     erreur = consigne − cap   (ramenée dans [−180°, 180°])
 *     angle_safran = 90° + erreur   (borné à 0°–180°)
 * Exemple de la soutenance : consigne 120°, cap 80° → erreur 40° → safran 130°.
 * Carte : Arduino UNO · Bibliothèques : Wire, Servo, HMC5883L (Korneliusz Jarzebski)
 *
 * Source : présentation de soutenance du semestre 2 (capture du code).
 * La fonction setup() n'apparaissait pas sur la capture : elle est ajoutée
 * ici (broche du servo et initialisation du capteur à adapter au montage).
 */
#include <Wire.h>
#include <Servo.h>
#include <HMC5883L.h>

Servo rudder;
HMC5883L compass;

// ---- Ajouté (absent de la capture) ----
void setup() {
  Serial.begin(9600);
  rudder.attach(9);   // broche du servo du safran (à adapter)
  rudder.write(90);   // safran au centre
  while (!compass.begin()) {
    Serial.println("HMC5883L introuvable, vérifier le câblage I2C");
    delay(500);
  }
}
// ---------------------------------------

void loop() {
  int target = map(analogRead(A2), 0, 1023, 0, 180);
  Vector dir = compass.readNormalize();
  int heading = atan2(dir.YAxis, dir.XAxis) * 180 / PI;
  if (heading < 0) heading += 360;

  int error = target - heading;
  if (error > 180) error -= 360;
  if (error < -180) error += 360;

  rudder.write(constrain(90 + error, 0, 180));
}
