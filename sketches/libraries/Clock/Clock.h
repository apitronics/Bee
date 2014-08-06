#ifndef Clock_h
#define Clock_h


#include <stdint.h>
#include "Wire.h"
#include "RTClib.h"

#define DS1339_I2C_ADDRESS 0x68 // This is the I2C address
#define ALARM_1_REG 0x7 // address of Alarm1 byte register
#define ALARM_2_REG 0xB
#define CONTROL_REG 0xE // address of Control byte register
#define STATUS_REG 0xF // address of Status byte register
#define OSCILLATOR_STOP_FLAG 7 // bit of status register where OSF is

#define I2C_BEGIN xmWireE.begin

#define I2C_BEGIN_TRANSMIT xmWireE.beginTransmission
#define I2C_WRITE xmWireE.write
#define I2C_REQUEST xmWireE.requestFrom
#define I2C_READ xmWireE.receive

#define I2C_READY xmWireE.ready

#define I2C_END_TRANSMIT xmWireE.endTransmission

class Clock
{
public:
Clock();
void begin(DateTime);
void enableAlarm1();
void enableAlarm2();
void setAlarm1Delta(uint8_t minutes, uint8_t seconds);
void setAlarm2Delta(uint8_t minutes);
boolean alarmFlag();
boolean triggeredByA1();
boolean triggeredByA2();
void getFlags();
void clearFlags();
void getDate();
void print();
void setDate(DateTime);
void setDate(String date);
void configureInterrupt();
void sleep();
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
byte test;
void I2Cdisable();
void I2Cenable();
String timestamp();

private:
byte _flags;
void _clearStatusRegister();
void _print();
bool _initialized = false;
byte decToBcd(byte);
byte bcdToDec(byte);
bool _semaphore;
void _writeDateToEEPROM();
};

extern Clock clock;
#endif



