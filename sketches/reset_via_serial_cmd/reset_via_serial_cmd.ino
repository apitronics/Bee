
#include <LufaXmegaSerial.h>
#include <avr/wdt.h>

LufaXmegaSerial USBSerial;

//USB's VCC is brought down to 3.3V and connected to this
//pin (PF5).  0b0010 0000 = 0x32
int PIN_TO_DETECT_CONNECTED_USB = 0x32;

void setup() {
  // Serial.begin(57600);
  // Serial.println("starting");
  PORTC.DIRSET = PIN7_bm; //D13 as output
  PORTC.OUTSET = PIN7_bm; //turn on LED
  delay(1000);
	USBSerial.begin(9600);

        
}

void check_for_serial_communication(void) {
  
  USBSerial.task(); // call this often
  // Serial.println("called serial check");
  int received_command = 0;
  while (USBSerial.available() > 0) {
    received_command = USBSerial.read();
    if (received_command == 'P') {  //If a specific cmd is sent, turn off.
      USBSerial.write("shutting down");
      // Serial.println("shutting down mcu");
      delay(500);
      USBSerial.write("\r");
      USBSerial.write("\n"); 
      delay(500);
      
      USBSerial.end();  //End the serial connection
      delay(500);
      wdt_enable(WDTO_250MS); //Enable the watchdog timer.
      for(;;);  //loop until the watchdog timer resets the MCU.
    }
  }
  return ;
  
}

void loop()
{
  USBSerial.task(); // call this often
  delay(1000);
  Serial.print(".");

  if (PORTF.IN & PIN_TO_DETECT_CONNECTED_USB) {  //If the Bee detects that the USB cable is plugged in. 
    PORTC.OUTCLR = PIN7_bm; //turn off LED
    check_for_serial_communication();            //then check if there's serial data coming in.

  }
  delay(1);
}
