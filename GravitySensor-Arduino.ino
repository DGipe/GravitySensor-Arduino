/*
* Ultrasonic Sensor - ECE4810
*
* Bench Test - V 2.0
* 
* David Gipe
*/



#include <PulseInOne.h> //Sets to listen on INT1
#include <Arduino.h>
#include <samd.h>
#include "wiring_private.h" // pinPeripheral() function

// Constants for Clock generators
#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)
#define GENERIC_CLOCK_GENERATOR_XOSC32K   (1u)
#define GENERIC_CLOCK_GENERATOR_OSCULP32K (2u) /* Initialized at reset for WDT */
#define GENERIC_CLOCK_GENERATOR_OSC8M     (3u)
// Constants for Clock multiplexers
#define GENERIC_CLOCK_MULTIPLEXER_DFLL48M (0u)





//Variables
  //Pins
  const int trigPin = 12; //Ultrasonic trigger


  //Specific Gravity
  int durationRaw;
  float distance;
  unsigned long lastTime  = 0;
  int pingTimer     = 0;
  int pingDelay     = 500; // milliseconds between ping pulses
  int flag = 0;

  


void loop() {

  
  

  unsigned long time = millis();
  unsigned long dt   = time - lastTime;
  lastTime = time;
  
  pingTimer += dt;
  if(pingTimer > pingDelay){  
  pingTimer = 0;
  ping();
  }

  
} 
void ping(){

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  PulseInOne::begin(); //Start listening on INT1
}

void pingReturn(unsigned long duration) {
  durationRaw = duration;
  if(flag == 1){
    SerialUSB.print("Raw: ");
    SerialUSB.println(durationRaw);
    flag = 0;
  }else{
    SerialUSB.print("\tRaw: ");
    SerialUSB.println(durationRaw);
    flag = 1; 
  }
}

