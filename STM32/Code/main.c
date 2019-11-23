#include <string.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/systick.h>
#include "console.h"
#include "settings.h"
#include "main.h"
#include "ds18x20.h"
#include "max6674.h"
#include "webserver.h"
#include "lcd.h"

serial_port_t sp_USART2;

struct ds1820Measurement_t ds1820Values[3];
settings_t settings;
char buf[50];
char webStr[30];

int main(void)
{
	uint32_t data;
	uint32_t counter=0;

	rcc_clock_setup_in_hse_16mhz_out_72mhz();

	//Enable IO ports A,B,C
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);

	//Disable debug port, we use those pins.
	data = AFIO_MAPR;
	data &= ~AFIO_MAPR_SWJ_MASK;
	data |= AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF;
	AFIO_MAPR = data;

	//Enable SPI1 and SPI2
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_SPI1EN);
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);

	//Enable systick. Needed by delay functions.
	systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8); // 72MHz / 8 => 9000000 counts per second
	systick_counter_enable();
	systick_set_reload(0x00FFFFFF);

	//Setup console
	console_Setup(19200);
	console_SendString("System started\r\n");
	
	//Copy settings from flash
	memcpy(&settings,(void *)SETTINGS_ADDRESS,sizeof(settings_t));

	//Wait litle to make sure LCD is ok.
	delay_ms(500);

	LCDInit();
	LCDClear();

	//Initialize Max6674
	Max6674Init();
	Max6674SetCalibration(settings.Max6674Divider,settings.Max6674Offset);

	//Initialize webserver
	webserverInit();

	gpio_set_mode(GPIOC,GPIO_MODE_INPUT ,GPIO_CNF_INPUT_FLOAT, GPIO7);
	gpio_set_mode(GPIOC,GPIO_MODE_INPUT ,GPIO_CNF_INPUT_FLOAT, GPIO9);

//	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_50_MHZ ,GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
//	gpio_set(GPIOB,GPIO1);

	//Setup struct for DS1820 measurements
	memset(ds1820Values,0,sizeof(ds1820Values));
	memcpy(ds1820Values[0].id,settings.ids[0].id,sizeof(((OWID_t*)0)->id));
	memcpy(ds1820Values[1].id,settings.ids[1].id,sizeof(((OWID_t*)0)->id));
	memcpy(ds1820Values[2].id,settings.ids[2].id,sizeof(((OWID_t*)0)->id));

	unsigned char dot=0;

	int sw = 0;

	while(1)
	{
		counter++;
		if(counter>30000)
		{
			int i;
			int val;

			if(!(GPIOC_IDR & (1<<7)))
				sw=1;

			if(!(GPIOC_IDR & (1<<9)))
				sw=2;

			counter=0;

			DS18X20ReadSensors(ds1820Values,3);
			LCDClear();
			LCDGoto(0);
			for(i=0;i<3;i++)
			{
				if(ds1820Values[i].celcius < 100 && ds1820Values[i].celcius > 0)
				{
					IntToDec(buf,ds1820Values[i].celcius,2);
					buf[2]=0;
					LCDPrint(buf);
					IntToHex(webStr+i*2,ds1820Values[i].celcius,2);
				}
				else
				{
					IntToHex(webStr+i*2,0,2);
					LCDPrint("--");
				}

				LCDPrint(" ");
			}

			val=Max6674Read();
			IntToDec(buf,val,4);
			buf[4]=0;
			LCDPrint(buf);

			IntToHex(webStr+6,val,4);
			IntToHex(webStr+10,sw,2);

			webStr[12]=0;

			dot++;
			if(sw==1)
				LCDPrint(" A");

			if(sw==2)
				LCDPrint(" T");

			switch(dot%4)
			{
				case 0:
					LCDPrint("-");
					break;
				case 1:
					LCDPrint("+");
					break;
				case 2:
					LCDPrint("*");
					break;
				case 3:
					LCDPrint("+");
					break;
			}
	
		}
		console_Poll();
		webserverPoll();
	}
}

