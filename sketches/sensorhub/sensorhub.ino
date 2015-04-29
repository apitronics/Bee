#include <Sensorhub.h>

TemperatureSensor temp;

#define NUM_SENSORS 3
Sensor *sensorArray[] = {&temp, &temp, &temp};


void setup(){
  OSC.CTRL |= OSC_RC32MEN_bm | OSC_RC32KEN_bm;  /* Enable the internal 32MHz & 32KHz oscillators */
  while(!(OSC.STATUS & OSC_RC32KRDY_bm));       /* Wait for 32Khz oscillator to stabilize */
  while(!(OSC.STATUS & OSC_RC32MRDY_bm));       /* Wait for 32MHz oscillator to stabilize */
  DFLLRC32M.CTRL = DFLL_ENABLE_bm ;             /* Enable DFLL - defaults to calibrate against internal 32Khz clock */
  CCP = CCP_IOREG_gc;                           /* Disable register security for clock update */
  CLK.CTRL = CLK_SCLKSEL_RC32M_gc;              /* Switch to 32MHz clock */
  OSC.CTRL &= ~OSC_RC2MEN_bm;                   /* Disable 2Mhz oscillator */
  Serial.begin(57600);
  Serial.println("STARTUP");
}

void loop(){
  for(int i=0; i<NUM_SENSORS; i++){
    sensorArray[i]->getSensor();
    sensorArray[i]->getEvent();
    Serial.print("This sensor receives ");
    Serial.print(sensorArray[i]->getSize());
    Serial.println(" bytes of raw data");
    Serial.print("Data: ");
    for(uint8_t j=0; j<sensorArray[i]->getSize(); j++){
      Serial.print("[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(sensorArray[i]->raw[j]);
      Serial.print(", ");
    }
    Serial.println();
  }
  delay(1000);
}
