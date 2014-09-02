//Apitronics - DS1820B.ino
//Sept 2

#include <Clock.h>
#include <Onboard.h>
#include <XBeePlus.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Bee.h>

#define XBEE_ENABLE

#include <OneWire.h>
#include <DallasTemperature.h>

#define DSB1820B_UUID 0x0018
#define DSB1820B_LENGTH_OF_DATA 2
#define DSB1820B_SCALE 100
#define DSB1820B_SHIFT 50

//counts number of Transmission Errors (TR)
#define TR_UUID 0x0003
#define RR_max 10    //maximum number of retries Xbee attempts before reporting error - this is the scalar
const int maxRetries = 5;  //how many times we attempt to send packets

const byte minA1 = 0;
const byte secA1 = 10;
const byte minA2 = 1;







//~~~~~~~~~~~~~~~~~~~~~~~~~CLASSES~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//Create Sensorhub Counter for counting Transmission errors
class Counter: public Sensor
{
      public:
        Counter():Sensor(TR_UUID, 2, 1, 0, true, 1){};
        void init(){
          _XB_TR_cntr = 0;
        }
        void setZero(){
            _XB_TR_cntr = 0;
            return;
        }
        void incr(){
            _XB_TR_cntr += RR_max;
            return;
        }
        String getName(){
          return "Xbee Transmission Attempts";
        }
        String getUnits(){
          return " attempts";
        }
        void getData(){
          int tmp = _XB_TR_cntr;
          data[1] = tmp >> 8;
          data[0] = tmp;
        }
      private:
        unsigned int _XB_TR_cntr;            
};

//Create Sensorhub Sensor from DallasTemp library
class DSB1820B: public Sensor
{
        public:
                OneWire oneWire;
                DallasTemperature dsb = DallasTemperature(&oneWire);
                uint8_t _index;
                                
                DSB1820B(uint8_t index=0, uint8_t samplePeriod=1):Sensor(DSB1820B_UUID, DSB1820B_LENGTH_OF_DATA, DSB1820B_SCALE, DSB1820B_SHIFT, false, samplePeriod){
                  _index = index;
                };
                String getName(){ return "Temperature Probe"; }
                String getUnits(){ return "C"; }
                void init(){
                  dsb.begin();
                }
                void getData(){
                      
                  dsb.requestTemperatures();
                  float sample = dsb.getTempCByIndex(_index);
                  uint16_t tmp = (sample + DSB1820B_SHIFT) * DSB1820B_SCALE;
                  data[1]=tmp>>8;
                  data[0]=tmp;

                }
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define NUM_SAMPLES 32
DateTime date = DateTime(__DATE__, __TIME__);

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;
DSB1820B dsb0;
DSB1820B dsb1(1);
Counter XBTR_Cntr;

#define NUM_SENSORS 5
Sensor * sensor[] = {&onboardTemp, &batteryGauge, &dsb0, &dsb1, &XBTR_Cntr};
Sensorhub sensorhub(sensor,NUM_SENSORS);

//~~~~~~~~~~~~~~~~~~~~~ BEGIN MAIN CODE ~~~~~~~~~~~~~~~~~~~~~~~~//
void setup(){
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
  Serial.println("Launching program.");
  XBTR_Cntr.setZero();
}

bool firstRun=true;

void loop(){
  //if A1 woke us up and its log time OR if its the first run OR if the button has been pushed
  bool buttonPressed =  !clock.triggeredByA2() && !clock.triggeredByA1() ;
  clock.print();
  if( clock.triggeredByA1() ||  buttonPressed || firstRun){
    Serial.println("Sampling sensors:");
    sensorhub.sample(true);
    clock.setAlarm1Delta(minA1, secA1);
  }
  
  if( ( clock.triggeredByA2() ||  buttonPressed ||firstRun)){
    xbee.enable();
    Serial.println("Creating datapoint from samples");
    sensorhub.log(true);
    #ifdef XBEE_ENABLE
    sendDataPacket(&sensorhub.data[0], sensorhub.getDataSize());
    #endif
    clock.setAlarm2Delta(minA2);
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
        xbee.hardReset();
        delay(100);
      }
    }
    clock.setAlarm2Delta(minA2);
    xbee.hardReset();
    sleep();
    return false;
}

boolean sendDataPacket(uint8_t * arrayPointer, uint8_t arrayLength){
    for(int i=0; i<maxRetries;i++){ 
     XBTR_Cntr.incr(); 
      if( xbee.sendData(arrayPointer, arrayLength) ) {
        XBTR_Cntr.setZero();
        return true;
      }
      if (maxRetries - i == 2){
        xbee.hardReset();
        delay(100);
      }
    }
    xbee.hardReset();
    return false;
}



