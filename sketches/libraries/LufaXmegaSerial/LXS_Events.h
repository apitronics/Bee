/**
 * Project "LufaXmegaSerial", providing USB serial for Xmega as an Arduino library
 * originally adapted from LUFA's VirtualSerial demo
 * modified to work as an Arduino library
 * modification performed by Frank Zhao
 * Copyright 2013 Frank Zhao, all rights reserved
 * See "license.txt" for licensing info (MIT license)
*/

#ifndef _LXS_EVENTS_H_
#define _LXS_EVENTS_H_

#include <lxs_extra_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <LUFA/Drivers/USB/USB.h>

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);

#ifdef __cplusplus
}
#endif

#endif