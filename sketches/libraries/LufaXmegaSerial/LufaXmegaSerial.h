/**
 * Project "LufaXmegaSerial", providing USB serial for Xmega as an Arduino library
 * Copyright 2013 Frank Zhao, all rights reserved
 * See "license.txt" for licensing info (MIT license)
*/

#ifndef _LUFAXMEGASERIAL_H_
#define _LUFAXMEGASERIAL_H_

#include <lxs_extra_defs.h>
#include <Stream.h>

class LufaXmegaSerial : public Stream
{
	private:
	public:
		LufaXmegaSerial();
		void begin(), begin(unsigned long x);
		void task();
		void end();
		virtual int available(void);
		virtual int peek(void);
		virtual int read(void);
		virtual void flush(void);
		virtual size_t write(uint8_t);
		inline size_t write(unsigned long n) { return write((uint8_t)n); }
		inline size_t write(long n) { return write((uint8_t)n); }
		inline size_t write(unsigned int n) { return write((uint8_t)n); }
		inline size_t write(int n) { return write((uint8_t)n); }
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool();
};

#ifdef __cplusplus
extern "C" {
#endif
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Platform/Platform.h>
#ifdef __cplusplus
}
#endif

#endif