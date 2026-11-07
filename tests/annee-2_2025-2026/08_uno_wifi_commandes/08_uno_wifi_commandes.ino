/*
 * 08_uno_wifi_commandes
 * Projet Bateau autopiloté · Année 2 (2025-2026) · EEE & MIA
 *
 * Première commande Wi-Fi (Arduino UNO WiFi Rev2, WiFiNINA) : la carte rejoint le réseau « Bateau » et reçoit
 * des requêtes HTTP de l'application MIT App Inventor : /avant, /stop, /gauche, /droite, /centre.
 * Limite constatée : l'UNO WiFi Rev2 ne peut pas créer son propre point d'accès → passage à l'ESP32.
 *
 * Source : dossier « EEE MIA » (code recopié du PDF, indentation restaurée).
 */

#include <WiFiNINA.h>
#include <Servo.h>
// --- Configuration Wi-Fi ---
const char* ssid = "Bateau";
const char* password = "12345678";
// --- Pins moteur H-bridge ---
const int IN1 = 5;
const int IN2 = 4;
const int ENA = 6;  // PWM pour vitesse moteur
// --- Pin servo ---
const int servoPin = 9;
Servo direction;
WiFiServer server(80);
void setup() {
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  // Au démarrage moteur à l'arrêt
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  direction.attach(servoPin);
  direction.write(90);  // servo centré
  Serial.print("Connexion Wi-Fi...");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, password);
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\nConnecté !");
  Serial.print("IP locale: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
void loop() {
  WiFiClient client = server.available();
  if (client) {
    String requete = client.readStringUntil('\r');
    client.flush();
    if (requete.indexOf("/avant") != -1) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, 255);  // vitesse max
      Serial.println("AVANT");
    } else if (requete.indexOf("/stop") != -1) {
      analogWrite(ENA, 0);  // arrêt moteur
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      Serial.println("STOP");
    } else if (requete.indexOf("/gauche") != -1) {
      direction.write(45);
      Serial.println("GAUCHE");
    } else if (requete.indexOf("/droite") != -1) {
      direction.write(135);
      Serial.println("DROITE");
    } else if (requete.indexOf("/centre") != -1) {
      direction.write(90);
      Serial.println("CENTRE");
    }
    // Réponse HTTP simple
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>Commande reçue !</h1>");
    client.stop();
  }
}
