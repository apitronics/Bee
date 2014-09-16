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


 //maximum number of retries Xbee attempts before reporting error - this is the scalar
const int maxRetries = 5;  //how many times we attempt to send packets
// const uint16_t BEE_UUID = 0x8000;
byte minA1 = 0;
byte secA1 = 30;
byte minA2 = 1;




void setup()
{
  BeeDevice myBee;
  Serial.begin(57600);
  Serial.println("start this Bee up!");
  
  Serial.print("sample rate: ");
  Serial.print(myBee.getSampleMin());
  Serial.print(":");
  Serial.println(myBee.getSampleSecR());

  Serial.print("log rate: ");
  Serial.print(myBee.getLogMin());
  Serial.println(":00");

  Serial.print("sleep: ");
  Serial.println(myBee.isSleepEnable());

  minA1 = myBee.getSampleMin();
  secA1 = myBee.getSampleSecR();
  minA2 = myBee.getLogMin();



}

void loop()
{
  

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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

bool waitforResponse()
{
  const int timeout_ms = 2000;
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