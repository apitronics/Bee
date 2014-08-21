#include <Wire.h>
#include <Sensorhub.h>
#include <Onboard.h>

void setup(){
  
  Serial.begin(57600);
  
  onboard.setupTempSense();
  onboard.setupBattSense();
 
}


void loop(){
  Serial.print(onboard.getBatt());
  Serial.println("V");
  Serial.print(onboard.getTemp());
  Serial.println("*C");
  Serial.println();
  
  delay(1000);
}
