/*
 * 10_esp32_moteur_page_web
 * Projet Bateau autopiloté · Année 2 (2025-2026) · EEE & MIA
 *
 * Premier essai sur ESP32 : serveur web (WebServer) qui affiche une page
 * de commande du moteur DC (avant / arrière / stop + curseur de vitesse PWM 0-255).
 * C'est l'ancêtre de l'interface finale. La page HTML/CSS/JS est dans web/ et
 * embarquée via page_html.h (généré par tools/build_web.py).
 * API ledcSetup/ledcAttachPin : ESP32 Arduino core 2.x.
 *
 * Source : dossier « EEE MIA » (code recopié du PDF, indentation restaurée).
 */

#include <WiFi.h>
#include <WebServer.h>
const char* ssid = "TON_WIFI";
const char* password = "MOT_DE_PASSE";
WebServer server(80);
// Pins H-Bridge
#define IN1 26
#define IN2 27
#define ENA 25
int vitesse = 0;
int pwmChannel = 0;
int pwmFreq = 20000;
int pwmResolution = 8;
// ================= HTML =================
// Page générée depuis web/ : python ../../../firmware/tools/build_web.py --src web --out page_html.h --var pageHTML
#include "page_html.h"
// =======================================
void handleRoot() {
  server.send(200, "text/html", pageHTML);
}
void moteurAvant() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  server.send(200, "text/plain", "AVANT");
}
void moteurArriere() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  server.send(200, "text/plain", "ARRIERE");
}
void moteurStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  server.send(200, "text/plain", "STOP");
}
void setVitesse() {
  if (server.hasArg("val")) {
    vitesse = server.arg("val").toInt();
    ledcWrite(pwmChannel, vitesse);
  }
  server.send(200, "text/plain", "OK");
}
void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ENA, pwmChannel);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté !");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/avant", moteurAvant);
  server.on("/arriere", moteurArriere);
  server.on("/stop", moteurStop);
  server.on("/vitesse", setVitesse);
  server.begin();
}
void loop() {
  server.handleClient();
}
