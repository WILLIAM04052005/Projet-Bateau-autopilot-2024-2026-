//This code regulates the speed of the motor about the desired
//reference following Figure 4, where C(s) is the proportional 
//controller.

//INCLUDE
#include <FlexiTimer2.h>
#include <digitalWriteFast.h>
 
//DC MOTOR DRIVER, to be adjusted according to real prototype 
int motorMoins = 5;
int motorPlus = 6;
int enablePin = 9;
 
//TIMER
#define CADENCE_MS 10
volatile double dt = CADENCE_MS/1000.;
volatile double temps = -CADENCE_MS/1000.;

// ENCODER 
#define codeurInterruptionA 0                   //Pin 0 = interruptA
#define codeurInterruptionB 1                   //Pin 1 = interruptB
#define codeurPinA 2                            //Pin 2 = codeur channel A associé à l'interrupteur 0
#define codeurPinB 3                            //Pin 3 = codeur channel B associé à l'interrupteur 1
volatile long tick_codeur = 0;                  //nb tick codeur

//VARIABLES
volatile double omega;                         //vitesse rotation moteur en rad.s-1
volatile double theta_precedent = 0;           //pour calcul intégral
volatile double theta;                         //angle moteur en rad
volatile double commande = 0.;                 //commande moteur

volatile double vref = 1.5;
 
// P-Controller
volatile double Kp = 0.1; // controller gain 
volatile double P_x = 0.;

volatile double ecart = 0.;
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
  
  Serial.println(String(omega) + "  " + String(temps)+ "  " + String(commande));
  //delay(100);
}

void commandeMoteur(double commande){
    digitalWrite(motorPlus, HIGH); 
    digitalWrite(motorMoins, LOW); 
     // Normalisation et contrôle du moteur
  if (commande > 255) {
    commande = 255;  
    // sachant que l'on est branché sur un pont en H 
  } 
  else if (commande <0) {
    commande = 0;
  }  
    analogWrite(enablePin, commande);
}

//CALCULS                                          // Programme d'interruption
  
void calculs(){                                 // SERT AUSSI POUR IMPLEMENTATION le controlleur
  int codeurDeltaPos;   
  codeurDeltaPos = tick_codeur;           // Nombre de ticks codeur depuis la dernière fois
  tick_codeur = 0;
  
  omega = ((2.*3.141592*((double)codeurDeltaPos))/4500.)/dt;   // Calcul de la vitesse de rotation
  theta = theta_precedent + omega*dt;                         // Calcul position
  theta_precedent = theta;
 /******* Régulation proportionnelle ********/
  // Ecart entre la consigne et la mesure
  ecart = vref-omega;
 
  // Terme proportionnel
  P_x = Kp * ecart;
 
  // Calcul de la commande
  commande = P_x ;
  
 commande=commande*255/10; // A voir avec David
  
  //******* Fin régulation  ********/
 
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
