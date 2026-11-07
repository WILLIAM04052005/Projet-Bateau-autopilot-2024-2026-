/*
 * =====================================================================
 *  Bateau autopiloté — Firmware final (ESP32)
 *  Projet Bateau · ICAM · Bachelor · Année 2025-2026
 *  EEE & MIA : William JUNIOR-ABO
 * =====================================================================
 *
 *  Le bateau crée un réseau Wi-Fi et héberge une interface web :
 *    - Mode manuel  : joystick (safran), marche/arrêt, sens, puissance
 *    - Mode auto    : maintien d'un cap choisi (PID + magnétomètre)
 *    - Télémétrie   : vitesse, tr/min, courant, cap, champ magnétique
 *
 *  Matériel : ESP32 · pont en H BTS7960 · moteur 775 12 V · servo ·
 *             magnétomètre BMM150 (I2C) · capteur de vitesse LM393 ·
 *             capteur de courant ACS758 · convertisseur DC-DC (servo)
 *
 *  Organisation :
 *    config.h        brochage et paramètres
 *    secrets.h       identifiants Wi-Fi (à créer depuis secrets.example.h)
 *    actuators.*     moteur de propulsion et servo du safran
 *    sensors.*       vitesse, courant, magnétomètre
 *    autopilot.*     régulateur PID de cap
 *    web_server.*    Wi-Fi et API HTTP
 *    web_page.h      interface web GÉNÉRÉE depuis ../web (tools/build_web.py)
 *
 *  Bibliothèques : ESP32Servo, BMM150 (bmm150.h) · Carte : ESP32 core 3.x
 * =====================================================================
 */
#include "actuators.h"
#include "autopilot.h"
#include "config.h"
#include "sensors.h"
#include "web_server.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("    Boat Control - Système Complet");
  Serial.println("========================================\n");

  Serial.println(">>> Initialisation Servo...");
  initRudder();
  Serial.println("    ✓ Servo initialisé à 90°\n");

  Serial.println(">>> Initialisation Moteur BTS7960...");
  initMotor();
  Serial.println("    ✓ Moteur initialisé\n");

  Serial.println(">>> Initialisation Capteur de Vitesse...");
  initSpeedSensor();
  Serial.println("    ✓ Capteur de vitesse initialisé (Pin 27)\n");

  Serial.println(">>> Initialisation Capteur de Courant...");
  initCurrentSensor();
  Serial.println("    ✓ Capteur de courant initialisé (Pin 34)\n");

  magInitialized = initMagnetometer();
  if (magInitialized) {
    Serial.println("✓✓✓ MAGNÉTOMÈTRE OPÉRATIONNEL ✓✓✓\n");
  } else {
    Serial.println("✗✗✗ MAGNÉTOMÈTRE NON DÉTECTÉ ✗✗✗");
    Serial.println("L'autopilote sera DÉSACTIVÉ");
    Serial.println("Vérifiez les connexions I2C:\n");
    Serial.println("  BMM150 SDA → ESP32 Pin 22");
    Serial.println("  BMM150 SCL → ESP32 Pin 21");
    Serial.println("  BMM150 VCC → 3.3V");
    Serial.println("  BMM150 GND → GND\n");
  }

  initWiFi();
  initWebServer();

  Serial.println("========================================");
  Serial.println("         SYSTÈME PRÊT");
  Serial.println("========================================\n");

  resetPID();
}

void loop() {
  calculateSpeed();        // tr/min et vitesse (fenêtre 1 s)
  updateCurrent();         // courant moteur (toutes les 500 ms)
  updateMagnetometer();    // champ magnétique (20 Hz)

  if (autopilotActive && magInitialized) {
    autopilotControl();    // PID de cap (20 Hz)
  }

  handleWebClients();      // commandes et télémétrie HTTP
  delay(5);
}
