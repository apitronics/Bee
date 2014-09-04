/**
 * Project "LufaXmegaSerial", providing USB serial for Xmega as an Arduino library
 * Copyright 2013 Frank Zhao, all rights reserved
 * See "license.txt" for licensing info (MIT license)
*/

#include <LufaXmegaSerial.h>

LufaXmegaSerial USBSerial;

int i = 0;

void setup()
{
	// the baud rate is actually meaningless since USB virtual serial ports don't care
	PORTC.DIRSET = PIN7_bm; //D13 as output
  	PORTC.OUTSET = PIN7_bm; //turn on LED
  	delay(1000);
	USBSerial.begin(57600);
	USBSerial.write("hello\r\n");

}

void loop()
{
	// just echos the characters you type into terminal back at you
	while (USBSerial.available() > 0)
	{
		USBSerial.write(USBSerial.read());
	}

	USBSerial.task(); // call this often
	// note: all of the other functions calls "task()" internally as well, so if you always call "available()", you are already calling "task()"
	i++;
	if(i >= 30000){
		i = 0;
		PORTC.OUTCLR = PIN7_bm; //turn on LED
	  	delay(1000);
  	  	PORTC.OUTSET = PIN7_bm; //turn on LED
		USBSerial.write("running...\r\n");
	}
}