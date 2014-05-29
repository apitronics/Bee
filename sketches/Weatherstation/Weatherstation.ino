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
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  Serial.begin(57600);
  xbee.begin(9600);
  delay(1000);
  
  clock.begin(date);
  configureSleep();
  
  Serial.println("IDs:");
  Serial.print("[");
  for(int i=0; i<UUID_WIDTH*NUM_SENSORS;i++){
    Serial.print(sensorhub.ids[i]);
    Serial.print(", ");
  }
  
  Serial.print("]");
  Serial.println();
  
  Serial.println("starting");
  sensorhub.init();
  
  xbee.sendIDs(&sensorhub.ids[0], UUID_WIDTH*NUM_SENSORS);
  
}


uint32_t start;

void loop(){
  
  clock.print();
  start = millis();
  for(int i=0;i<NUM_SAMPLES;i++) sensorhub.sample();  
  sensorhub.log();
  
  
  Serial.print("sampling took: ");
  Serial.print(millis()-start);
  Serial.println("ms");
  Serial.println("Data packet:" );
  Serial.print("[");
  for(int i=0; i<sensorhub.getDataSize();i++){
    Serial.print(sensorhub.data[i]);
    Serial.print(", ");
  }
  Serial.print("]");
  
  Serial.println();
  
  xbee.sendData(&sensorhub.data[0], sensorhub.getDataSize());
  
  
  clock.setAlarm1Delta(0, 15);
  weatherPlug.sleep();
  sleep();
}



