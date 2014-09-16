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


#define DSB1820B_UUID 0x0018
#define DSB1820B_LENGTH_OF_DATA 2
#define DSB1820B_SCALE 100
#define DSB1820B_SHIFT 50

//counts number of Transmission Errors (TR)
#define TR_UUID 0x0003
#define RR_max 10    //maximum number of retries Xbee attempts before reporting error - this is the scalar
const int maxRetries = 5;  //how many times we attempt to send packets

byte minA1 = 0;
byte secA1 = 30;
byte minA2 = 1;



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

void setup()
{
  BeeDevice myBee;
}

void loop()
{
  

}



bool look4Response()
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
          break;
        }
      }
}