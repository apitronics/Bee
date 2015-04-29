#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Onboard.h>

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;


#define NUM_SENSORS 2

Sensor * sensor[] = {&onboardTemp, &batteryGauge,&onboardTemp};

Sensorhub sensorhub(sensor,NUM_SENSORS);


void setup(){
  OSC.CTRL |= OSC_RC32MEN_bm | OSC_RC32KEN_bm;  /* Enable the internal 32MHz & 32KHz oscillators */
  while(!(OSC.STATUS & OSC_RC32KRDY_bm));       /* Wait for 32Khz oscillator to stabilize */
  while(!(OSC.STATUS & OSC_RC32MRDY_bm));       /* Wait for 32MHz oscillator to stabilize */
  DFLLRC32M.CTRL = DFLL_ENABLE_bm ;             /* Enable DFLL - defaults to calibrate against internal 32Khz clock */
  CCP = CCP_IOREG_gc;                           /* Disable register security for clock update */
  CLK.CTRL = CLK_SCLKSEL_RC32M_gc;              /* Switch to 32MHz clock */
  OSC.CTRL &= ~OSC_RC2MEN_bm;                   /* Disable 2Mhz oscillator */
  
  Serial.begin(57600);
  sensorhub.init();
}

void loop(){
  Serial.println("16 samples: ");
  for(int i=0; i<16; i++){
    sensorhub.sample(true);
    delay(25);
  }
  
  Serial.print("Datapoint: ");
  sensorhub.hold(true);
  
  delay(1000);
  
}
