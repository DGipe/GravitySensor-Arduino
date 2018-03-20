/*
* Ultrasonic Sensor - ECE4810
*
* Bench Test - V 2.1
* 
* David Gipe
*/


#include <PulseInOne.h> //Sets to listen on INT1

//Variables
  //Pins
  const int trigPin = 12; //Ultrasonic trigger
  const int tempPin = 2; //LM35 input
  const int volPin = A1; //Battery Monitoring

  //Specific Gravity
  int durationRaw;
  float specificGravity;
  unsigned long lastTime  = 0;
  int pingTimer     = 0;
  int pingDelay     = 500; // milliseconds between ping pulses

  //Temperature
  int tempRaw;
  float tempCel;
  float tempFar;

  //Battery Voltage
  float volActual;

void setup() {

  SerialUSB.begin(9600);

  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);

  PulseInOne::setup(pingReturn);

}

void loop() {
 // SerialUSB.println("Startup Initiated");
  unsigned long time = millis();
  unsigned long dt   = time - lastTime;
  lastTime = time;
  
  pingTimer += dt;
  if(pingTimer > pingDelay){  
  pingTimer = 0;
  ping();
  } 

  tempRaw = analogRead(tempPin);

//Calculations - will soon be offloaded to app to compute
 //Temp
 tempCel = (( tempRaw/1024.0)*5000)/10;  //Obtained from internet, standard for LM35
 tempFar = (tempCel*9)/5 + 32; //Standard C to F conversion

 //Voltage
 volActual = analogRead(volPin) * (5.0 / 1023.0);
   
}


void ping(){

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  PulseInOne::begin(); //Start listening on INT1
}


void pingReturn(unsigned long duration) {

  durationRaw = duration;
  specificGravity = .0003333*duration+.645;  //Equation from Y-intercept of two readings, arbirtrary

  SerialUSB.print("Raw: ");
  SerialUSB.print(duration);
  SerialUSB.print(" us, est. SG: ");
  SerialUSB.print(specificGravity, 3);
  SerialUSB.print(" Temp: ");
  SerialUSB.print(tempFar, 1);
  SerialUSB.print(" F ");
  SerialUSB.print(volActual);
  SerialUSB.println(" V");   
}

