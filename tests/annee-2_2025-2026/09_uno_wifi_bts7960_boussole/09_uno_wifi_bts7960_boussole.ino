/*
 * 09_uno_wifi_bts7960_boussole
 * Projet Bateau autopiloté · Année 2 (2025-2026) · EEE & MIA
 *
 * Version « finale » prévue sur Arduino UNO WiFi Rev2 : pont en H BTS7960, servo, boussole QMC5883L,
 * capteur de vitesse IR (mesure de période) et capteur de courant ACS758 50 A (40 mV/A),
 * commandes Wi-Fi /avant /arriere /stop /gauche /droite /centre et télémétrie sur le port série.
 *
 * Source : dossier « EEE MIA » (code recopié du PDF, indentation restaurée).
 */

#include <WiFiNINA.h>
#include <Wire.h>
#include <Servo.h>
#include <QMC5883LCompass.h>
// ==================== Wi-Fi Configuration ====================
const char* ssid = "Bateau";
const char* password = "12345678";
WiFiServer server(80);
// ==================== Motor Control (BTS7960) ====================
#define RPWM 5
#define LPWM 6
#define R_EN 7
#define L_EN 8
// ==================== Sensors & Servo ====================
#define CURRENT_PIN A0
#define IR_SENSOR_PIN 2
#define SERVO_PIN 9
Servo direction;
QMC5883LCompass compass;
// ==================== Global Variables ====================
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
float rpm = 0;
float currentOffset = 2.5;  // 0A = ~2.5V (ACS758)
float sensitivity = 0.04;   // 40 mV/A for ACS758-50B
// ==================== Setup ====================
void controlMotor(int pwmValue, bool forward);
void handleCommand(String req);
void readTelemetry();
void countPulse();
float calculateRPM();

void setup() {
  Serial.begin(9600);
  Wire.begin();
  // H-Bridge setup
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  // Servo setup
  direction.attach(SERVO_PIN);
  direction.write(90);  // Center rudder
  // IR sensor for RPM
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), countPulse, FALLING);
  // Compass setup
  compass.init();
  compass.setCalibration(-635, 1243, -2943, 1184, -1115, 2087);
  // Wi-Fi connection
  Serial.print("Connecting to Wi-Fi...");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, password);
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("HTTP server started");
}
// ==================== Main Loop ====================
void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    handleCommand(request);
    // Send simple response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>Command received!</h1>");
    client.stop();
  }
  // Read and display telemetry
  readTelemetry();
  delay(500);
}
// ==================== Functions ====================
// Handle incoming Wi-Fi commands
void handleCommand(String req) {
  req.toLowerCase();
  if (req.indexOf("/avant") != -1) {
    controlMotor(180, true);
    Serial.println("FORWARD");
  } else if (req.indexOf("/arriere") != -1) {
    controlMotor(180, false);
    Serial.println("REVERSE");
  } else if (req.indexOf("/stop") != -1) {
    controlMotor(0, true);
    Serial.println("STOP");
  } else if (req.indexOf("/gauche") != -1) {
    direction.write(45);
    Serial.println("LEFT");
  } else if (req.indexOf("/droite") != -1) {
    direction.write(135);
    Serial.println("RIGHT");
  } else if (req.indexOf("/centre") != -1) {
    direction.write(90);
    Serial.println("CENTER");
  }
}
// Motor control via BTS7960
void controlMotor(int pwmValue, bool forward) {
  if (forward) {
    analogWrite(RPWM, pwmValue);
    analogWrite(LPWM, 0);
  } else {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, pwmValue);
  }
}
// Telemetry: Heading, RPM, Current
void readTelemetry() {
  compass.read();
  int heading = compass.getAzimuth();
  float sensorVoltage = analogRead(CURRENT_PIN) * (5.0 / 1023.0);
  float motorCurrent = (sensorVoltage - currentOffset) / sensitivity;
  rpm = calculateRPM();
  Serial.print("Heading: ");
  Serial.print(heading);
  Serial.print("° | RPM: ");
  Serial.print(rpm);
  Serial.print(" | Current: ");
  Serial.print(motorCurrent, 2);
  Serial.println(" A");
}
// IR speed sensor interrupt
void countPulse() {
  unsigned long now = micros();
  pulseInterval = now - lastPulseTime;
  lastPulseTime = now;
}
// RPM calculation
float calculateRPM() {
  if (pulseInterval > 0) {
    float freq = 1000000.0 / pulseInterval;
    return (freq * 60.0);  // 1 pulse per revolution
  } else {
    return 0;
  }
}
