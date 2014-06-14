#include <Sensorhub.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
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

void setup(){  
  xbee.begin(9600);
  Serial.begin(57600);
  delay(1000);
  
  clock.begin(date);
  configureSleep();
  sensorhub.init();
  
  #ifdef DEBUG
  Serial.println("IDs:");
  Serial.print("[");
  for(int i=0; i<UUID_WIDTH*NUM_SENSORS;i++){
    Serial.print(sensorhub.ids[i]);
    Serial.print(", ");
  }
  Serial.print("]");
  Serial.println();
  
  Serial.println("starting");
  #endif
  
  xbee.sendIDs(&sensorhub.ids[0], UUID_WIDTH*NUM_SENSORS);
  xbee.refresh();
  while(!xbee.available()){
    xbee.refresh();
  }
  xbee.meetCoordinator();
}


long int start=0;

void loop(){
  clock.print();
  start = millis();
  for(int i=0;i<NUM_SAMPLES;i++) sensorhub.sample();
 
 
  #ifdef DEBUG 
  sensorhub.log();
  #else
  sensorhub.log(true);
  #endif
  
    
  xbee.sendData(&sensorhub.data[0], sensorhub.getDataSize());
  
  clock.setAlarm1Delta(0, 15);
  
  sleep();
  
}



