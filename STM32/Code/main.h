#ifndef _MAIN_H
#define _MAIN_H

#include "serial.h"
#include "ds18x20.h"

typedef struct
{
	uint8_t id[OW_ROMCODE_SIZE];
} OWID_t;

typedef struct
{
	OWID_t ids[3];
	uint16_t Max6674Divider;
	uint16_t Max6674Offset;
	uint8_t ip[4];
} settings_t;

extern settings_t settings;
extern serial_port_t sp_USART2;
extern struct ds1820Measurement_t ds1820Values[3];
extern char webStr[30];

void glue_handle_rx_data_from_usb(char *buf,int len,uint8_t ep);
void glue_poll_usart_and_send_data_to_usb();
void glue_set_line_coding_cb(uint32_t dwDTERate,uint8_t bDataBits,uint32_t bParityType);
void ProcessCommandChannelData(char *buf,int len);


#endif

