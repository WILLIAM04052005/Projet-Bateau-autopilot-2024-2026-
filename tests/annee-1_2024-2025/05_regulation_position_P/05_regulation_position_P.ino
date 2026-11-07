/*
 * 05_regulation_position_P
 * Projet Bateau autopiloté · Année 1 (2024-2025) · Domaine EEE / MIA
 *
 * Régulation de POSITION angulaire du moteur par correcteur proportionnel (Kp = 0.56, consigne 2π rad).
 * Même base que le test 04 (codeur 4500 points/tour, FlexiTimer2, L293D).
 *
 * Fichier d'origine : Angle_control/Angle_control.ino
 */

//This code control motor's position  (in rad)
//based on the encoder counts.
//INCLUDE
#include <FlexiTimer2.h>
#include <digitalWriteFast.h>

 int codeurDeltaPos;
//
int motorMoins = 5;
int motorPlus = 6;
int enablePin = 9;
 
//TIMER
#define CADENCE_MS 10
volatile double dt = CADENCE_MS/1000.;
volatile double temps = -CADENCE_MS/1000.;

// 
#define codeurInterruptionA 0                   //Pin 0 = interruptA
#define codeurInterruptionB 1                   //Pin 1 = interruptB
#define codeurPinA 2                            //Pin 2 = codeur channel A associé à l'interrupteur 0
#define codeurPinB 3                            //Pin 3 = codeur channel B associé à l'interrupteur 1
volatile long tick_codeur = 0;                  //nb tick codeur

//VARIABLES
volatile double omega;                         //vitesse rotation moteur en rad.s-1
volatile double theta_precedent;//=EEPROM.read(600)/100;           //pour calcul intégral
volatile double theta;                         //angle moteur en rad
volatile double commande = 0.;                 //commande moteur

volatile double angref=2*3.141592 ;
 

volatile double Kp = 0.56;

volatile double P_x = 0.;

volatile double ecart ;
void setup() {
    // Configuration des ports en mode "sortie"
    pinMode(motorMoins, OUTPUT);
    pinMode(motorPlus, OUTPUT);
    pinMode(enablePin, OUTPUT);

    // ENCODEUR
    pinMode(codeurPinA, INPUT); 
    pinMode(codeurPinB, INPUT);
    digitalWrite(codeurPinA, HIGH);                 //turn pullup resistor on
    digitalWrite(codeurPinB, HIGH);                 //turn pullup resistor on
    attachInterrupt(0, updateEncoderA, CHANGE);     //Pin 0 et 1 interrupteur à chaque changement d'état
    attachInterrupt(1, updateEncoderB, CHANGE);     //renvoi vers updateEncoder
    
    Serial.begin(9600);
    Serial.flush();

    tick_codeur = 0;

    FlexiTimer2::set(CADENCE_MS, 1/1000., calculs); // résolution timer = 1 ms
    FlexiTimer2::start();    
}


void loop() {
  //commande = 255.;
  //commandeMoteur(commande);
  Serial.println(String(theta)+ "  " + String(commande));
  //delay(100);
}

void commandeMoteur(double commande){
    digitalWrite(motorPlus, HIGH); 
    digitalWrite(motorMoins, LOW); 
     // Normalisation et contrôle du moteur
  if (commande > 255) {
    commande = 255;  // sachant que l'on est branché sur un pont en H L293D
  } 
  else if (commande <0) {
    commande = 0;
  }  
    analogWrite(enablePin, commande);
}

//CALCULS                                          // Programme d'interruption
  
void calculs(){  
 
 
  // SERT AUSSI POUR IMPLEMENTATION PID
     
  codeurDeltaPos = tick_codeur;           // Nombre de ticks codeur depuis la dernière fois
  tick_codeur =0;
  
  omega = ((2.*3.141592*((double)codeurDeltaPos))/4500.)/dt;   // Calcul de la vitesse de rotation
  theta = theta_precedent + omega*dt;                         // Calcul position
  theta_precedent = theta;
 // EEPROM.write(600, theta*100);
 /******* P controller ********/
  // Ecart entre la consigne et la mesure
  //if 
  ecart = angref-theta;
 
  // Terme proportionnel
  P_x = Kp * ecart;
 
  // Calcul de la commande
  commande = P_x ;
  
 commande=commande*255/10;

  /******* Fin control ********/
 
  // Envoi de la commande au moteur
  
   commandeMoteur(commande);
 
  temps += dt;
}

void updateEncoderA()
{
  if (digitalReadFast2(codeurPinA) == digitalReadFast2(codeurPinB)) {
    tick_codeur--;
  }
  else {
    tick_codeur++;
  }
}

void updateEncoderB()
{
  if (digitalReadFast2(codeurPinA) == digitalReadFast2(codeurPinB)) {
    tick_codeur++;
  }
  else {
    tick_codeur--;
  }
}
