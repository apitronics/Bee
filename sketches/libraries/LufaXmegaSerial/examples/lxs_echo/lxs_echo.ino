/**
 * Project "LufaXmegaSerial", providing USB serial for Xmega as an Arduino library
 * Copyright 2013 Frank Zhao, all rights reserved
 * See "license.txt" for licensing info (MIT license)
*/

#include <LufaXmegaSerial.h>

LufaXmegaSerial USBSerial;

void setup()
{
	// the baud rate is actually meaningless since USB virtual serial ports don't care
	USBSerial.begin(9600);
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
}