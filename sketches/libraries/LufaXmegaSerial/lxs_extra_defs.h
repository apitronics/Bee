/**
 * Project "LufaXmegaSerial", providing USB serial for Xmega as an Arduino library
 * Copyright 2013 Frank Zhao, all rights reserved
 * See "license.txt" for licensing info (MIT license)
*/

// this file acts as command line options for avr-gcc, tricking the Arduino IDE into compiling LUFA correctly

#ifndef _LXS_EXTRA_DEFS_H_
#define _LXS_EXTRA_DEFS_H_

#define USE_LUFA_CONFIG_HEADER
#define BOARD BOARD_A3BU_XPLAINED
#define ARCH ARCH_XMEGA
#define F_USB 48000000UL

#endif