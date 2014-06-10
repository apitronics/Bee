#include <Sensorhub.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Bee.h>

#include <WeatherPlug.h>
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

#define NUM_SENSORS 9
Sensor * sensor[] = {&onboardTemp, &batteryGauge, &BMP_temp,&BMP_press, &windDir, &windSpeed, &rainfall, &sht_temp, &sht_rh};
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
  Serial.println("Launching program.");
}


uint32_t start;

void loop(){
  xbee.enable();
  clock.print();
  start = millis();
  for(int i=0;i<NUM_SAMPLES;i++) sensorhub.sample();  
  sensorhub.log(true);
  
  Serial.print("sampling took: ");
  Serial.print(millis()-start);
  Serial.println("ms");
  
  //send data packet until reception is acknowledged
  while(!xbee.sendData(&sensorhub.data[0], sensorhub.getDataSize()));
  
  clock.setAlarm1Delta(5, 0);
  weatherPlug.sleep();
  xbee.disable();
  sleep();
}



