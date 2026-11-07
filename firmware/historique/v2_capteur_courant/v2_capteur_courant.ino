#include <WiFi.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <bmm150.h>

// I2C PINS
#define I2C_SDA 22
#define I2C_SCL 21

// SERVO 
Servo myServo;
const int servoPin = 18;

// SPEED SENSOR
#define SENSOR_PIN 27  
volatile int pulseCount = 0;
unsigned long lastSpeedCalc = 0;
int currentRPM = 0;
float currentSpeed = 0.0; 

const float PULSES_PER_ROTATION = 1.0;  // Nombre d'impulsions par tour
const float SPEED_CALIBRATION = 0.1;    

// CAPTEUR DE COURANT 
#define PIN_MES 34              
const float VREF_CURRENT = 3.3; 
const int ADC_MAX = 4095;       
const float SENSITIVITY = 0.04;
const float CURRENT_OFFSET = 2.5 * 0.4; 

float currentAmpere = 0.0;    
unsigned long lastCurrentRead = 0;
const unsigned long currentReadInterval = 500;

// Fonction d'interruption pour compter les impulsions
void IRAM_ATTR countPulse() {
  pulseCount++;
}

// WIFI CONFIGURATION
#define USE_AP_MODE true

const char* STA_SSID = "VotreReseau";       
const char* STA_PASSWORD = "VotreMotDePasse"; 

const char* AP_SSID = "BoatControl";
const char* AP_PASSWORD = "12345678";

WiFiServer server(80);

// BTS7960 MOTEUR
#define RPWM 25
#define LPWM 26

#define PWM_FREQ 20000
#define PWM_RESOLUTION 8

String direction = "stop";
int speedVal = 0;
int heading = 180;
bool motorOn = false;

// VARIABLES AUTOPILOT
bool autopilotActive = false;
int targetHeading = 0;

// MAGNETOMETER 
BMM150 bmm = BMM150();
bmm150_mag_data magValue;

int magX = 0;
int magY = 0;
int magZ = 0;
bool magInitialized = false;

//  HEADING / AUTOPILOT
float currentHeading = 0;
float headingHistory[10] = {0, 0, 0, 0, 0};
int headingIndex = 0;

// PID Variables
float lastError = 0;
float integralError = 0;
unsigned long lastPIDTime = 0;

// Constantes PID
const float Kp = 0.35;
const float Ki = 0.01;
const float Kd = 0.05;

// Calibration
float headingOffset = 0.0;

// Timing pour lecture magnétomètre
unsigned long lastMagRead = 0;
const unsigned long magReadInterval = 50; // Lire toutes les 50ms (20Hz)

// CAPTEUR DE COURANT
float readCurrent(float voltage, float offset) {
  float current = (voltage * 1.063 - offset) / SENSITIVITY;
  if (abs(current) < 0.1) current = 0;
  return current;
}

void updateCurrent() {
  unsigned long currentTime = millis();
  if (currentTime - lastCurrentRead >= currentReadInterval) {
    int raw = analogRead(PIN_MES);
    float voltage = (raw * VREF_CURRENT) / ADC_MAX;
    currentAmpere = readCurrent(voltage, CURRENT_OFFSET);
    Serial.printf("[CURRENT] ADC:%d Voltage:%.3fV Current:%.3fA\n", raw, voltage, currentAmpere);
    lastCurrentRead = currentTime;
  }
}

// MOTOR APPLY
void applyMotor() {
  if (!motorOn) {
    ledcWrite(RPWM, 0);
    ledcWrite(LPWM, 0);
    return;
  }
  if (direction == "fwd") {
    ledcWrite(LPWM, 0);
    ledcWrite(RPWM, speedVal);
  }
  else if (direction == "rev") {
    ledcWrite(RPWM, 0);
    ledcWrite(LPWM, speedVal);
  }
  else {
    ledcWrite(RPWM, 0);
    ledcWrite(LPWM, 0);
  }
}

// SERVO CONTROL
void applyServo(int angle) {

  static unsigned long lastMove = 0;
  if (millis() - lastMove < 20) return;
  lastMove = millis();

  angle = constrain(angle, 0, 180);

  static int lastAngle = 90;
  lastAngle = angle;

  myServo.write(lastAngle);
}

// SPEED CALCULATION
void calculateSpeed() {
  unsigned long currentTime = millis();
  
  // Calcul toutes les 1 seconde 
  if (currentTime - lastSpeedCalc >= 1000) {
    // Calcul RPM
    currentRPM = (int)((pulseCount / PULSES_PER_ROTATION) * 60);
    
    // Calcul vitesse (à calibrer selon votre système)
    currentSpeed = currentRPM * SPEED_CALIBRATION;
    
    // Debug
    Serial.printf("[SPEED] Pulses:%d RPM:%d Speed:%.2f\n", 
                  pulseCount, currentRPM, currentSpeed);
    
    // Reset du compteur
    pulseCount = 0;
    lastSpeedCalc = currentTime;
  }
}

// I2C SCANNER
void scanI2C() {
  Serial.println("\n=== Scan I2C ===");
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }
  
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.printf("%d devices found\n", nDevices);
  
  Serial.println("================\n");
}

// MAGNETOMETER INIT
bool initMagnetometer() {
  Serial.println("\n=== Init Magnétomètre ===");
  
  // Initialiser I2C avec pins explicites
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400kHz
  delay(100);
  
  // Scanner I2C
  scanI2C();
  
  // Initialiser BMM150
  Serial.println("Initialisation BMM150...");
  
  if (bmm.initialize() == BMM150_OK) {
    Serial.println("✓ BMM150 initialisé avec succès");
    
    // Test de lecture
    bmm.read_mag_data();
    Serial.printf("Test lecture: X=%d Y=%d Z=%d\n", 
                  bmm.raw_mag_data.raw_datax,
                  bmm.raw_mag_data.raw_datay,
                  bmm.raw_mag_data.raw_dataz);
    
    return true;
  } else {
    Serial.println("✗ ERREUR: Impossible d'initialiser BMM150");
    Serial.println("Vérifiez:");
    Serial.println("  - Connexions SDA(22) et SCL(21)");
    Serial.println("  - Alimentation du module");
    Serial.println("  - Adresse I2C (devrait être 0x10 ou 0x11)");
    return false;
  }
}

