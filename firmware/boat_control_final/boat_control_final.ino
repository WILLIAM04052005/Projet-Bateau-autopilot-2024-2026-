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
  calculateSpeed();      
  updateCurrent();         
  updateMagnetometer();  

  if (autopilotActive && magInitialized) {
    autopilotControl();   
  }

  handleWebClients();     
  delay(5);
}
