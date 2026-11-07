/*
 * Code Arduino/ESP32 pour contrôle de bateau électrique
 * ======================================================
 * 
 * Ce code crée un serveur web qui reçoit les commandes de l'interface React
 * et renvoie les données de télémétrie.
 * 
 * Endpoints:
 * - GET  /status     -> Vérifie la connexion
 * - GET  /telemetry  -> Retourne les données des capteurs
 * - POST /command    -> Reçoit les commandes (heading, power, motorOn, gear)
 * 
 * CONFIGURATION:
 * - Modifiez WIFI_SSID et WIFI_PASSWORD pour le mode Station
 * - Ou utilisez le mode AP (Access Point) avec les valeurs par défaut
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ============ CONFIGURATION WiFi ============
// Mode Station (se connecte à votre réseau WiFi)
// const char* WIFI_SSID = "VotreSSID";
// const char* WIFI_PASSWORD = "VotreMotDePasse";
// #define WIFI_MODE_AP false

// Mode Access Point (crée son propre réseau)
const char* AP_SSID = "BoatControl";
const char* AP_PASSWORD = "12345678";
#define WIFI_MODE_AP true

// ============ PINS (À adapter selon votre montage) ============
#define PIN_MOTOR_PWM     25    // PWM pour contrôle puissance moteur
#define PIN_MOTOR_ENABLE  26    // Enable moteur
#define PIN_MOTOR_DIR_A   27    // Direction A (forward)
#define PIN_MOTOR_DIR_B   14    // Direction B (reverse)
#define PIN_SERVO_DIR     13    // Servo pour direction

#define PIN_VOLTAGE_SENSOR 34   // Capteur tension (ADC)
#define PIN_CURRENT_SENSOR 35   // Capteur courant (ADC)
#define PIN_SPEED_SENSOR   32   // Capteur vitesse (pulse)

// ============ VARIABLES GLOBALES ============
WebServer server(80);

// État des commandes
int currentHeading = 0;      // 0-360 degrés
int currentPower = 0;        // 0-100%
bool motorOn = false;
String currentGear = "neutral"; // forward, neutral, reverse

// Télémétrie
float actualPower = 0;
float current = 0;
float speed = 0;
float voltage = 48.0;

// Variables pour calcul vitesse
volatile unsigned long pulseCount = 0;
unsigned long lastSpeedCalc = 0;

// ============ INTERRUPTION CAPTEUR VITESSE ============
void IRAM_ATTR speedPulseISR() {
  pulseCount++;
}

// ============ HEADERS CORS ============
void sendCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ============ HANDLERS HTTP ============

// OPTIONS pour CORS preflight
void handleOptions() {
  sendCorsHeaders();
  server.send(204);
}

// GET /status - Vérifie la connexion
void handleStatus() {
  sendCorsHeaders();
  server.send(200, "application/json", "{\"status\":\"ok\",\"version\":\"1.0\"}");
}

// GET /telemetry - Retourne les données capteurs
void handleTelemetry() {
  sendCorsHeaders();
  
  StaticJsonDocument<200> doc;
  doc["actualPower"] = actualPower;
  doc["current"] = current;
  doc["speed"] = speed;
  doc["voltage"] = voltage;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// POST /command - Reçoit les commandes
void handleCommand() {
  sendCorsHeaders();
  
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    // Parse les commandes
    if (doc.containsKey("heading")) {
      currentHeading = doc["heading"].as<int>();
      applyDirection();
    }
    
    if (doc.containsKey("power")) {
      currentPower = doc["power"].as<int>();
      applyPower();
    }
    
    if (doc.containsKey("motorOn")) {
      motorOn = doc["motorOn"].as<bool>();
      applyMotorState();
    }
    
    if (doc.containsKey("gear")) {
      currentGear = doc["gear"].as<String>();
      applyGear();
    }
    
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
  }
}

// ============ FONCTIONS DE CONTRÔLE MOTEUR ============

void applyDirection() {
  // Convertit le heading (0-360) en position servo
  // Adaptez selon votre servo et montage
  int servoPos = map(currentHeading, 0, 360, 0, 180);
  
  // Si vous utilisez la librairie ESP32Servo:
  // directionServo.write(servoPos);
  
  // Ou avec ledcWrite pour PWM:
  // int pwmValue = map(servoPos, 0, 180, 500, 2500); // microsecondes
  // ledcWrite(0, pwmValue);
  
  Serial.printf("Direction: %d° -> Servo: %d\n", currentHeading, servoPos);
}

void applyPower() {
  if (motorOn && currentGear != "neutral") {
    int pwmValue = map(currentPower, 0, 100, 0, 255);
    ledcWrite(1, pwmValue);
    actualPower = currentPower;
  } else {
    ledcWrite(1, 0);
    actualPower = 0;
  }
  Serial.printf("Power: %d%% -> PWM: %d\n", currentPower, (int)actualPower);
}

void applyMotorState() {
  digitalWrite(PIN_MOTOR_ENABLE, motorOn ? HIGH : LOW);
  if (!motorOn) {
    ledcWrite(1, 0);
    actualPower = 0;
  }
  Serial.printf("Motor: %s\n", motorOn ? "ON" : "OFF");
  applyPower();
}

void applyGear() {
  if (currentGear == "forward") {
    digitalWrite(PIN_MOTOR_DIR_A, HIGH);
    digitalWrite(PIN_MOTOR_DIR_B, LOW);
  } else if (currentGear == "reverse") {
    digitalWrite(PIN_MOTOR_DIR_A, LOW);
    digitalWrite(PIN_MOTOR_DIR_B, HIGH);
  } else { // neutral
    digitalWrite(PIN_MOTOR_DIR_A, LOW);
    digitalWrite(PIN_MOTOR_DIR_B, LOW);
  }
  Serial.printf("Gear: %s\n", currentGear.c_str());
  applyPower();
}

// ============ LECTURE CAPTEURS ============

void readSensors() {
  // Lecture tension (adaptez selon votre diviseur de tension)
  int rawVoltage = analogRead(PIN_VOLTAGE_SENSOR);
  // Exemple: diviseur 1:10, ADC 12 bits (0-4095), Vref 3.3V
  voltage = (rawVoltage / 4095.0) * 3.3 * 10.0; 
  
  // Lecture courant (adaptez selon votre capteur ACS712 ou autre)
  int rawCurrent = analogRead(PIN_CURRENT_SENSOR);
  // Exemple: ACS712-30A, sensibilité 66mV/A, offset 2.5V
  float currentVoltage = (rawCurrent / 4095.0) * 3.3;
  current = (currentVoltage - 1.65) / 0.066; // Ajustez selon votre capteur
  if (current < 0) current = 0;
  
  // Calcul vitesse (pulses par seconde -> noeuds)
  unsigned long now = millis();
  if (now - lastSpeedCalc >= 1000) {
    // Adaptez le facteur selon votre capteur
    // Exemple: 1 pulse = 0.1 mètre, donc pulses/s * 0.1 = m/s
    // 1 noeud = 0.5144 m/s
    float metersPerSecond = pulseCount * 0.1;
    speed = metersPerSecond / 0.5144;
    
    pulseCount = 0;
    lastSpeedCalc = now;
  }
  
  // Simulation si pas de capteurs connectés (pour test)
  if (voltage < 1) voltage = 48.0 - (actualPower / 100.0 * 2);
  if (current < 0.1 && motorOn) current = actualPower / 10.0;
  if (speed < 0.1 && motorOn && currentGear != "neutral") {
    speed = actualPower / 100.0 * 15.0;
  }
}

// ============ SETUP ============

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== Boat Control ESP32 ===\n");
  
  // Configuration pins moteur
  pinMode(PIN_MOTOR_ENABLE, OUTPUT);
  pinMode(PIN_MOTOR_DIR_A, OUTPUT);
  pinMode(PIN_MOTOR_DIR_B, OUTPUT);
  
  // Configuration PWM
  ledcSetup(1, 5000, 8); // Canal 1, 5kHz, 8 bits
  ledcAttachPin(PIN_MOTOR_PWM, 1);
  
  // Configuration capteur vitesse avec interruption
  pinMode(PIN_SPEED_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_SPEED_SENSOR), speedPulseISR, RISING);
  
  // État initial
  digitalWrite(PIN_MOTOR_ENABLE, LOW);
  digitalWrite(PIN_MOTOR_DIR_A, LOW);
  digitalWrite(PIN_MOTOR_DIR_B, LOW);
  ledcWrite(1, 0);
  
  // Configuration WiFi
  #if WIFI_MODE_AP
    // Mode Access Point
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.printf("AP Mode - SSID: %s\n", AP_SSID);
    Serial.printf("IP Address: %s\n", WiFi.softAPIP().toString().c_str());
  #else
    // Mode Station
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
  #endif
  
  // Configuration du serveur web
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/status", HTTP_OPTIONS, handleOptions);
  
  server.on("/telemetry", HTTP_GET, handleTelemetry);
  server.on("/telemetry", HTTP_OPTIONS, handleOptions);
  
  server.on("/command", HTTP_POST, handleCommand);
  server.on("/command", HTTP_OPTIONS, handleOptions);
  
  server.begin();
  Serial.println("HTTP server started!");
  Serial.println("\nEndpoints:");
  Serial.println("  GET  /status     - Check connection");
  Serial.println("  GET  /telemetry  - Get sensor data");
  Serial.println("  POST /command    - Send commands");
}

// ============ LOOP ============

void loop() {
  server.handleClient();
  readSensors();
  delay(10);
}