// ================= READ MAGNETOMETER =================
void readMagnetometer() {
  if (!magInitialized) return;
  
  // Lire les données brutes
  bmm.read_mag_data();
  magX = bmm.raw_mag_data.raw_datax;
  magY = bmm.raw_mag_data.raw_datay;
  magZ = bmm.raw_mag_data.raw_dataz;
}

// ================= HEADING CALCULATION =================
float calculateHeading() {

  float x = magX;
  float y = magY;

  float heading = atan2(y, x) * 180.0 / PI;

  if (heading < 0)
    heading += 360;

  heading += headingOffset;

  if (heading >= 360) heading -= 360;
  if (heading < 0) heading += 360;

  return heading;
}

// ================= SMOOTH HEADING =================
float smoothHeading(float newHeading) {

  headingHistory[headingIndex] = newHeading;
  headingIndex = (headingIndex + 1) % 10;

  float sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += headingHistory[i];
  }

  return sum / 10.0;
}

// AUTOPILOT CONTROL
void autopilotControl() {

  if (!autopilotActive || !magInitialized) return;

  unsigned long now = millis();
  float dt = (now - lastPIDTime) / 1000.0;

  if (dt < 0.05) return;   // 20Hz PID
  lastPIDTime = now;

  // Read heading
  float rawHeading = calculateHeading();
  currentHeading = smoothHeading(rawHeading);

  // Calculate error
  float error = targetHeading - currentHeading;

  // Normalize error (-180 to 180)
  if (error > 180) error -= 360;
  if (error < -180) error += 360;

  // DEAD BAND (important for boats)
  if (abs(error) < 8) {
    applyServo(90);
    return;
  }

  // PID

  float P = Kp * error;

  integralError += error * dt;
  integralError = constrain(integralError, -100, 100);
  float I = Ki * integralError;

  float derivative = (error - lastError) / dt;
  float D = Kd * derivative;

  float pid = P + I + D;

  lastError = error;

  // Convert PID output to servo angle
  int servoAngle = 90 + pid;

  servoAngle = constrain(servoAngle, 65, 115);
  // zone stable
  if (abs(servoAngle - 90) < 4) {
    servoAngle = 90;
  }

  applyServo(servoAngle);

  // Debug
  Serial.printf(
  "AUTO | Target:%d° Current:%.1f° Error:%.1f Servo:%d\n",
  targetHeading,
  currentHeading,
  error,
  servoAngle
  );
}

