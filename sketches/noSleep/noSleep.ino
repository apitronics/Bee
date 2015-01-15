
#include <Clock.h>
#include <Onboard.h>
#include <XBeePlus.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Bee.h>
#include <WeatherPlug.h>

#define NUM_SAMPLES 32
DateTime date = DateTime(__DATE__, __TIME__);

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;

#define NUM_SENSORS 2
Sensor * sensor[] = {&onboardTemp, &batteryGauge};
Sensorhub sensorhub(sensor,NUM_SENSORS);

#define DEBUG
//#define XBEE_ENABLE

void setup(){  
  xbee.begin(9600);
  Serial.begin(57600);
  delay(1000);
  
  clock.begin(date);
  sensorhub.init();

  Serial.println("IDs:");
  Serial.print("[");
  for(int i=0; i<UUID_WIDTH*NUM_SENSORS;i++){
    Serial.print(sensorhub.ids[i]);
    Serial.print(", ");
  }
  Serial.print("]");
  Serial.println();
  
  Serial.println("starting");

}


uint32_t count = 0;

void loop(){

  

  if( true ){
    //Serial.println("Sampling sensors");
    sensorhub.sample(false);
    count++;
  }
  
  //this if statement will average out the aggregated samples and send them to the Hive
  if( count == 32){
    clock.print();
    Serial.println("Creating datapoint from samples");
    sensorhub.log(true); 
    count = 0;
  }
}



