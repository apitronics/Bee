#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Clock.h>

DateTime date = DateTime(__DATE__, __TIME__);

const int A1_minutes_freq = 0;
const int A1_seconds_freq = 5;
const int A2_minutes_freq = 1;

void setup(){
  
  Serial.begin(57600);
  
  //sets the time from date time object above iff oscillator stop flag is high
  clock.begin(date);
 
  clock.setAlarm1Delta(A1_minutes_freq,A1_seconds_freq);
  clock.setAlarm2Delta(A2_minutes_freq);
}


void loop(){
  //reads CTRL register to see if any flags have been raised
  clock.getFlags();
  
  if(clock.triggeredByA1() ){
    Serial.println("A1 triggered");
    clock.setAlarm1Delta(A1_minutes_freq,A1_seconds_freq);
  }
  
  if(clock.triggeredByA2() ){
    Serial.println("A2 triggered");
    clock.setAlarm2Delta(A2_minutes_freq);
  }
  
  
}
