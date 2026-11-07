//-----Compass-----//

#include <Wire.h>
#include <QMC5883LCompass.h>

QMC5883LCompass compass;


//-----Servo Motor-----//

#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int directionPin = 1;  // analog pin used to connect the potentiometer
int servoPin = 3;


//-----Command Computing-----//

int val;

int command;

//-----DC Motor-----//

const int in1 = 5;
const int in2 = 6;
const int button = 4;

int speed;
bool motorSense = 0;
bool pressed = 0;


//-----LCD Display-----//
/*
#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x27 or 0x3f for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3f, 16, 2);
*/
#include <LiquidCrystal_AIP31068_I2C.h>
// Set the LCD address to 0x3f to differentiate from the real display
LiquidCrystal_AIP31068_I2C lcd(0x3f,16,2);


//-----Speed Sensors-----//

int sensor = 7; // broche pour détection du capteur
int etatSensor ; // état du capteur (haut ou bas)
unsigned long start_time=0; //temps de début d'un comptage
unsigned long end_time =0; //temps de fin d'un comptage

int nb_trous =20 ; // nombre de trous de la roue codeuse
int etat_old= 1 ; // 
int  etat_new = 1; // les états vont changer à chaque chaque modiication de la valeu lue par le capteur (haut/5V ou bas/0V)
int compt=0; // comptage initialisé à 0
float rps=0; // vitesse intialisée à 0
int rpm;
long temps; //mesure du temps pour l'acquisition

bool check = false;


//-----Speed Sensor-----//

#include "ACS712.h"

//  Arduino UNO has 5.0 volt with a max ADC value of 1023 steps
//  ACS712 5A  uses 185 mV per A
//  ACS712 20A uses 100 mV per A
//  ACS712 30A uses  66 mV per A

ACS712  ACS(A2, 5.0, 1023, 1000);
//  ESP 32 example (might requires resistors to step down the logic voltage)
//  ACS712  ACS(25, 3.3, 4095, 185)

float mA;

void setup() {
  Serial.begin(115200);

  //-----Initiate Compass-----//
  Wire.begin();
  compass.init();


  //-----Initiate Micro Servo-----//

  myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object


  //-----Rotation sense button-----//
  
  pinMode(button,INPUT_PULLUP);


  //-----Initiate Display-----//

  lcd.init();
  //lcd.backlight();

  //-----Speed Sensors-----//

  pinMode(sensor,INPUT); // la broche 3 est déclarée comme entrée
  temps = millis(); // mesure du temps 

  //-----Current Sensor//
  ACS.autoMidPoint();
}

void loop() {

  //-----Sensors-----//

  readCompass();
  //readVirtualCompass();
  
  measureCurrent();
  measureSpeed();

  controlDirection();

  //-----Propulsion-----//
  
  controlPropulsion();


  //-----Display-----//
  
  updateDisplay();
}
