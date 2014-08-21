#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Onboard.h>

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;


#define NUM_SENSORS 2

Sensor * sensor[] = {&onboardTemp, &batteryGauge,&onboardTemp1, &batteryGauge1};

Sensorhub sensorhub(sensor,NUM_SENSORS);


void setup(){
  
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
  sensorhub.log(true);
  
  delay(1000);
  
}
