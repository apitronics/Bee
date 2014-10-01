
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
  
  #ifdef XBEE_ENABLE
  //This is where the Bee associates itself with the Hive
  while(!xbee.sendIDs(&sensorhub.ids[0], UUID_WIDTH*NUM_SENSORS));
  xbee.refresh();
  //Now we wait for the Hive to respond
  while(!xbee.available()){
    xbee.refresh();
  }
  //Once the Hive has responded, we will know the address of the Coordinator module
  xbee.meetCoordinator();
  #endif
}


bool firstRun=true;

void loop(){
  //if A1 woke us up and its log time OR if its the first run OR if the button has been pushed
  bool buttonPressed =  !clock.triggeredByA2() && !clock.triggeredByA1() ;
  clock.print();
  Serial.print("Button pressed: ");
  Serial.println(buttonPressed);
  
  //this if statement just samples
  if( clock.triggeredByA1() ||  buttonPressed || firstRun){
    Serial.println("Sampling sensors");
    sensorhub.sample(true);
    clock.setAlarm1Delta(0,15);
  }
  
  //this if statement will average out the aggregated samples and send them to the Hive
  if( ( clock.triggeredByA2() ||  buttonPressed ||firstRun)){
    #ifdef XBEE_ENABLE
    xbee.enable();
    #endif
    Serial.println("Creating datapoint from samples");
    sensorhub.log(true); 
    #ifdef XBEE_ENABLE
    while(!xbee.sendData(&sensorhub.data[0], sensorhub.getDataSize()));
    xbee.disable();
    #endif
    clock.setAlarm2Delta(15);
  }
  
  firstRun=false;
  sleep(); 
}



