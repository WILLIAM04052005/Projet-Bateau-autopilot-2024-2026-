
#pragma once

#define USE_AP_MODE true

#define I2C_SDA 22
#define I2C_SCL 21

// Servo de direction
#define SERVO_PIN 18
#define SERVO_CENTER 90       
#define SERVO_MIN 30            
#define SERVO_MAX 150          
#define SERVO_STEP 3            
#define SERVO_UPDATE_MS 80       

// Pont en H BTS7960 (propulsion)
#define RPWM 25               
#define LPWM 26                
#define PWM_FREQ 20000           
#define PWM_RESOLUTION 8         // rapport cyclique 0-255

//Capteur de vitesse (LM393, fourche)
#define SENSOR_PIN 27
#define PULSES_PER_ROTATION 1.0f 
#define SPEED_CALIBRATION 0.1f   
#define SPEED_PERIOD_MS 1000   
// Capteur de courant (ACS758 50 A) 
#define PIN_MES 34             
#define CURRENT_VREF 3.3f        
#define CURRENT_ADC_MAX 4095    
#define CURRENT_SENSITIVITY 0.04f
#define CURRENT_OFFSET (2.5f * 0.4f) 
#define CURRENT_GAIN 1.063f     
#define CURRENT_DEADBAND 0.1f    
#define CURRENT_PERIOD_MS 500

//  Magnétomètre BMM150
#define MAG_PERIOD_MS 50      
#define HEADING_SMOOTHING 10    

//  Autopilote
#define KP 0.35f
#define KI 0.01f
#define KD 0.05f
#define PID_PERIOD_S 0.05f      
#define HEADING_DEADBAND 8       
#define INTEGRAL_LIMIT 100       
#define AUTO_SERVO_MIN 65       
#define AUTO_SERVO_MAX 115
#define AUTO_SERVO_STABLE 4      
