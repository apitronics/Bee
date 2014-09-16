//Apitronics - hive2bee.ino
//9-16-2014
// demonstrate read service with BeeDevice

#include <Clock.h>
#include <Onboard.h>
#include <XBeePlus.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Bee.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BeeDevice.h>




#define XBEE_ENABLE
DateTime date = DateTime(__DATE__, __TIME__);

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;
BeeDevice myBee;

#define NUM_SENSORS 5
Sensor * sensor[] = {&onboardTemp, &batteryGauge};
Sensorhub sensorhub(sensor,NUM_SENSORS);

 //maximum number of retries Xbee attempts before reporting error - this is the scalar
const int maxRetries = 5;  //how many times we attempt to send packets
// const uint16_t BEE_UUID = 0x8000;
byte minA1 = 0;
byte secA1 = 30;
byte minA2 = 1;
bool sleep_enabled = true;
uint16_t response_timeout_ms = 5000;




void setup()
{


  delay(2000);
  pinMode(5,OUTPUT);  
  digitalWrite(5,HIGH);
  Serial.begin(57600);
  xbee.begin(57600);
  //xbee.hardReset();
  Serial.print("Initialized: serial");  
  clock.begin(date);
  configureSleep();
  Serial.print(", clock");  
  sensorhub.init();
  Serial.println(", sensors");

  initXbee();

  explicitBee();
  minA1 = myBee.getSampleMin();
  secA1 = myBee.getSampleSecR();
  minA2 = myBee.getLogMin();
  sleep_enabled = myBee.isSleepEnable();



  Serial.println("Launching program.");

}
bool firstRun=true;

void loop()
{
  bool buttonPressed =  !clock.triggeredByA2() && !clock.triggeredByA1() ;
  clock.print();
  if( clock.triggeredByA1() ||  buttonPressed || firstRun)
  {
    sampleDevices();
  }
  if( ( clock.triggeredByA2() ||  buttonPressed ||firstRun))
  {
    logDevices();
  }

firstRun=false;
#ifdef XBEE_ENABLE
xbee.disable();
#endif
sleep(); 

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
boolean sendIDPacket(uint8_t * pointer, uint8_t length){
    for(int i=0; i<maxRetries;i++){
      if( xbee.sendIDs(pointer, length) ) {
        return true;
      }
      if (maxRetries - i == 2){
        delay(100);
      }
    }
    clock.setAlarm2Delta(minA2);
    sleep();
    return false;
}

boolean sendDataPacket(uint8_t * arrayPointer, uint8_t arrayLength){
    for(int i=0; i<maxRetries;i++){ 
      if( xbee.sendData(arrayPointer, arrayLength) ) {
        return true;
      }
      if (maxRetries - i == 2){
        delay(100);
      }
    }
    return false;
}

bool waitforResponse(uint16_t timeout_ms = 2000)
{
  unsigned int refreshCntr = 0;
  xbee.refresh();
      Serial.print("waiting for response");
      while(!xbee.available()){
        xbee.refresh();
        Serial.print(".");
        delay(1);
        return true;
        refreshCntr++;
        if (refreshCntr >= timeout_ms){
          return false;
        }
      }
  return false;
}

void initXbee()
{
  #ifdef XBEE_ENABLE
  
  const int refreshPeriod = 2000;
  bool connected2Hive = false;
  unsigned int refreshCntr = 0;
  do
      {
      Serial.println("Connecting to Hive...");
      //send IDs packet until reception is acknowledged'
      while(!sendIDPacket(&sensorhub.ids[0],UUID_WIDTH*NUM_SENSORS));  
      Serial.println(" Hive received packet...");
      //wait for message from Coordinator
      xbee.refresh();
      Serial.print("refreshing");
      while(!xbee.available()){
        xbee.refresh();
        Serial.print(".");
        delay(1);
        connected2Hive = true;
        refreshCntr++;
        if (refreshCntr % refreshPeriod == 0){
          if (refreshCntr >= (refreshPeriod*3)){
              clock.setAlarm2Delta(minA2);
              xbee.hardReset();
              sleep();
              refreshCntr = 0;        
           }
          connected2Hive = false;
          break;
        }
      }
  } while(!connected2Hive);
  
  
  Serial.println("done");
  xbee.meetCoordinator();
  
  Serial.println(" Hive address saved.");
  #endif

}

void explicitBee()
{
  Serial.println("my Bee settings:");
  
  Serial.print("sample rate: ");
  Serial.print(myBee.getSampleMin());
  Serial.print(":");
  Serial.println(myBee.getSampleSecR());

  Serial.print("log rate: ");
  Serial.print(myBee.getLogMin());
  Serial.println(":00");

  Serial.print("sleep: ");
  Serial.println(myBee.isSleepEnable());

}

void sampleDevices()
{
    Serial.println("Sampling sensors:");
    sensorhub.sample(true);
    clock.setAlarm1Delta(minA1, secA1);
}

void logDevices()
{
      xbee.enable();
      Serial.println("Logging datapoint from samples");
      sensorhub.log(true);
      #ifdef XBEE_ENABLE
      sendDataPacket(&sensorhub.data[0], sensorhub.getDataSize());
      if(waitforResponse(response_timeout_ms) == true)
      {
        parseResponse();
      }

      #endif
      clock.setAlarm2Delta(minA2);
}

void parseResponse()
{
  #ifdef XBEE_ENABLE
  int index = (int)xbee.getResponseLength();
  uint8_t dataByte = 0;
  for(int i=0; i<index; i++){
      dataByte = xbee.getResponseByte(i);

      Serial.println("here comes data:");
      Serial.println(dataByte, HEX);


  }





  #endif
}