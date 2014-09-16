/*
 * BeeDevice.h
 *
 * Created: 9/16/2014
 *  Author: Colin Dignam
 *
 * set sample period, log period, sleep (bool)
 * this is the predecessor to the soon-to-be Device class
 */


#ifndef BeeDevice_H
#define BeeDevice_H

#include <inttypes.h>

 class BeeDevice
 {
 public:
   BeeDevice(const uint16_t UUID = 0x8000, uint16_t samplerate = 30, uint16_t lograte = 1, uint8_t beeCTRL = 0x01)
    {
      _UUID = UUID;
      _sampl_sec = samplerate;
      _log_min = lograte;
      _beeCTRL = beeCTRL;
    }
   ~BeeDevice(){}

   uint8_t getSampleMin()
   {
    return (uint8_t)(_sampl_sec / 60);
   }
   uint8_t getSampleSecR()
   {
    return (uint8_t)(_sampl_sec % 60);
   }
   bool isSleepEnable()
   {
    return (bool)(_beeCTRL & 0x01);
   }
   uint8_t

  private:
    uint16_t _UUID;
    uint16_t _sampl_sec;
    uint16_t _log_min;
    uint8_t _beeCTRL;



 };












#endif