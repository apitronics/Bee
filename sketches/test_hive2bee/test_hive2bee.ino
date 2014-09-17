//Apitronics - hive2bee.ino
//9-17-2014
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





//-------------------------------------------//
class Dummy: public Sensor
{
      public:
        Dummy(uint16_t myUUID):Sensor(myUUID, 2, 1, 0, true, 1)
        {
          _myUUID = myUUID;
        }
        void init(){
        }
        String getName(){
          return "dummy";
        }
        String getUnits(){
          return " units";
        }
        void getData(){
          int tmp = 0;
          data[1] = tmp >> 8;
          data[0] = tmp;
        }
      private:
        uint16_t _myUUID;
};
//------------------------------------------//

#define XBEE_ENABLE
DateTime date = DateTime(__DATE__, __TIME__);

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;
BeeDevice myBee;
Dummy dummyBee(0x8000);

#define NUM_SENSORS 3
Sensor * sensor[] = {&onboardTemp, &batteryGauge, &dummyBee};
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
  Serial.println("");
  Serial.println("===============================================");
  Serial.print("Initialized: serial");

  
  clock.begin(date);
  configureSleep();
  Serial.print(", clock");  
  sensorhub.init();
  Serial.print(", sensors ");
  for(int i=0; i<NUM_SENSORS; i++){
      Serial.print(sensorhub.sensors[i]->getUUID(), HEX);
      Serial.print(" ");
  }
  Serial.println("");


  initBee();

  explicitBeeDevice();
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

void initBee()
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

void explicitBeeDevice()
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


bool waitforResponse(uint16_t timeout_ms = 2000)
{
  unsigned int refreshCntr = 0;
  xbee.refresh();
    Serial.print("waiting for response");
      while(!xbee.available()){
        xbee.refresh();
        Serial.print(".");
        delay(1);
        refreshCntr++;
        if (refreshCntr >= timeout_ms){
          Serial.println("timeout");
          return false;
        }
      }
    uint8_t responseID = xbee.getResponseApiID();
    Serial.println(responseID, HEX);
    if(responseID == 0x90){
      xbee.pullData();
      return true;      
    }
    else {
      Serial.print(" wrong api id");
      return false;
    }
}



void parseResponse()
{
  int index = (int)xbee.getResponseLength();
  uint8_t dataByte = 0;
  Serial.println("received data:");
  for(int i=0; i<index; i++){
      dataByte = xbee.getResponseByte(i);
      Serial.print(dataByte, HEX);
      Serial.print(" ");
  }
  Serial.println("");
}


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

