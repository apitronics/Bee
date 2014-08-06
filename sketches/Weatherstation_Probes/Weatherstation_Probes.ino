#include <Clock.h>
#include <Onboard.h>
#include <XBeePlus.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Bee.h>
#include <WeatherPlug.h>

//#define XBEE_ENABLE

#define EC5_HUMIDITY_UUID 0x17
#define EC5_LENGTH_OF_DATA 2
#define EC5_TEMP_SCALE 10
#define EC5_TEMP_SHIFT 0

class EC5_SoilHumidity: public Sensor
{
   public:
      EC5_SoilHumidity(uint8_t channel, uint8_t samplePeriod=1):Sensor(EC5_HUMIDITY_UUID, EC5_LENGTH_OF_DATA, EC5_TEMP_SCALE, EC5_TEMP_SHIFT, false,samplePeriod){
        _channel = channel;
      };
        String getName() { return "EC-5 Soil Humidity";}
        String getUnits() {return "mV"; }
        void init() {weatherPlug.init();}
        void getData() { 
          uint16_t sample = weatherPlug._getADC(_channel)/2.0*1.0071108127079073;
          data[1] = sample >> 8;
          data[0] = sample;
        }
        uint16_t _channel;
};

#include <OneWire.h>
#include <DallasTemperature.h>

#define DSB1820B_UUID 0x0018
#define DSB1820B_LENGTH_OF_DATA 2
#define DSB1820B_SCALE 100
#define DSB1820B_SHIFT 50

//Create Sensorhub Sensor from DallasTemp library
class DSB1820B: public Sensor
{
        public:
                OneWire oneWire;
                DallasTemperature dsb = DallasTemperature(&oneWire);
                
                DSB1820B(uint8_t samplePeriod=1):Sensor(DSB1820B_UUID, DSB1820B_LENGTH_OF_DATA, DSB1820B_SCALE, DSB1820B_SHIFT, true, samplePeriod){};
                String getName(){ return "Temperature Probe"; }
                String getUnits(){ return "C"; }
                void init(){
                  weatherPlug.init();
                }
                void getData(){
                  
                  weatherPlug.i2cChannel(2);
                  weatherPlug.disableI2C();
                  dsb.begin();
                  
                  dsb.requestTemperatures();
                  float sample = dsb.getTempCByIndex(4);
                  uint16_t tmp = (sample + DSB1820B_SHIFT) * DSB1820B_SCALE;
                  data[1]=tmp>>8;
                  data[0]=tmp;
                  weatherPlug.enableI2C();
                  weatherPlug.i2cChannel(1);
                }

};


#define NUM_SAMPLES 32
DateTime date = DateTime(__DATE__, __TIME__);

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;
BMP_Temperature BMP_temp;
BMP_Pressure BMP_press;
WindDirection windDir;
WindSpeed windSpeed;
Rainfall rainfall;
SHT2x_temp sht_temp;
SHT2x_RH sht_rh;
EC5_SoilHumidity ec5(0);  //channel 0
DSB1820B dsb;

#define NUM_SENSORS 11
Sensor * sensor[] = {&onboardTemp, &batteryGauge, &BMP_temp,&BMP_press, &windDir, &windSpeed, &rainfall, &sht_temp, &sht_rh, &ec5, &dsb};
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
  //send IDs packet until reception is acknowledged
  while(!xbee.sendIDs(&sensorhub.ids[0], UUID_WIDTH*NUM_SENSORS));
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


bool firstRun=true;

void loop(){
  //if A1 woke us up and its log time OR if its the first run OR if the button has been pushed
  bool buttonPressed =  !clock.triggeredByA2() && !clock.triggeredByA1() ;
  clock.print();
  if( clock.triggeredByA1() ||  buttonPressed || firstRun){
    Serial.print("Sampling sensors");
    sensorhub.sample(true);
    clock.setAlarm1Delta(0,15);
  }
  
  if( ( clock.triggeredByA2() ||  buttonPressed ||firstRun)){
    xbee.enable();
    Serial.println("Creating datapoint from samples");
    sensorhub.log(true);
    #ifdef XBEE_ENABLE
    while(!xbee.sendData(&sensorhub.data[0], sensorhub.getDataSize()));
    #endif
    clock.setAlarm2Delta(10);
  }
  firstRun=false;
  weatherPlug.sleep();
  #ifdef XBEE_ENABLE
  xbee.disable();
  #endif
  sleep();  
}

