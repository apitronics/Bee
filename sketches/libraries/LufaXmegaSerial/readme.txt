Written by Frank Zhao, starting on Aug 17 2013

At the time of this writing, none of the Arduino "core" available supports USB virtual serial ports on the Xmega architecture

This library uses LUFA to make a USB virtual serial class that operates like HardwareSerial

A lot of tricks have been used to make this code compile under Arduino IDE. Most of the issues came from the fact that you can't change any of the command line options when executing avr-gcc, and how Arduino IDE does not search the library directory deeply for C files to compile

Install this library like any other Arduino library, http://arduino.cc/en/Guide/Libraries

The "task()" function should be called often, preferably once every 10 ms minimum