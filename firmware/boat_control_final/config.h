// =====================================================================
//  config.h — Brochage et paramètres du bateau autopiloté (ESP32)
//  Projet Bateau autopiloté · ICAM · 2025-2026
// =====================================================================
#pragma once

// ----------------------------- Wi-Fi ---------------------------------
// true  : l'ESP32 crée son propre réseau (Access Point)  → http://192.168.4.1
// false : l'ESP32 se connecte au réseau défini dans secrets.h (mode Station),
//         puis repasse en Access Point si la connexion échoue.
#define USE_AP_MODE true

// ----------------------------- Bus I2C -------------------------------
#define I2C_SDA 22
#define I2C_SCL 21

// ------------------------ Servo de direction -------------------------
#define SERVO_PIN 18
#define SERVO_CENTER 90          // safran droit (°)
#define SERVO_MIN 30             // butée mécanique gauche (°)
#define SERVO_MAX 150            // butée mécanique droite (°)
#define SERVO_STEP 3             // pas maximal par mise à jour (°)
#define SERVO_UPDATE_MS 80       // période minimale entre deux mouvements (ms)

// --------------------- Pont en H BTS7960 (propulsion) ----------------
#define RPWM 25                  // marche avant
#define LPWM 26                  // marche arrière
#define PWM_FREQ 20000           // 20 kHz : hors du spectre audible
#define PWM_RESOLUTION 8         // rapport cyclique 0-255

// ------------------ Capteur de vitesse (LM393, fourche) --------------
#define SENSOR_PIN 27
#define PULSES_PER_ROTATION 1.0f // impulsions par tour (à ajuster)
#define SPEED_CALIBRATION 0.1f   // facteur tr/min → vitesse (à calibrer)
#define SPEED_PERIOD_MS 1000     // fenêtre de comptage (ms)

// ------------------- Capteur de courant (ACS758 50 A) ----------------
#define PIN_MES 34               // entrée ADC
#define CURRENT_VREF 3.3f        // tension de référence ADC ESP32
#define CURRENT_ADC_MAX 4095     // ADC 12 bits
#define CURRENT_SENSITIVITY 0.04f// 40 mV/A (ACS758LCB-050B)
#define CURRENT_OFFSET (2.5f * 0.4f) // tension à 0 A ramenée à l'entrée ADC (V)
#define CURRENT_GAIN 1.063f      // correction empirique mesurée au banc
#define CURRENT_DEADBAND 0.1f    // en dessous : 0 A
#define CURRENT_PERIOD_MS 500

// ----------------------- Magnétomètre BMM150 -------------------------
#define MAG_PERIOD_MS 50         // lecture à 20 Hz
#define HEADING_SMOOTHING 10     // moyenne glissante sur N mesures

// ----------------------------- Autopilote ----------------------------
#define KP 0.35f
#define KI 0.01f
#define KD 0.05f
#define PID_PERIOD_S 0.05f       // 20 Hz
#define HEADING_DEADBAND 8       // erreur de cap tolérée (°) : safran au centre
#define INTEGRAL_LIMIT 100       // anti-emballement du terme intégral
#define AUTO_SERVO_MIN 65        // débattement réduit en mode auto (°)
#define AUTO_SERVO_MAX 115
#define AUTO_SERVO_STABLE 4      // |angle-90| < 4 → 90
