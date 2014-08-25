//Apitronics - DS1820B.ino
//Aug 25

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

const byte minA1 = 0;
const byte secA1 = 15;
const byte minA2 = 1;

//Create Sensorhub Sensor from DallasTemp library
class DSB1820B: public Sensor
{
        public:
                OneWire oneWire;
                DallasTemperature dsb = DallasTemperature(&oneWire);
                uint8_t _index;
                                
                DSB1820B(uint8_t index=0, uint8_t samplePeriod=1):Sensor(DSB1820B_UUID, DSB1820B_LENGTH_OF_DATA, DSB1820B_SCALE, DSB1820B_SHIFT, true, samplePeriod){
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


#define NUM_SAMPLES 32
DateTime date = DateTime(__DATE__, __TIME__);

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;
DSB1820B dsb0;
DSB1820B dsb1(1);

#define NUM_SENSORS 4
Sensor * sensor[] = {&onboardTemp, &batteryGauge, &dsb0, &dsb1};
Sensorhub sensorhub(sensor,NUM_SENSORS);

void setup(){
  delay(1000);
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  Serial.begin(57600);
  xbee.begin(9600);
  Serial.print("Initialized: serial");
  
  clock.begin(date);
  configureSleep();
  Serial.print(", clock");
  
  sensorhub.init();
  Serial.println(", sensors");
  
  #ifdef XBEE_ENABLE
  Serial.print("Connecting to Hive...");
  //send IDs packet until reception is acknowledged'
  
  while(!sendPacket(&sensorhub.ids[0],UUID_WIDTH*NUM_SENSORS));  
  
  Serial.print(" Hive received packet...");
  //wait for message from Coordinator
  
 
  xbee.refresh();
  while(!xbee.available()){
    xbee.refresh();
  }
  xbee.meetCoordinator();
  
  Serial.println(" Hive address saved.");
  #endif
  Serial.println("Launching program.");
  
}

bool sendPacket(uint8_t * pointer, uint8_t length){

    for(int i=0; i<3;i++){
      if( xbee.sendIDs(pointer, length) ) {
        return true;
      }
    }
    clock.setAlarm2Delta(5);
    sleep();
    return false;
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
    while(!xbee.sendData(&sensorhub.data[0], sensorhub.getDataSize()));
    #endif
    clock.setAlarm2Delta(minA2);
  }
  firstRun=false;
 
  #ifdef XBEE_ENABLE
  xbee.disable();
  #endif
  sleep();  
}

