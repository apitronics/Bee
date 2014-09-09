#include <AnalogSensor.h>


#include <Sensorhub.h>

#define ANALOG_UUID 0x0019
#define ANALOG_LENGTH_OF_DATA 2
#define ANALOG_SCALE 1000
#define ANALOG_SHIFT 0


  AnalogSensor PA1Sense = AnalogSensor(1, ANALOG_UUID, ANALOG_LENGTH_OF_DATA, ANALOG_SCALE, ANALOG_SHIFT);
  AnalogSensor PA2Sense = AnalogSensor(2, ANALOG_UUID, ANALOG_LENGTH_OF_DATA, ANALOG_SCALE, ANALOG_SHIFT);
  AnalogSensor PA3Sense = AnalogSensor(3, ANALOG_UUID, ANALOG_LENGTH_OF_DATA, ANALOG_SCALE, ANALOG_SHIFT);
  AnalogSensor PA4Sense = AnalogSensor(4, ANALOG_UUID, ANALOG_LENGTH_OF_DATA, ANALOG_SCALE, ANALOG_SHIFT);
  AnalogSensor PA5Sense = AnalogSensor(5, ANALOG_UUID, ANALOG_LENGTH_OF_DATA, ANALOG_SCALE, ANALOG_SHIFT);
  AnalogSensor PA6Sense = AnalogSensor(6, ANALOG_UUID, ANALOG_LENGTH_OF_DATA, ANALOG_SCALE, ANALOG_SHIFT);


void setup() {
  Serial.begin(57600);
  Serial.println("starting ");
  
  PA1Sense.init();
  PA2Sense.init();  //redundant
  PA3Sense.init();
  PA4Sense.init();
  PA5Sense.init();
  PA6Sense.init();
 
  
}

void loop() {
    delay(2000);
    Serial.println("=======================================================");
    PA1Sense.readSensorADC();
    PA2Sense.readSensorADC();
    PA3Sense.readSensorADC();
    PA4Sense.readSensorADC();
    PA5Sense.readSensorADC();
    PA6Sense.readSensorADC();
    Serial.println();
    PA1Sense.getData();
    PA2Sense.getData();
    PA3Sense.getData();
    PA4Sense.getData();
    PA5Sense.getData();
    PA6Sense.getData();

  
}
