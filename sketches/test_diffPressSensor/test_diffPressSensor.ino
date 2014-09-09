#include <DiffPressureSensor.h>


#include <Sensorhub.h>
#define DEBUG

  DiffPressureSensor PA1Sense = DiffPressureSensor(1);
  DiffPressureSensor PA2Sense = DiffPressureSensor(2);
  DiffPressureSensor PA3Sense = DiffPressureSensor(3);
  DiffPressureSensor PA4Sense = DiffPressureSensor(4);
  DiffPressureSensor PA5Sense = DiffPressureSensor(5);
  DiffPressureSensor PA6Sense = DiffPressureSensor(6);


void setup() {
  Serial.begin(57600);
  Serial.println("starting ");
  
  PA1Sense.init();
  
}

void loop() {
    delay(1000);
    Serial.println("=======================================================");
    PA1Sense.readSensorADC();
    PA2Sense.readSensorADC();
    PA3Sense.readSensorADC();
    PA4Sense.readSensorADC();
    PA5Sense.readSensorADC();
    PA6Sense.readSensorADC();

  
}
