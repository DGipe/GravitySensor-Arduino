/*
* Ultrasonic Sensor - ECE4810
*
* Bench Test - V 2.0
* 
* David Gipe
*/

//SD Card
#include <SD.h>
#include <SPI.h>

#include <PulseInOne.h> //Sets to listen on INT1
File dataFile;


//Variables
  //Pins
  const int trigPin = 12; //Ultrasonic trigger
  const int csPin = 53; //SD Card pin
  const int tempPin = 1; //LM35 input

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

  //Log Counters
  int logCount = 0;
  int logNum = 1;
  
void setup() {

  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);

  PulseInOne::setup(pingReturn);

  SD.begin();
  pinMode(csPin, OUTPUT);
}

void loop() {

  unsigned long time = millis();
  unsigned long dt   = time - lastTime;
  lastTime = time;
  
  pingTimer += dt;
  if(pingTimer > pingDelay){  
  pingTimer = 0;
  ping();
  } 

  tempRaw = analogRead(tempPin);

//Calculations
 //Temp
 tempCel = (( tempRaw/1024.0)*5000)/10;  //Obtained from internet, standard for LM35
 tempFar = (tempCel*9)/5 + 32; //Standard C to F conversion
    
if (logCount == 27280) //Only write to SD card every 10min
{
 dataFile = SD.open("data.txt", FILE_WRITE);
 dataFile.println(logNum);
 dataFile.print("Raw delay time: ");
 dataFile.println(durationRaw);
 dataFile.print("Specific Gravity: ");
 dataFile.println(specificGravity, 3);
 dataFile.print("Temperature: ");
 dataFile.println(tempFar, 3);
 dataFile.println();
 dataFile.close(); 

  logCount = 0; //reset "timer"
  logNum++;
  }


logCount++;
}


void ping(){

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  PulseInOne::begin(); //Start listening on INT1
}


void pingReturn(unsigned long duration) {

  durationRaw = duration;
  specificGravity = .0003333*duration+.645;  //Equation from Y-intercept of two readings

  Serial.print("Raw: ");
  Serial.print(duration);
  Serial.print(" us, est. SG: ");
  Serial.print(specificGravity, 3);
  Serial.print(" Temp: ");
  Serial.print(tempFar, 1);
  Serial.println(" F");  
}

