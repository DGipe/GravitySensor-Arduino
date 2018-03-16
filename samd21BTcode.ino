#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

//Variables
  //pins
  const int LED = 13;
  int count = 0;
  int count2 = 60;


  unsigned long lastTime  = 0;


  unsigned long previousMillis = 0;        // will store last time
  const long interval = 500;           // interval at which to delay
 
Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2); //bluetooth Module
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}
 
void setup() {
  pinMode(LED,OUTPUT);
 
  Serial2.begin(57600); //baud rate for bluetooth module
  
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(10, PIO_SERCOM); //plug Rx here
  pinPeripheral(11, PIO_SERCOM); //plug Tx here

  delay(200); // wait for voltage stabilize
  
  delay(3000); // wait for settings to take affect. 
  
}
 

void loop() {

  unsigned long currentMillis = millis();
  unsigned long dt   = currentMillis - lastTime;
  lastTime = currentMillis;
  

  
  if (Serial2.available()) { // check if anything in UART buffer
    if(Serial2.read() == '1'){ // did we receive this character?
       digitalWrite(13,!digitalRead(13)); // if so, toggle the onboard LED
    }
  }


  currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {

    if (count >= 99){
    count = 0;
  }else{
    count++;
  }
  if (count2 <= 0){
    count2 = 99;
  }else{
    count2--;
  }
    
    previousMillis = currentMillis;
    String data = String(count) + "," + String(count2) + "F" + ",74.5%";
    Serial2.print(data); // print this to bluetooth module
  }
  
}
