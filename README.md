# ECE4810 - Specific Gravity Sensor
## V3.0
Major Changes:
* Removes PulseinZero() command
* No longer uses echo pin on sensor
* Removes all calculations from code. Will be done by the app

## V2.0
Major Changes:
* Removes Pulsein() command in exchange for hardware interupts
* No more LCD output (was causing weird delays in sensor reading)

## Notes:

PulseInOne.zip enables the program to listen to the INT1 port on the arduino. Install libraty in arduino IDE and connect "echo" pin to corosponding INT pin per chart:

https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

Specific Gravity calculation is arbitraty. 
