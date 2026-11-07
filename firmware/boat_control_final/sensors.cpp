#include "sensors.h"
#include "config.h"
#include <Wire.h>
#include <bmm150.h>

// VITESSE 
static volatile int pulseCount = 0;
static unsigned long lastSpeedCalc = 0;
int currentRPM = 0;
float currentSpeed = 0.0;

static void IRAM_ATTR countPulse() {
  pulseCount++;
}

void initSpeedSensor() {
  pinMode(SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), countPulse, FALLING);
  lastSpeedCalc = millis();
}

void calculateSpeed() {
  unsigned long now = millis();
  if (now - lastSpeedCalc >= SPEED_PERIOD_MS) {
    currentRPM = (int)((pulseCount / PULSES_PER_ROTATION) * 60);
    currentSpeed = currentRPM * SPEED_CALIBRATION;

    Serial.printf("[SPEED] Pulses:%d RPM:%d Speed:%.2f\n", pulseCount, currentRPM, currentSpeed);

    pulseCount = 0;
    lastSpeedCalc = now;
  }
}

// COURANT
float currentAmperes = 0.0;
static unsigned long lastCurrentRead = 0;

// I = (V_mesurée × gain − V_offset) / sensibilité
static float readCurrent(float voltage, float offset) {
  float current = (voltage * CURRENT_GAIN - offset) / CURRENT_SENSITIVITY;
  if (abs(current) < CURRENT_DEADBAND) current = 0;
  return current;
}

void initCurrentSensor() {
  pinMode(PIN_MES, INPUT);
}

void updateCurrent() {
  unsigned long now = millis();
  if (now - lastCurrentRead >= CURRENT_PERIOD_MS) {
    int rawADC = analogRead(PIN_MES);
    float voltage = (rawADC * CURRENT_VREF) / CURRENT_ADC_MAX;
    currentAmperes = readCurrent(voltage, CURRENT_OFFSET);
    lastCurrentRead = now;
  }
}

// MAGNÉTOMÈTRE
static BMM150 bmm = BMM150();
int magX = 0, magY = 0, magZ = 0;
bool magInitialized = false;
float currentHeading = 0;

static float headingOffset = 0.0;              // fixé par calibrateNorth()
static float headingHistory[HEADING_SMOOTHING] = {0};
static int headingIndex = 0;
static unsigned long lastMagRead = 0;

static void scanI2C() {
  Serial.println("\n=== Scan I2C ===");
  int nDevices = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Device found at 0x%02X !\n", address);
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found");
  else Serial.printf("%d devices found\n", nDevices);
  Serial.println("================\n");
}

bool initMagnetometer() {
  Serial.println("\n=== Init Magnétomètre ===");
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);  // I2C rapide 400 kHz
  delay(100);
  scanI2C();

  Serial.println("Initialisation BMM150...");
  if (bmm.initialize() == BMM150_OK) {
    Serial.println("✓ BMM150 initialisé avec succès");
    bmm.read_mag_data();
    Serial.printf("Test lecture: X=%d Y=%d Z=%d\n",
                  bmm.raw_mag_data.raw_datax, bmm.raw_mag_data.raw_datay, bmm.raw_mag_data.raw_dataz);
    lastMagRead = millis();
    return true;
  }
  Serial.println("✗ ERREUR: Impossible d'initialiser BMM150");
  Serial.println("Vérifiez:");
  Serial.println("  - Connexions SDA(22) et SCL(21)");
  Serial.println("  - Alimentation du module");
  Serial.println("  - Adresse I2C (devrait être 0x10 ou 0x11)");
  return false;
}

static void readMagnetometer() {
  if (!magInitialized) return;
  bmm.read_mag_data();
  magX = bmm.raw_mag_data.raw_datax;
  magY = bmm.raw_mag_data.raw_datay;
  magZ = bmm.raw_mag_data.raw_dataz;
}

void updateMagnetometer() {
  unsigned long now = millis();
  if (magInitialized && (now - lastMagRead >= MAG_PERIOD_MS)) {
    readMagnetometer();
    lastMagRead = now;

    static unsigned long lastDebug = 0;  // trace toutes les 2 s
    if (now - lastDebug >= 2000) {
      Serial.printf("[MAG] X=%d Y=%d Z=%d Heading=%.1f°\n", magX, magY, magZ, calculateHeading());
      lastDebug = now;
    }
  }
}

// Cap = atan2(Y, X), ramené dans [0, 360[ puis corrigé de l'offset de calibration
float calculateHeading() {
  float heading = atan2((float)magY, (float)magX) * 180.0 / PI;
  if (heading < 0) heading += 360;
  heading += headingOffset;
  if (heading >= 360) heading -= 360;
  if (heading < 0) heading += 360;
  return heading;
}

float smoothHeading(float newHeading) {
  headingHistory[headingIndex] = newHeading;
  headingIndex = (headingIndex + 1) % HEADING_SMOOTHING;
  float sum = 0;
  for (int i = 0; i < HEADING_SMOOTHING; i++) sum += headingHistory[i];
  return sum / HEADING_SMOOTHING;
}

void calibrateNorth() {
  if (!magInitialized) return;
  float rawHeading = atan2((float)magY, (float)magX) * 180.0 / PI;
  if (rawHeading < 0) rawHeading += 360;
  headingOffset = -rawHeading;
  Serial.printf("Calibration: Offset = %.1f°\n", headingOffset);
}
