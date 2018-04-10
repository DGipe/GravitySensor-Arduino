/*
* Ultrasonic Sensor - ECE4820
*
* Bench Test - V 2.2
* 
* Brandon Robbins
* David Gipe
* Izat Dahger
*/

#include <PulseInOne.h>

#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

//Variables
  //pins
  
const int trigPin = 12; //Ultrasonic trigger, echo on pin 1 EXTI 1
  const int tempPin = 2; //LM35 input
  const int voltPin = 1; //Battery Monitoring

  const int pulsePin = 9;
  
  //status light
  const int LED = 13;
  
  //Specific Gravity
  //attach echo to pin 1/Tx
  int durationRaw;
  float specificGravity;
  unsigned long lastTime  = 0;
  int pingTimer     = 0;
  int pingDelay     = 300; // milliseconds between ping pulses
  
  int pulseState = 0;
  int startTime;
  int runTime;

  //Temperature
  int tempRaw;

  //Battery Voltage
  float volActual;


  //Bluetooth
  unsigned long previous = 0;        // will store last time
  Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2); //bluetooth Module
  
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}
 
void setup() {
  SerialUSB.begin(9600);
  pinMode(LED,OUTPUT);
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);

  PulseInOne::setup(pingReturn);
 
  Serial2.begin(57600); //baud rate for your bluetooth module
  
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(10, PIO_SERCOM); //plug Rx here
  pinPeripheral(11, PIO_SERCOM); //plug Tx here

  delay(200); // wait for voltage stabilize
  
  delay(3000); // wait for settings to take affect. 
  
}
 

void loop() {
  if (Serial2.available()) { // check if anything in UART buffer
    if(Serial2.read() == '1'){ // did we receive this character?
       digitalWrite(13,!digitalRead(13)); // if so, toggle the onboard LED
    }
  }

  unsigned long current = millis();
  unsigned long dt   = current - lastTime;
  lastTime = current;
  
  pingTimer += dt;
  if(pingTimer > pingDelay){  
    pingTimer = 0;
    ping();
  } 


  current = millis();
  
  if (current - previous >= pingDelay) {
    
    tempRaw = analogRead(tempPin);
    volActual = analogRead(voltPin);
    
    previous = current;
    //for distance test use durationRaw = durationRaw/29/2;
    String data = String(durationRaw) + "," + String(tempRaw) + "," + String(volActual) + "," + String(runTime);
    Serial2.print(data); // print this to bluetooth module
    SerialUSB.println(data);
  }
  
}

void ping(){


  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  int wait = 0;
  bool start = false;

do{

pulseState = digitalRead(pulsePin);

if ((pulseState == HIGH) && (start == false)) {
    start = true;
    startTime = micros();
  
  }
if ((pulseState == LOW) && (start ==true))
{
  wait++;
}
if ((pulseState == HIGH) && (start ==true)){
  runTime = micros()-startTime;  
}

}while ( wait < 100);

  

//  PulseInOne::begin(); //Start listening on INT1
}

void pingReturn(unsigned long duration) {
  durationRaw = duration;
}
