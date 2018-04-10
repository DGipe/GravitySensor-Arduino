/*
* Ultrasonic Sensor - ECE4820
*
* Bench Test - V 3.0
* 
* Brandon Robbins
* David Gipe
* Izat Dahger
*/

#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

//Variables
  //pins
  
  const int trigPin = 12; //Ultrasonic trigger, echo on pin 1 EXTI 1
  const int tempPin = 2; //LM35 input
  const int voltPin = 1; //Battery Monitoring
  const int pulsePin = 9; //Un-processed input from RX of sensor
  
  //status light
  const int LED = 13;
  
  //Specific Gravity

  float specificGravity;
  unsigned long lastTime  = 0;
  int pingTimer     = 0;
  int pingDelay     = 300; // milliseconds between ping pulses  
  int startTime; //Start time of first pulse
  int runTime; //Total time of combined pulses

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
  SerialUSB.begin(9600); //USB Comm. Diagnostisc purposes only. Not going to be in final version
  pinMode(LED,OUTPUT);
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
 
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
    String data = String(runTime) + "," + String(tempRaw) + "," + String(volActual);
    Serial2.print(data); // print this to bluetooth module
    SerialUSB.println(data);  //USB Comm. Diagnostisc purposes only. Not going to be in final version
    }  
}


void ping(){

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  int wait = 0;
  bool start = false;

  do{
      int pulseState = digitalRead(pulsePin);

      if ((pulseState == HIGH) && (start == false)) { //If first signal is recieved
          start = true;
          startTime = micros(); //Get start time      
          }
          
      if ((pulseState == LOW) && (start ==true)) { //Counter to ensure signal is complete
          wait++;
          }
          
      if ((pulseState == HIGH) && (start ==true)){
          runTime = micros()-startTime;  //calculate time changed
          }

    }while ( wait < 700);
}