void setup(){
  // Set 1 Flash Wait State for 48MHz, cf tables 20.9 and 35.27 in SAMD21 Datasheet 
  NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val; 

  //Turn on the digital interfce clock
  PM->APBAMASK.reg |= PM_APBAMASK_GCLK;

  // STEP 1 -> ENABLE THE XOSC32K CLOCK (EXTERNAL ON-BOARD CRYSTAL)
  SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP (0x6u) | SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K;
  //seperate call 
  SYSCTRL->XOSC32K.bit.ENABLE = 1; 

  while((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == 0){
    // WAIT FOR THE OSCILLATOR TO STABALIZE
  }
  /*
   * SOFTWARE RESET THE MODULE TO ENSURE IT IS RE-INITIALIZED CORRECTLY.
   * NOTE: DUE TO SYNCHRONIZATION, THERE IS A DELAY FROM WRITING CTRL.SWRST UNTIL THE RESET IS COMPLETE
   * CTRL.SWRST AND STATUS.SYNCBUSY WILL BOTH BE CLEARED WHEN THE RESET IS COMPLETE
   */
//   GCLK->CTRL.reg = GCLK_CTRL_SWRST;
//
//   while((GCLK->CTRL.reg & GCLK_CTRL_SWRST) && (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)){
//    // WAIT FOR RESET TO COMPLETE
//   }

   //STEP 2 -> PUT XOSC32K AS SOURCE OF GENERIC CLOCK GENERATOR 1
   GCLK->GENDIV.reg = GCLK_GENDIV_ID(GENERIC_CLOCK_GENERATOR_XOSC32K);
   
   
   while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){
    //WAIT FOR SYNCRONIZATION 
   }

   //WRITE GENERIC CLOCK GENERATOR 1 CONFIGURATION
   GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(GENERIC_CLOCK_GENERATOR_XOSC32K) | // Generic Clock Generator 1
                      GCLK_GENCTRL_SRC_XOSC32K | // Selected source is External 32KHz Oscillator
//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                      GCLK_GENCTRL_GENEN ;
    while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){
      //WAIT FOR SYNCRONIZATION
    }

    //STEP 3 -> PUT GENERIC CLOCK GENERATOR 1 AS SOURCE FOR GENERIC CLOCK MULTIPLEXER 0 (DFLL48M REFERENCE)
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GENERIC_CLOCK_MULTIPLEXER_DFLL48M) | //GENERIC CLOCK MULTIPLEXTER 0
                        GCLK_CLKCTRL_GEN_GCLK1 | //GENERIC CLOCK GENERATOR 1 IS SOURCE
                        GCLK_CLKCTRL_CLKEN;
    while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){
      // WAIT FOR SYNCHRONIZATION
    }

    //STEP 4 -> ENABLE DFLL48M CLOCK

    //DFLL CONFIGURATION IN CLOSED LOOP MODE 

    //REMOVE THE OnDemand MODE, ITS A BUG http://avr32.icgroup.norway.atmel.com/bugzilla/show_bug.cgi?id=9905
    SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;

    while((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0){
      //WAIT FOR SYNCRONIZATION
    }
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | //ENABLE THE CLOSED LOOP MODE
                             SYSCTRL_DFLLCTRL_WAITLOCK |
                             SYSCTRL_DFLLCTRL_QLDIS; //DISABLE QUICK LOCK
    while((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0){
      //WAIT FOR SYNCRONIZATION
    }

    //ENABLE THE DFLL
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE;

    while((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 || (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0){
      // WAIT FOR LOCKS FLAGS
    }

    while((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0){
      //WAIT FOR SYNCRONIZATION
    }

    //STEP 5 -> SWITCH THE GENERIC CLOCK GENERATOR 0 TO DFLL48M, CPU WILL RUN AT 48MHz
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(GENERIC_CLOCK_GENERATOR_MAIN); // GENERIC CLOCK GENERATOR
    while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){
      //WAIT FOR SYNCHRONIZATION
    }

    //WRITE GENERIC CLOCK GENERATOR 0 CONFIGURATION
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(GENERIC_CLOCK_GENERATOR_MAIN) | // GENERIC CLOCK  GENERATOR 0
                        GCLK_GENCTRL_SRC_DFLL48M | //SELECTED SOURCE IS DFLL 48MHz
    //                  GCLK_GENCTRL_OE | //OUTPUT CLOCK TO A PIN FOR TESTS
                        GCLK_GENCTRL_IDC | //SET 50/50 DUTY CYCLE
                        GCLK_GENCTRL_GENEN;
    while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){
      //WAIT FOR SYNCRONIZATION                    
    }

    // STEP 6 -> MODIFY PRESCALER VALUE OF 0SC8M TO HAVE 8MHz 

    SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_1_Val;
    SYSCTRL->OSC8M.bit.ONDEMAND = 0;

    //STEP 7 -> PUT 0SC8M AS SOURCE FOR GENERIC CLOCK GENERATOR 3

    GCLK->GENDIV.reg = GCLK_GENDIV_ID(GENERIC_CLOCK_GENERATOR_OSC8M); // GENERIC CLOCK GENERATOR 3

    //WRITE GENERIC CLOCK GENERATOR 3 CONFIGURATION
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(GENERIC_CLOCK_GENERATOR_OSC8M) | // GENERIC CLOCK GENERATOR 3
                        GCLK_GENCTRL_SRC_OSC8M | // SELECTED SOURCE IS RC OSC 8MHz (ALREADY ENABLED AT RESET)
//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                        GCLK_GENCTRL_GENEN ;

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY ){
    //WAIT FOR SYNCRONIZATION
  }
  /* NOW THAT ALL SYSTEM CLOCK ARE CONFIGURED, WE CAN SET CPU AND APBX BUS CLOCKS.
   *  THERE VALUES ARE NORMALLY THE ONE PRESENT AFTER RESET
   */
  PM->CPUSEL.reg = PM_CPUSEL_CPUDIV_DIV1;
  PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1_Val;
  PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1_Val;
  PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1_Val;   

  SystemCoreClock = VARIANT_MCK;

  //STEP 8 -> LOAD ADC FACTORY CALIBRATION VALUES
  
  //ADC BIAS CALIBRATION
  uint32_t bias = (*((uint32_t *) ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;

  //ADC LINEARITY BITS 4:0
  uint32_t linearity = (*((uint32_t*) ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;

  //ADC Linearity bits 7:5
  linearity |= ((*((uint32_t*) ADC_FUSES_LINEARITY_1_ADDR) & ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;

  //STEP 9 -> DISABLE AUTOMATIC NVM WRITE OPERATIONS

  NVMCTRL->CTRLB.bit.MANW = 1;

  SerialUSB.begin(9600);
  while(!SerialUSB);

  SerialUSB.println("Up and Running");

  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);

  PulseInOne::setup(pingReturn);
}   



//SYSTEM INIT HELP FOUND HERE https://electronics.stackexchange.com/questions/332569/how-to-configure-atmel-sam-d20-for-internal-48mhz-clock-source