// HTML PAGE
String buildPage() {
  String html = R"HTML(
<!DOCTYPE html><html lang="fr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Boat Control - Système Complet</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:#0a0e1a;color:#e2e8f0;overflow:hidden;touch-action:none}
.app{max-width:100vw;margin:0 auto;padding:12px;min-height:100vh}

@media (orientation: landscape) {
  .app{display:flex;flex-direction:column;max-height:100vh;padding:8px}
  .header{margin-bottom:8px}
  .controls{flex:1;grid-template-columns:1fr 2fr 1fr;gap:10px;align-items:stretch}
  .card{padding:10px}
  .telemetry{grid-template-columns:repeat(6,1fr);gap:6px;margin-top:8px}
}

.header{display:flex;align-items:center;justify-content:space-between;padding:10px 14px;background:linear-gradient(135deg,#1a1f35,#0f1525);border-radius:14px;margin-bottom:12px;border:1px solid #1e293b}
.header h1{font-size:16px;font-weight:700;background:linear-gradient(135deg,#60a5fa,#a78bfa);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.header-left{display:flex;flex-direction:column;gap:2px}
.status-dot{width:10px;height:10px;border-radius:50%;background:#ef4444;box-shadow:0 0 8px #ef4444}
.status-dot.on{background:#22c55e;box-shadow:0 0 8px #22c55e}
.mag-status{width:10px;height:10px;border-radius:50%;background:#ef4444;box-shadow:0 0 8px #ef4444;margin-left:8px}
.mag-status.ok{background:#22c55e;box-shadow:0 0 8px #22c55e}
.header-info{font-size:11px;color:#64748b}
.mode-toggle{display:flex;gap:4px;background:#0f172a;border-radius:10px;padding:3px;border:1px solid #334155}
.mode-btn{padding:6px 12px;border-radius:8px;font-size:11px;font-weight:600;cursor:pointer;transition:all .2s;border:none;background:transparent;color:#64748b}
.mode-btn.active{background:linear-gradient(135deg,#3b82f6,#2563eb);color:#fff;box-shadow:0 2px 8px rgba(59,130,246,0.3)}

.controls{display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;margin-bottom:12px}

.card{background:linear-gradient(135deg,#111827,#1a1f35);border-radius:14px;padding:14px;border:1px solid #1e293b;text-align:center}
.card-label{font-size:10px;text-transform:uppercase;letter-spacing:1px;color:#64748b;margin-bottom:8px}

.joystick-wrap{position:relative;width:180px;height:180px;margin:0 auto}
.joystick-bg{width:100%;height:100%;border-radius:50%;border:3px solid #334155;position:relative;background:radial-gradient(circle,#0f172a,#1e293b);box-shadow:inset 0 0 20px rgba(0,0,0,0.5);overflow:hidden}
.joystick-bg::after{content:'';position:absolute;bottom:0;left:0;right:0;height:50%;background:rgba(239,68,68,0.15);border-top:2px dashed #ef4444}
.joystick-knob{position:absolute;top:50%;left:50%;width:60px;height:60px;background:linear-gradient(135deg,#60a5fa,#3b82f6);border-radius:50%;transform:translate(-50%,-50%);cursor:grab;box-shadow:0 4px 15px rgba(96,165,250,0.4);border:2px solid #1e40af;transition:box-shadow 0.1s;z-index:10}
.joystick-knob:active{cursor:grabbing;box-shadow:0 6px 25px rgba(96,165,250,0.6)}
.joystick-center{position:absolute;top:50%;left:50%;width:12px;height:12px;background:#60a5fa;border-radius:50%;transform:translate(-50%,-50%);box-shadow:0 0 10px #60a5fa;pointer-events:none}
.joystick-val{font-size:24px;font-weight:700;color:#60a5fa;margin-top:8px;text-shadow:0 2px 10px rgba(96,165,250,0.3)}
.joystick-dir{font-size:14px;color:#94a3b8;margin-top:4px}
.servo-indicator{font-size:12px;color:#22c55e;margin-top:4px;font-weight:600}

.motor-btn{width:80px;height:80px;border-radius:50%;border:3px solid #334155;font-size:14px;font-weight:700;cursor:pointer;transition:all .3s;display:flex;align-items:center;justify-content:center;margin:0 auto}
.motor-btn.off{background:linear-gradient(135deg,#1e293b,#0f172a);color:#64748b}
.motor-btn.on{background:linear-gradient(135deg,#166534,#15803d);color:#22c55e;border-color:#22c55e;box-shadow:0 0 20px rgba(34,197,94,.3)}

.gear-wrap{display:flex;flex-direction:column;gap:4px;margin-top:10px}
.gear-btn{padding:8px;border:1px solid #334155;background:#1e293b;color:#94a3b8;border-radius:8px;font-size:12px;font-weight:600;cursor:pointer;transition:all .2s}
.gear-btn.active{border-color:#60a5fa;color:#60a5fa;background:#1e3a5f}
.gear-btn:disabled{opacity:.4;cursor:not-allowed}

.power-wrap{display:flex;flex-direction:column;align-items:center;gap:8px;height:100%}
.power-val{font-size:32px;font-weight:800;background:linear-gradient(135deg,#22c55e,#f59e0b,#ef4444);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.power-unit{font-size:11px;color:#64748b}
.power-gauge-container{position:relative;width:50px;height:240px;background:#1e293b;border-radius:25px;border:2px solid #334155;overflow:hidden;box-shadow:inset 0 0 15px rgba(0,0,0,0.5);cursor:pointer;user-select:none}
.power-gauge-fill{position:absolute;bottom:0;width:100%;background:linear-gradient(to top,#22c55e 0%,#84cc16 25%,#facc15 50%,#fb923c 75%,#ef4444 100%);transition:height 0.2s;border-radius:25px;pointer-events:none}

.telemetry{display:grid;grid-template-columns:repeat(6,1fr);gap:8px}
.telem-card{background:linear-gradient(135deg,#111827,#1a1f35);border-radius:12px;padding:10px 8px;text-align:center;border:1px solid #1e293b}
.telem-val{font-size:20px;font-weight:700;color:#60a5fa}
.telem-unit{font-size:11px;color:#64748b}
.telem-label{font-size:9px;text-transform:uppercase;letter-spacing:1px;color:#475569;margin-top:4px}
.telem-card.warn .telem-val{color:#f59e0b}
.telem-card.crit .telem-val{color:#ef4444}
.telem-card.batt .telem-val{color:#22c55e}
.telem-card.mag .telem-val{color:#22c55e}
.telem-card.speed .telem-val{color:#818cf8;font-size:24px}
.telem-card.current .telem-val{color:#f59e0b}

.page{display:none}
.page.active{display:block}
.autopilot-container{display:grid;grid-template-columns:1fr 2fr 1fr;gap:12px;align-items:start;height:calc(100vh - 140px)}
.autopilot-center{display:flex;flex-direction:column;align-items:center;justify-content:center;gap:20px}
.autopilot-icon{width:280px;height:280px;position:relative}
.autopilot-status{text-align:center;margin-top:16px}
.autopilot-status h2{font-size:24px;font-weight:700;color:#60a5fa;margin-bottom:8px}
.autopilot-status p{font-size:14px;color:#94a3b8}
.autopilot-heading-display{font-size:18px;color:#22c55e;margin-top:8px;font-weight:600}
.autopilot-controls{display:flex;gap:12px;margin-top:20px}
.autopilot-btn{padding:12px 24px;border-radius:12px;font-size:14px;font-weight:600;cursor:pointer;transition:all .3s;border:2px solid #334155;background:linear-gradient(135deg,#1e293b,#0f172a);color:#94a3b8}
.autopilot-btn:hover{border-color:#60a5fa;color:#60a5fa;box-shadow:0 0 15px rgba(96,165,250,0.3)}
.autopilot-btn.active{background:linear-gradient(135deg,#166534,#15803d);color:#22c55e;border-color:#22c55e;box-shadow:0 0 20px rgba(34,197,94,.3)}
.autopilot-side{display:flex;flex-direction:column;gap:8px}
.heading-input-card{background:linear-gradient(135deg,#111827,#1a1f35);border-radius:14px;padding:16px;border:1px solid #1e293b;text-align:center}
.heading-input-label{font-size:10px;text-transform:uppercase;letter-spacing:1px;color:#64748b;margin-bottom:12px}
.heading-input-display{font-size:42px;font-weight:800;color:#60a5fa;margin-bottom:12px}
.heading-adjust{display:flex;gap:8px;justify-content:center}
.heading-adjust button{width:50px;height:50px;border-radius:12px;border:1px solid #334155;background:#1e293b;color:#60a5fa;font-size:20px;font-weight:700;cursor:pointer;transition:all .2s}
.heading-adjust button:active{background:#334155;transform:scale(0.95)}
</style>
</head>
<body>
<div class="app">

<div class="header">
  <div class="header-left">
    <h1>⚓ Boat Control - Système Complet</h1>
    <div class="header-info"><span id="hInfo">Cap: -- | Vitesse: --</span></div>
  </div>
  <div style="display:flex;align-items:center;gap:12px">
    <div class="mode-toggle">
      <button class="mode-btn active" id="btnManual" onclick="switchMode('manual')">Manuel</button>
      <button class="mode-btn" id="btnAutopilot" onclick="switchMode('autopilot')">Auto-Pilote</button>
    </div>
    <div style="display:flex;align-items:center;gap:4px">
      <div class="status-dot" id="statusDot"></div>
      <div class="mag-status" id="magStatus"></div>
    </div>
  </div>
</div>

<div class="page active" id="pageManual">

<div class="controls">
  <div class="card">
    <div class="card-label">Direction (Servo)</div>
    <div class="joystick-wrap" id="joystickWrap">
      <div class="joystick-bg">
        <div class="joystick-center"></div>
        <div class="joystick-knob" id="joystickKnob"></div>
      </div>
    </div>
    <div class="joystick-val" id="headVal">--°</div>
    <div class="joystick-dir" id="headDir">--</div>
    <div class="servo-indicator" id="servoAngle">Servo: 90°</div>
  </div>

  <div class="card">
    <div class="card-label">Moteur</div>
    <button class="motor-btn off" id="motorBtn" onclick="toggleMotor()">OFF</button>
    <div class="gear-wrap">
      <button class="gear-btn" id="gFwd" onclick="setDir('fwd')" disabled>▲ Avant</button>
      <button class="gear-btn active" id="gN" onclick="setDir('stop')" disabled>■ Stop</button>
      <button class="gear-btn" id="gRev" onclick="setDir('rev')" disabled>▼ Arrière</button>
    </div>
  </div>

  <div class="card">
    <div class="card-label">Puissance Moteur</div>
    <div class="power-wrap">
      <div class="power-val" id="pwrVal">0</div>
      <div class="power-unit">/ 255</div>
      <div class="power-gauge-container" id="powerGauge">
        <div class="power-gauge-fill" id="powerFill" style="height:0%"></div>
      </div>
    </div>
  </div>
</div>

<div class="telemetry">
  <div class="telem-card speed">
    <div class="telem-val" id="speedVal">0.0</div>
    <div class="telem-unit">km/h</div>
    <div class="telem-label">Vitesse</div>
  </div>
  <div class="telem-card">
    <div class="telem-val" id="rpmVal">0</div>
    <div class="telem-unit">RPM</div>
    <div class="telem-label">Rotation</div>
  </div>
  <div class="telem-card" id="currentCard">
    <div class="telem-val" id="currentVal">0.0</div>
    <div class="telem-unit">A</div>
    <div class="telem-label">Courant</div>
  </div>
  <div class="telem-card mag">
    <div class="telem-val" id="currentHeading">--</div>
    <div class="telem-unit">°</div>
    <div class="telem-label">Cap Réel</div>
  </div>
  <div class="telem-card">
    <div class="telem-val" id="magX">--</div>
    <div class="telem-unit">raw</div>
    <div class="telem-label">MAG X</div>
  </div>
  <div class="telem-card">
    <div class="telem-val" id="magY">--</div>
    <div class="telem-unit">raw</div>
    <div class="telem-label">MAG Y</div>
  </div>
</div>

</div>

<div class="page" id="pageAutopilot">
<div class="autopilot-container">
  
  <div class="autopilot-side">
    <div class="telem-card speed">
      <div class="telem-val" id="speedValAuto">0.0</div>
      <div class="telem-unit">km/h</div>
      <div class="telem-label">Vitesse</div>
    </div>
    <div class="telem-card">
      <div class="telem-val" id="rpmValAuto">0</div>
      <div class="telem-unit">RPM</div>
      <div class="telem-label">Rotation</div>
    </div>
    <div class="telem-card" id="currentCardAuto">
      <div class="telem-val" id="currentValAuto">0.0</div>
      <div class="telem-unit">A</div>
      <div class="telem-label">Courant</div>
    </div>
    <div class="telem-card mag">
      <div class="telem-val" id="currentHeadingAuto">--</div>
      <div class="telem-unit">°</div>
      <div class="telem-label">Cap Réel</div>
    </div>
    <div class="telem-card">
      <div class="telem-val" id="magXAuto">--</div>
      <div class="telem-unit">raw</div>
      <div class="telem-label">MAG X</div>
    </div>
    <div class="telem-card">
      <div class="telem-val" id="magYAuto">--</div>
      <div class="telem-unit">raw</div>
      <div class="telem-label">MAG Y</div>
    </div>
  </div>

  <div class="autopilot-center">
    <svg class="autopilot-icon" viewBox="0 0 300 300" xmlns="http://www.w3.org/2000/svg">
      <circle cx="150" cy="150" r="140" fill="none" stroke="#1e293b" stroke-width="2"/>
      <circle cx="150" cy="150" r="110" fill="none" stroke="#1e293b" stroke-width="1" stroke-dasharray="4 4"/>
      <circle cx="150" cy="150" r="80" fill="none" stroke="#1e293b" stroke-width="1" stroke-dasharray="4 4"/>
      <circle cx="150" cy="150" r="50" fill="none" stroke="#1e293b" stroke-width="1" stroke-dasharray="4 4"/>
      <line x1="150" y1="10" x2="150" y2="290" stroke="#334155" stroke-width="1" opacity="0.3"/>
      <line x1="10" y1="150" x2="290" y2="150" stroke="#334155" stroke-width="1" opacity="0.3"/>
      <line id="trajectoryLine" x1="150" y1="150" x2="150" y2="30" stroke="#60a5fa" stroke-width="3" stroke-dasharray="8 4" opacity="0.8"/>
      <g id="compassRose">
        <text x="150" y="25" text-anchor="middle" fill="#60a5fa" font-size="14" font-weight="700">N</text>
        <text x="275" y="155" text-anchor="middle" fill="#64748b" font-size="12">E</text>
        <text x="150" y="285" text-anchor="middle" fill="#64748b" font-size="12">S</text>
        <text x="25" y="155" text-anchor="middle" fill="#64748b" font-size="12">W</text>
      </g>
      <g id="boatIcon" transform="translate(150, 150)">
        <path d="M -15,-25 L -8,-20 L -8,20 L -12,25 L 12,25 L 8,20 L 8,-20 L 15,-25 Z" 
              fill="url(#boatGradient)" stroke="#60a5fa" stroke-width="2"/>
        <rect x="-6" y="-15" width="12" height="8" fill="#1e3a8a" opacity="0.6" rx="2"/>
        <rect x="-4" y="5" width="8" height="12" fill="#1e3a8a" opacity="0.4" rx="1"/>
        <path d="M 0,-28 L -4,-22 L 4,-22 Z" fill="#22c55e" id="boatDirection"/>
      </g>
      <circle cx="150" cy="150" r="3" fill="#60a5fa"/>
      <circle cx="150" cy="150" r="8" fill="none" stroke="#60a5fa" stroke-width="1"/>
      <defs>
        <linearGradient id="boatGradient" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" style="stop-color:#3b82f6;stop-opacity:1" />
          <stop offset="100%" style="stop-color:#1e40af;stop-opacity:1" />
        </linearGradient>
      </defs>
    </svg>

    <div class="autopilot-status">
      <h2 id="autopilotStatusText">Pilote Automatique Désactivé</h2>
      <p id="autopilotCourse">Cap cible: <span id="autopilotHeading">0°</span></p>
      <p class="autopilot-heading-display">Cap actuel: <span id="autopilotCurrentHeading">--°</span> | Erreur: <span id="autopilotError">--°</span></p>
    </div>

    <div class="autopilot-controls">
      <button class="autopilot-btn" id="autopilotToggle" onclick="toggleAutopilot()">
        Activer Auto-Pilote
      </button>
    </div>

  </div>

  <div class="autopilot-side">
    <div class="heading-input-card">
      <div class="heading-input-label">Cap Cible</div>
      <div class="heading-input-display" id="targetHeadingDisplay">0°</div>
      <div class="heading-adjust">
        <button onclick="adjustTargetHeading(-10)">-10</button>
        <button onclick="adjustTargetHeading(-1)">-1</button>
        <button onclick="adjustTargetHeading(1)">+1</button>
        <button onclick="adjustTargetHeading(10)">+10</button>
      </div>
    </div>
    
    <div class="card">
      <div class="card-label">Caps Rapides</div>
      <div style="display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-top:8px">
        <button onclick="setTargetHeading(0)" style="padding:8px;border-radius:8px;border:1px solid #334155;background:#1e293b;color:#60a5fa;font-size:12px;cursor:pointer">N (0°)</button>
        <button onclick="setTargetHeading(90)" style="padding:8px;border-radius:8px;border:1px solid #334155;background:#1e293b;color:#60a5fa;font-size:12px;cursor:pointer">E (90°)</button>
        <button onclick="setTargetHeading(180)" style="padding:8px;border-radius:8px;border:1px solid #334155;background:#1e293b;color:#60a5fa;font-size:12px;cursor:pointer">S (180°)</button>
        <button onclick="setTargetHeading(270)" style="padding:8px;border-radius:8px;border:1px solid #334155;background:#1e293b;color:#60a5fa;font-size:12px;cursor:pointer">W (270°)</button>
      </div>
    </div>
    
    <div class="card">
      <div class="card-label">Calibration Magnétomètre</div>
      <div style="display:flex;flex-direction:column;gap:4px;margin-top:8px">
        <button onclick="calibrateCompass()" style="padding:10px;border-radius:8px;border:1px solid #334155;background:#1e293b;color:#22c55e;font-size:12px;cursor:pointer;font-weight:600">🧭 Définir Nord Actuel</button>
        <p style="font-size:10px;color:#64748b;text-align:center;margin-top:4px">Pointez le bateau vers le Nord et cliquez</p>
      </div>
    </div>
  </div>

</div>
</div>

</div>

<script>
var mot = false;
var curDir = 'stop';
var curHead = 0;
var curServoAngle = 90;
var curSpd = 0;
var currentMode = 'manual';
var autopilotActive = false;
var targetHeading = 0;
var realCurrentHeading = 0;
var magOK = false;

function headingToServoAngle(heading) {
  var servoAngle;
  
  if (heading >= 270) {
    servoAngle = heading - 270;
  } else if (heading <= 90) {
    servoAngle = heading + 90;
  } else {
    servoAngle = 90;
  }
  
  return Math.max(0, Math.min(180, Math.round(servoAngle)));
}

function switchMode(mode) {
  currentMode = mode;
  
  document.getElementById('pageManual').className = mode === 'manual' ? 'page active' : 'page';
  document.getElementById('pageAutopilot').className = mode === 'autopilot' ? 'page active' : 'page';
  document.getElementById('btnManual').className = mode === 'manual' ? 'mode-btn active' : 'mode-btn';
  document.getElementById('btnAutopilot').className = mode === 'autopilot' ? 'mode-btn active' : 'mode-btn';
}

function calibrateCompass() {
  if (confirm('Le bateau est-il pointé vers le Nord géographique ?')) {
    fetch('/calibrate')
      .then(response => response.text())
      .then(data => {
        alert('Calibration effectuée ! Le cap actuel est défini comme Nord (0°)');
      })
      .catch(err => {
        alert('Erreur de calibration');
        console.error(err);
      });
  }
}

function toggleAutopilot() {
  if (!magOK) {
    alert('⚠️ Magnétomètre non disponible! Vérifiez les connexions I2C (SDA=22, SCL=21)');
    return;
  }
  
  autopilotActive = !autopilotActive;
  var btn = document.getElementById('autopilotToggle');
  var statusText = document.getElementById('autopilotStatusText');
  
  if (autopilotActive) {
    btn.innerText = 'Désactiver Auto-Pilote';
    btn.className = 'autopilot-btn active';
    statusText.innerText = 'Pilote Automatique Actif';
    statusText.style.color = '#22c55e';
    
    if (!mot) {
      mot = true;
      document.getElementById('statusDot').className = 'status-dot on';
      fetch('/motor?on=1');
    }
    
    fetch('/autopilot?active=1&heading=' + targetHeading);
  } else {
    btn.innerText = 'Activer Auto-Pilote';
    btn.className = 'autopilot-btn';
    statusText.innerText = 'Pilote Automatique Désactivé';
    statusText.style.color = '#60a5fa';
    
    fetch('/autopilot?active=0');
  }
  
  updateAutopilotVisualization();
}

function setTargetHeading(heading) {
  targetHeading = (heading + 360) % 360;
  
  document.getElementById('targetHeadingDisplay').innerText = targetHeading + '°';
  document.getElementById('autopilotHeading').innerText = targetHeading + '°';
  
  updateAutopilotVisualization();
  
  if (autopilotActive) {
    fetch('/autopilot?active=1&heading=' + targetHeading);
  }
}

function adjustTargetHeading(delta) {
  targetHeading = (targetHeading + delta + 360) % 360;
  setTargetHeading(targetHeading);
}

function updateAutopilotVisualization() {
  var targetAngle = targetHeading;
  var targetRad = (targetAngle - 90) * Math.PI / 180;
  var x2 = 150 + Math.cos(targetRad) * 120;
  var y2 = 150 + Math.sin(targetRad) * 120;
  
  document.getElementById('trajectoryLine').setAttribute('x2', x2);
  document.getElementById('trajectoryLine').setAttribute('y2', y2);
  
  var currentAngle = realCurrentHeading;
  document.getElementById('boatIcon').setAttribute('transform', 
    'translate(150, 150) rotate(' + currentAngle + ')');
  
  var color = autopilotActive ? '#22c55e' : '#60a5fa';
  document.getElementById('boatDirection').setAttribute('fill', color);
  document.getElementById('trajectoryLine').setAttribute('stroke', color);
  
  if (autopilotActive) {
    var error = targetHeading - realCurrentHeading;
    while (error > 180) error -= 360;
    while (error < -180) error += 360;
    document.getElementById('autopilotError').innerText = Math.round(error) + '°';
  }
}

function toggleMotor() {
  mot = !mot;
  var b = document.getElementById('motorBtn');
  b.className = 'motor-btn ' + (mot ? 'on' : 'off');
  b.innerText = mot ? 'ON' : 'OFF';
  document.getElementById('statusDot').className = 'status-dot' + (mot ? ' on' : '');
  document.getElementById('gFwd').disabled = !mot;
  document.getElementById('gN').disabled = !mot;
  document.getElementById('gRev').disabled = !mot;
  fetch('/motor?on=' + (mot ? '1' : '0'));
  if (!mot) { setDir('stop'); setSpeed(0); }
}

function setDir(d) {
  curDir = d;
  fetch('/dir?d=' + d);
  document.getElementById('gFwd').className = 'gear-btn' + (d=='fwd'?' active':'');
  document.getElementById('gN').className = 'gear-btn' + (d=='stop'?' active':'');
  document.getElementById('gRev').className = 'gear-btn' + (d=='rev'?' active':'');
}

var powerGauge = document.getElementById('powerGauge');
var isPowerDragging = false;

function updatePowerFromEvent(e) {
  var rect = powerGauge.getBoundingClientRect();
  var clientY;
  if (e.type.indexOf('touch') >= 0) {
    clientY = e.touches[0].clientY;
  } else {
    clientY = e.clientY;
  }
  
  var y = clientY - rect.top;
  var pct = 1 - (y / rect.height);
  pct = Math.max(0, Math.min(1, pct));
  var val = Math.round(pct * 255);
  setSpeed(val);
}

powerGauge.addEventListener('mousedown', function(e) {
  isPowerDragging = true;
  updatePowerFromEvent(e);
});

powerGauge.addEventListener('touchstart', function(e) {
  isPowerDragging = true;
  updatePowerFromEvent(e);
});

document.addEventListener('mousemove', function(e) {
  if (isPowerDragging) {
    e.preventDefault();
    updatePowerFromEvent(e);
  }
});

document.addEventListener('touchmove', function(e) {
  if (isPowerDragging) {
    e.preventDefault();
    updatePowerFromEvent(e);
  }
}, {passive: false});

document.addEventListener('mouseup', function() {
  isPowerDragging = false;
});

document.addEventListener('touchend', function() {
  isPowerDragging = false;
});

var st = null;
function setSpeed(v) {
  curSpd = parseInt(v);
  document.getElementById('pwrVal').innerText = v;
  var pct = Math.round(v / 255 * 100);
  document.getElementById('powerFill').style.height = pct + '%';
  
  clearTimeout(st);
  st = setTimeout(function(){ fetch('/speed?val=' + v); }, 80);
}

var joystickWrap = document.getElementById('joystickWrap');
var joystickKnob = document.getElementById('joystickKnob');
var isDragging = false;
var joystickCenterX = 0;
var joystickCenterY = 0;
var maxRadius = 60;

function updateJoystickCenter() {
  var rect = joystickWrap.getBoundingClientRect();
  joystickCenterX = rect.left + rect.width / 2;
  joystickCenterY = rect.top + rect.height / 2;
}

var lastServoSend = 0;
var servoDelay = 40;

function sendServoCommand(angle){
  var now = Date.now();
  
  if(now - lastServoSend > servoDelay){
    fetch('/servo?angle=' + angle);
    lastServoSend = now;
  }
}

function handleJoystickStart(e) {
  isDragging = true;
  updateJoystickCenter();
  handleJoystickMove(e);
}

function handleJoystickMove(e) {
  if (!isDragging) return;
  e.preventDefault();
  
  var clientX, clientY;
  if (e.type.indexOf('touch') >= 0) {
    clientX = e.touches[0].clientX;
    clientY = e.touches[0].clientY;
  } else {
    clientX = e.clientX;
    clientY = e.clientY;
  }
  
  var dx = clientX - joystickCenterX;
  var dy = clientY - joystickCenterY;
  var distance = Math.sqrt(dx*dx + dy*dy);
  
  if (distance > maxRadius) {
    var angle = Math.atan2(dy, dx);
    dx = Math.cos(angle) * maxRadius;
    dy = Math.sin(angle) * maxRadius;
  }
  
  var angleRad = Math.atan2(dx, -dy);
  var angleDeg = (angleRad * 180 / Math.PI + 360) % 360;
  
  if (angleDeg > 90 && angleDeg < 270) {
    angleDeg = (angleDeg < 180) ? 90 : 270;
    var limitedAngleRad = (angleDeg - 90) * Math.PI / 180;
    dx = Math.cos(limitedAngleRad) * Math.min(distance, maxRadius);
    dy = Math.sin(limitedAngleRad) * Math.min(distance, maxRadius);
  }
  
  joystickKnob.style.transform = 'translate(calc(-50% + ' + dx + 'px), calc(-50% + ' + dy + 'px))';
  
  curHead = Math.round(angleDeg);
  curServoAngle = headingToServoAngle(curHead);
  updateCompass();
  
  if(!autopilotActive){
    sendServoCommand(curServoAngle);
  }
}

function handleJoystickEnd(e) {
  isDragging = false;
  joystickKnob.style.transform = 'translate(-50%, -50%)';
}

joystickKnob.addEventListener('mousedown', handleJoystickStart);
joystickKnob.addEventListener('touchstart', handleJoystickStart);
document.addEventListener('mousemove', handleJoystickMove);
document.addEventListener('touchmove', handleJoystickMove, {passive: false});
document.addEventListener('mouseup', handleJoystickEnd);
document.addEventListener('touchend', handleJoystickEnd);

function updateCompass(){
  document.getElementById("headVal").innerText = curHead + "°";
  
  var dir = "N";
  if(curHead >= 45 && curHead < 135) dir = "E";
  else if(curHead >= 135 && curHead < 225) dir = "S";
  else if(curHead >= 225 && curHead < 315) dir = "W";
  
  document.getElementById("headDir").innerText = dir;
  document.getElementById("servoAngle").innerText = "Servo: " + curServoAngle + "°";
}

function updateMagnetometer() {
  fetch('/mag')
    .then(response => response.json())
    .then(data => {
      magOK = data.ok;
      
      // Mettre à jour le statut du magnétomètre
      document.getElementById('magStatus').className = magOK ? 'mag-status ok' : 'mag-status';
      
      if (magOK) {
        document.getElementById("magX").innerText = data.x;
        document.getElementById("magY").innerText = data.y;

        realCurrentHeading = parseFloat(data.heading);
        
        document.getElementById("currentHeading").innerText = Math.round(realCurrentHeading);
        
        if (currentMode === 'autopilot') {
          document.getElementById("currentHeadingAuto").innerText = Math.round(realCurrentHeading);
          document.getElementById("autopilotCurrentHeading").innerText = Math.round(realCurrentHeading);
          document.getElementById("magXAuto").innerText = data.x;
          document.getElementById("magYAuto").innerText = data.y;
          updateAutopilotVisualization();
        }
      } else {
        document.getElementById("currentHeading").innerText = "ERR";
        if (currentMode === 'autopilot') {
          document.getElementById("currentHeadingAuto").innerText = "ERR";
        }
      }
    })
    .catch(err => {
      console.log('Mag error:', err);
      magOK = false;
      document.getElementById('magStatus').className = 'mag-status';
    });
}

function updateSpeedData() {
  fetch('/speed_data')
    .then(response => response.json())
    .then(data => {
      var speed = parseFloat(data.speed).toFixed(1);
      var rpm = parseInt(data.rpm);
      
      document.getElementById("speedVal").innerText = speed;
      document.getElementById("rpmVal").innerText = rpm;
      
      if (currentMode === 'autopilot') {
        document.getElementById("speedValAuto").innerText = speed;
        document.getElementById("rpmValAuto").innerText = rpm;
      }
      
      // Mise à jour de l'info header
      var headingText = magOK ? Math.round(realCurrentHeading) + "°" : "--";
      document.getElementById("hInfo").innerText = "Cap: " + headingText + " | Vitesse: " + speed + " km/h";
    })
    .catch(err => {
      console.log('Speed data error:', err);
    });
}

updateJoystickCenter();
updateCompass();
window.addEventListener('resize', updateJoystickCenter);
window.addEventListener('orientationchange', function() {
  setTimeout(updateJoystickCenter, 100);
});

function updateCurrentData() {
  fetch('/current_data')
    .then(response => response.json())
    .then(data => {
      var current = parseFloat(data.current).toFixed(2);
      
      document.getElementById("currentVal").innerText = current;
      if (currentMode === 'autopilot') {
        document.getElementById("currentValAuto").innerText = current;
      }
      
      // Coloration selon le niveau de courant
      var cardClass = 'telem-card';
      if (Math.abs(parseFloat(current)) > 4.0) cardClass += ' crit';
      else if (Math.abs(parseFloat(current)) > 2.0) cardClass += ' warn';
      else cardClass += ' current';
      
      document.getElementById("currentCard").className = cardClass;
      if (currentMode === 'autopilot') {
        document.getElementById("currentCardAuto").className = cardClass;
      }
    })
    .catch(err => {
      console.log('Current data error:', err);
    });
}

setInterval(updateMagnetometer, 200);
setInterval(updateSpeedData, 500);
setInterval(updateCurrentData, 500);
</script>
</body></html>
)HTML";
  return html;
}

// HTTP REQUEST HANDLER
void handleClient(WiFiClient client) {
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');

  if (req.indexOf("GET /servo") >= 0) {
    int idx = req.indexOf("angle=");
    if (idx >= 0) {
      String v = req.substring(idx + 6);
      v = v.substring(0, v.indexOf(' '));
      int angle = v.toInt();
      applyServo(angle);
    }
    client.println("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nOK");
  }
  
  else if (req.indexOf("GET /dir") >= 0) {
    int idx = req.indexOf("d=");
    if (idx >= 0) {
      String d = req.substring(idx + 2);
      d = d.substring(0, d.indexOf(' '));
      direction = d;
      applyMotor();
    }
    client.println("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nOK");
  }
  
  else if (req.indexOf("GET /speed?") >= 0) {
    int idx = req.indexOf("val=");
    if (idx >= 0) {
      String v = req.substring(idx + 4);
      v = v.substring(0, v.indexOf(' '));
      speedVal = v.toInt();
      applyMotor();
    }
    client.println("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nOK");
  }
  
  else if (req.indexOf("GET /speed_data") >= 0) {
    String json = "{";
    json += "\"rpm\":" + String(currentRPM) + ",";
    json += "\"speed\":" + String(currentSpeed, 1);
    json += "}";
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
  }
  
  else if (req.indexOf("GET /current_data") >= 0) {
    String json = "{";
    json += "\"current\":" + String(currentAmpere, 3);
    json += "}";
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
  }
  
  else if (req.indexOf("GET /motor") >= 0) {
    int idx = req.indexOf("on=");
    if (idx >= 0) {
      String v = req.substring(idx + 3);
      v = v.substring(0, v.indexOf(' '));
      motorOn = (v == "1");
      applyMotor();
    }
    client.println("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nOK");
  }
  
  else if (req.indexOf("GET /autopilot") >= 0) {
    int idx = req.indexOf("active=");
    if (idx >= 0) {
      String v = req.substring(idx + 7);
      v = v.substring(0, v.indexOf('&') >= 0 ? v.indexOf('&') : v.indexOf(' '));
      autopilotActive = (v == "1");
      
      if (autopilotActive) {
        lastError = 0;
        integralError = 0;
        lastPIDTime = millis();
      }
    }
    
    idx = req.indexOf("heading=");
    if (idx >= 0) {
      String v = req.substring(idx + 8);
      v = v.substring(0, v.indexOf(' '));
      targetHeading = v.toInt();
      
      Serial.printf("Autopilot: Active=%d Target=%d°\n", autopilotActive, targetHeading);
    }
    
    client.println("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nOK");
  }
  
  else if (req.indexOf("GET /mag") >= 0) {
    String json = "{";
    json += "\"ok\":" + String(magInitialized ? "true" : "false") + ",";
    json += "\"x\":" + String(magX) + ",";
    json += "\"y\":" + String(magY) + ",";
    json += "\"z\":" + String(magZ) + ",";
    
    if (magInitialized) {
      currentHeading = calculateHeading();
      currentHeading = smoothHeading(currentHeading);
      json += "\"heading\":" + String(currentHeading, 1);
    } else {
      json += "\"heading\":0.0";
    }
    
    json += "}";
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
  }
  
  else if (req.indexOf("GET /calibrate") >= 0) {
    if (magInitialized) {
      float rawHeading = atan2(magY, magX) * 180.0 / PI;
      if (rawHeading < 0) rawHeading += 360;
      
      headingOffset = -rawHeading;
      
      Serial.printf("Calibration: Offset = %.1f°\n", headingOffset);
    }
    
    client.println("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nOK");
  }
  
  else if (req.indexOf("GET / ") >= 0 || req.indexOf("GET /index") >= 0) {
    String page = buildPage();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println("Connection: close");
    client.println();
    client.print(page);
  }
  
  else {
    client.println("HTTP/1.1 404 Not Found\r\nConnection:close\r\n\r\n404");
  }
  
  client.stop();
}

// SETUP 
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("    Boat Control - Système Complet");
  Serial.println("========================================\n");
  
  // ===== SERVO INIT =====
  Serial.println(">>> Initialisation Servo...");
  myServo.attach(servoPin);
  myServo.write(90);
  Serial.println("    ✓ Servo initialisé à 90°\n");
  
  // ===== MOTOR INIT =====
  Serial.println(">>> Initialisation Moteur BTS7960...");
  ledcAttach(25, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(26, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(25, speedVal);
  ledcWrite(26, speedVal);
  ledcWrite(RPWM, 0);
  ledcWrite(LPWM, 0);
  Serial.println("    ✓ Moteur initialisé\n");
  
  // ===== SPEED SENSOR INIT =====
  Serial.println(">>> Initialisation Capteur de Vitesse...");
  pinMode(SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), countPulse, FALLING);
  Serial.println("    ✓ Capteur de vitesse initialisé (Pin 27)\n");
  
  // ===== CURRENT SENSOR INIT =====
  Serial.println(">>> Initialisation Capteur de Courant...");
  pinMode(PIN_MES, INPUT);
  Serial.println("    ✓ Capteur de courant initialisé (Pin 34)\n");
  
  // ===== MAGNETOMETER INIT =====
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
  
  // Initialiser l'historique
  for (int i = 0; i < 10; i++) {
    headingHistory[i] = 0;
  }
  
  // ===== WIFI SETUP =====
  Serial.println(">>> Configuration WiFi...");
  
  if (USE_AP_MODE) {
    Serial.println("    Mode: Access Point");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.printf("    SSID: %s\n", AP_SSID);
    Serial.printf("    IP: %s\n", WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("    Mode: Station");
    WiFi.mode(WIFI_STA);
    WiFi.begin(STA_SSID, STA_PASSWORD);
    Serial.print("    Connexion");
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(500);
      Serial.print(".");
      timeout++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(" OK");
      Serial.printf("    IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println(" ÉCHEC");
      Serial.println("    Passage en mode AP...");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(AP_SSID, AP_PASSWORD);
      Serial.printf("    IP: %s\n", WiFi.softAPIP().toString().c_str());
    }
  }
  
  server.begin();
  Serial.println("    ✓ Serveur HTTP démarré sur port 80\n");
  
  Serial.println("========================================");
  Serial.println("         SYSTÈME PRÊT");
  Serial.println("========================================\n");
  
  lastPIDTime = millis();
  lastMagRead = millis();
  lastSpeedCalc = millis();
  lastCurrentRead = millis();
}

// LOOP 
void loop() {
  unsigned long currentTime = millis();
  
  // Calculer la vitesse
  calculateSpeed();
  
  // Lire le courant
  updateCurrent();
  
  // Lire le magnétomètre toutes les 50ms (20Hz)
  if (magInitialized && (currentTime - lastMagRead >= magReadInterval)) {
    readMagnetometer();
    lastMagRead = currentTime;
    
    // Debug périodique (toutes les 2 secondes)
    static unsigned long lastDebug = 0;
    if (currentTime - lastDebug >= 2000) {
      float h = calculateHeading();
      Serial.printf("[MAG] X=%d Y=%d Z=%d Heading=%.1f°\n", magX, magY, magZ, h);
      lastDebug = currentTime;
    }
  }
  
  // Contrôle autopilote (si actif)
  if (autopilotActive && magInitialized) {
    autopilotControl();
  }
  
  // Gérer les clients HTTP
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }
  
  delay(5);
}
