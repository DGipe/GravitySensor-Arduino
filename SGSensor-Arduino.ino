/*
* Ulstrasonic Sensor - ECE4600
*
* Bench Test - V 1.2
* 
* David Gipe
*/

//LCD
#include <LiquidCrystal.h> 
LiquidCrystal lcd(1, 2, 4, 5, 6, 7); 

//SD Card
#include <SD.h>
#include <SPI.h>
File dataFile;


//Variables
  //Pins
  const int trigPin = 9;
  const int csPin = 53; 
  const int echoPin = 10;
  const int tempPin = 1;

  //Specific Gravity
  long duration;
  int durationRaw;
  float specificGravity;

  //Temperature
  int tempRaw;
  float tempCel;
  float tempFar;

  //Log Counters
  int logCount = 0;
  int logNum = 1;
  
void setup() {
  
lcd.begin(16,2); 
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);

SD.begin();
pinMode(csPin, OUTPUT);

}

void loop() {

//Sensor Read  
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);

tempRaw = analogRead(tempPin);

//Calculations
  //Gravity
  durationRaw = duration;
  specificGravity = .0003333*duration+.645;

  //Temp
  tempCel = (( tempRaw/1024.0)*5000)/10; 
  tempFar = (tempCel*9)/5 + 32;
  
if (logCount = 600000){
  
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

 logCount = 0;
 logNum++;
}

lcd.setCursor(0,0); 
lcd.print("SG: ");

if (specificGravity < 1)
{
  lcd.print("Error");
}
else
{
lcd.print(specificGravity, 3);
}

lcd.print("  ");
lcd.print(durationRaw); 
lcd.setCursor(0,1);
lcd.print("Temp: "); 
lcd.print(tempFar, 1); 
lcd.print(" F");

logCount++;
delay(10);
}
