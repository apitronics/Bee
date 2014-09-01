/**
 * Project "LufaXmegaSerial", providing USB serial for Xmega as an Arduino library
 * Copyright 2013 Frank Zhao, all rights reserved
 * See "license.txt" for licensing info (MIT license)
*/

#include <avr/io.h>
#include <LufaXmegaSerial.h>
#include <LXS_Descriptors.h>

static RingBuffer_t usb_tx_buffer;
static RingBuffer_t usb_rx_buffer;

volatile uint8_t sof_count_down;

LufaXmegaSerial::LufaXmegaSerial()
{
	// empty constructor
}

void LufaXmegaSerial::begin(unsigned long x)
{
	// baud rate ignored
	begin();
}

void LufaXmegaSerial::begin()
{
	USB_Detach();

	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC32MHZ, 32000000 / 4, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StopInternalOscillator(CLOCK_SRC_INT_RC2MHZ);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
	GlobalInterruptEnable();

	sof_count_down = 0;
	RingBuffer_InitBuffer(&usb_tx_buffer, (uint8_t*)malloc(CDC_TXRX_EPSIZE / 2), CDC_TXRX_EPSIZE / 2);
	RingBuffer_InitBuffer(&usb_rx_buffer, (uint8_t*)malloc(CDC_TXRX_EPSIZE / 2), CDC_TXRX_EPSIZE / 2);

	USB_Init();
	USB_Device_EnableSOFEvents();

	task();
}

void LufaXmegaSerial::end()
{
	USB_Detach();
	free(usb_tx_buffer.Start);
	free(usb_rx_buffer.Start);
}

void LufaXmegaSerial::task()
{
	USB_USBTask();

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	if (sof_count_down <= 0 || RingBuffer_IsFull(&usb_tx_buffer))
	{
		sof_count_down = 100;
		Endpoint_SelectEndpoint(CDC_TX_EPADDR);
		if (Endpoint_WaitUntilReady() == ENDPOINT_READYWAIT_NoError)
		{
			while (RingBuffer_GetCount(&usb_tx_buffer) > 0) {
				Endpoint_Write_8(RingBuffer_Remove(&usb_tx_buffer));
			}
			Endpoint_ClearIN();
		}
	}

	Endpoint_SelectEndpoint(CDC_RX_EPADDR);
	if (Endpoint_IsOUTReceived())
	{
		while (Endpoint_BytesInEndpoint() > 0)
		{
			if (RingBuffer_IsFull(&usb_rx_buffer) == false) {
				RingBuffer_Insert(&usb_rx_buffer, Endpoint_Read_8());
			}
		}
		Endpoint_ClearOUT();
	}
}

size_t LufaXmegaSerial::write(uint8_t x)
{
	task();
	size_t st = 0;
	sof_count_down = 100;
	if (RingBuffer_IsFull(&usb_tx_buffer) == false) {
		st = 1;
		RingBuffer_Insert(&usb_tx_buffer, x);
	}
	task();
	return st;
}

int LufaXmegaSerial::available(void)
{
	task();
	return RingBuffer_GetCount(&usb_rx_buffer);
}

int LufaXmegaSerial::peek(void)
{
	task();
	return RingBuffer_Peek(&usb_rx_buffer);
}

int LufaXmegaSerial::read(void)
{
	task();
	return RingBuffer_Remove(&usb_rx_buffer);
}

void LufaXmegaSerial::flush(void)
{
	task();
	usb_rx_buffer.In = usb_rx_buffer.Start;
	usb_rx_buffer.Out = usb_rx_buffer.Start;
	usb_rx_buffer.Count = 0;
}

LufaXmegaSerial::operator bool() {
	task();
	return true;
}