
#include <LufaXmegaSerial.h>
#include <avr/wdt.h>

LufaXmegaSerial USBSerial;

//USB's VCC is brought down to 3.3V and connected to this
//pin (PF5).  0b0010 0000 = 0x32
int PIN_TO_DETECT_CONNECTED_USB = 0x32;


void setup() {
	USBSerial.begin(9600);
        PORTC.DIR= (1 << 7);  //These two lines turn on a 
        PORTC.OUTSET = (1<<PIN7);  //debugging LED on digital pin 7
        
}

void check_for_serial_communication(void) {
  
  USBSerial.task(); // call this often
  
  int received_command = 0;
  while (USBSerial.available() > 0) {
    received_command = USBSerial.read();
    if (received_command == 'P') {  //If a specific cmd is sent, turn off.
      USBSerial.write("shutting down");
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
  
  if (PORTF.IN & PIN_TO_DETECT_CONNECTED_USB) {  //If the Bee detects that the USB cable is plugged in. 
    check_for_serial_communication();            //then check if there's serial data coming in.
  }
}
