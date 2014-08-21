#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Onboard.h>
#include <Clock.h>
#include <Bee.h>

DateTime date = DateTime(__DATE__, __TIME__);

void setup(){
  Serial.begin(57600);
  
  Serial.println("Starting...");
  
  configureSleep();
  
  clock.begin(date); //for some reason this is necessary, otherwise button isn't pressed
  
}

void loop(){
  
  sleep();
  
  Serial.println("Button pressed!");
}

