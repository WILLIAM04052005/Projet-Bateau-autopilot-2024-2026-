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
