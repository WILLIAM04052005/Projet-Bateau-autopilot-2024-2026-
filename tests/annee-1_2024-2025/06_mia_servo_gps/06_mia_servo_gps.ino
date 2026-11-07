#include <LiquidCrystal.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Initialisation des composants
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo myServo;
SoftwareSerial gps1Serial(10, 13), gps2Serial(8, 6);
TinyGPSPlus gps1, gps2;

// Variables
int potPin = A1, potValue, angle, currentAngle, tolerance = 5;
String lat1 = "N/A", lon1 = "N/A", lat2 = "N/A", lon2 = "N/A";

void updateGPS();

void setup() {
  lcd.begin(16, 2);
  lcd.print("Navigation Auto");
  myServo.attach(9);
  gps1Serial.begin(9600);
  gps2Serial.begin(9600);
}

void loop() {
  // Lecture du potentiomètre et ajustement de l'angle
  potValue = analogRead(potPin);
  angle = map(potValue, 0, 1023, 0, 180);
  currentAngle = myServo.read();
  if (abs(currentAngle - angle) > tolerance) {
    myServo.write(currentAngle < angle ? currentAngle + 1 : currentAngle - 1);
  }

  // Mise à jour et affichage des coordonnées GPS
  updateGPS();
  lcd.setCursor(0, 0);
  lcd.print("Angle: ");
  lcd.print(angle);
  lcd.setCursor(0, 1);
  lcd.print("Lat1: ");
  lcd.print(lat1);
  lcd.print(" Lon1: ");
  lcd.print(lon1);
}

void updateGPS() {
  while (gps1Serial.available() > 0) gps1.encode(gps1Serial.read());
  while (gps2Serial.available() > 0) gps2.encode(gps2Serial.read());
  if (gps1.location.isUpdated()) {
    lat1 = String(gps1.location.lat(), 6);
    lon1 = String(gps1.location.lng(), 6);
  }
  if (gps2.location.isUpdated()) {
    lat2 = String(gps2.location.lat(), 6);
    lon2 = String(gps2.location.lng(), 6);
  }
}
