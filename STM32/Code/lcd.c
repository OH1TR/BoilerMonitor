#include <libopencm3/stm32/f1/gpio.h>
#include "lcd.h"
#include "delay.h"

#define DB0PORT GPIOC
#define DB1PORT GPIOC
#define DB2PORT GPIOB
#define DB3PORT GPIOB
#define DB4PORT GPIOB
#define DB5PORT GPIOB
#define DB6PORT GPIOB
#define DB7PORT GPIOB

#define DB0PIN GPIO11
#define DB1PIN GPIO12
#define DB2PIN GPIO5
#define DB3PIN GPIO4
#define DB4PIN GPIO6
#define DB5PIN GPIO7
#define DB6PIN GPIO8
#define DB7PIN GPIO9

#define WRPORT GPIOA
#define WRPIN GPIO15

#define RSPORT GPIOA
#define RSPIN GPIO14

#define EPORT GPIOC
#define EPIN GPIO10


void TriStateDataBus(int tris)
{
	uint8_t mode= tris ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_50_MHZ;
	uint8_t cnf= tris ? GPIO_CNF_INPUT_FLOAT : GPIO_CNF_OUTPUT_PUSHPULL;

	gpio_set_mode(DB0PORT,mode ,cnf, DB0PIN); // LCD DB0
	gpio_set_mode(DB1PORT,mode ,cnf, DB1PIN); // LCD DB1
	gpio_set_mode(DB2PORT,mode ,cnf, DB2PIN); // LCD DB2
	gpio_set_mode(DB3PORT,mode ,cnf, DB3PIN); // LCD DB3
	gpio_set_mode(DB4PORT,mode ,cnf, DB4PIN); // LCD DB4
	gpio_set_mode(DB5PORT,mode ,cnf, DB5PIN); // LCD DB5
	gpio_set_mode(DB6PORT,mode ,cnf, DB6PIN); // LCD DB6
	gpio_set_mode(DB7PORT,mode ,cnf, DB7PIN); // LCD DB7


}
void ClockLCDCommand(unsigned char data,unsigned char rs)
{
	gpio_clear(RSPORT,RSPIN);

	gpio_clear(DB0PORT,DB0PIN);
	gpio_clear(DB1PORT,DB1PIN);
	gpio_clear(DB2PORT,DB2PIN);
	gpio_clear(DB3PORT,DB3PIN);
	gpio_clear(DB4PORT,DB4PIN);
	gpio_clear(DB5PORT,DB5PIN);
	gpio_clear(DB6PORT,DB6PIN);
	gpio_clear(DB7PORT,DB7PIN);

	gpio_clear(EPORT,EPIN);
	gpio_clear(WRPORT,WRPIN);

	if(rs)
		gpio_set(RSPORT,RSPIN);

	if(data & 1)
		gpio_set(DB0PORT,DB0PIN);

	if(data & 2)
		gpio_set(DB1PORT,DB1PIN);

	if(data & 4)
		gpio_set(DB2PORT,DB2PIN);

	if(data & 8)
		gpio_set(DB3PORT,DB3PIN);

	if(data & 16)
		gpio_set(DB4PORT,DB4PIN);

	if(data & 32)
		gpio_set(DB5PORT,DB5PIN);

	if(data & 64)
		gpio_set(DB6PORT,DB6PIN);

	if(data & 128)
		gpio_set(DB7PORT,DB7PIN);


	delay_us(2); // 40ns min

	gpio_set(EPORT,EPIN); //Write cycle

	delay_us(2); // 230ns min

	gpio_clear(EPORT,EPIN);
}

void WaitBusy()
{
	int i;

	TriStateDataBus(1);

	gpio_clear(RSPORT,RSPIN);
	gpio_set(WRPORT,WRPIN);
	delay_us(1); // 40ns min
	gpio_set(EPORT,EPIN);

	for (i = 0; i < 1500000; i++)
	{
		uint16_t d = gpio_port_read(DB7PORT);
		if(!(d & (DB7PIN)))
			break;
	}

	gpio_clear(EPORT,EPIN);
	gpio_clear(WRPORT,WRPIN);

	TriStateDataBus(0);
}

void LCDInit()
{
	gpio_set_mode(WRPORT,GPIO_MODE_OUTPUT_50_MHZ ,GPIO_CNF_OUTPUT_PUSHPULL, WRPIN);
	gpio_set_mode(RSPORT,GPIO_MODE_OUTPUT_50_MHZ ,GPIO_CNF_OUTPUT_PUSHPULL, RSPIN);
	gpio_set_mode(EPORT,GPIO_MODE_OUTPUT_50_MHZ ,GPIO_CNF_OUTPUT_PUSHPULL, EPIN);


	TriStateDataBus(0);

	delay_ms(20); //15ms min

	ClockLCDCommand(0x30,0);

	delay_ms(10); //4.1ms min

	ClockLCDCommand(0x30,0);

	delay_ms(1); //100us min

	ClockLCDCommand(0x30,0);

	delay_ms(1); //100us min

	ClockLCDCommand(0x38,0);

	delay_ms(1); //100us min

	ClockLCDCommand(0x0e,0);

	delay_ms(1); //100us min

	ClockLCDCommand(0x06,0);

	delay_ms(1); //100us min

	ClockLCDCommand(0x01,0);

	delay_ms(1); //100us min

}

void LCDClear()
{
	ClockLCDCommand(0x01,0);
}

void LCDPrint(char *str)
{
	int i=0;
	while(str[i]!=0)
	{
		ClockLCDCommand(str[i],1);
		WaitBusy();
		i++;
	}
}

void LCDGoto(int pos)
{
	ClockLCDCommand(0x80 | (pos & 0x7f),0);
	WaitBusy();
}

// Debug function do not call with LCD connected
void LCDAllOn()
{
	gpio_set(EPORT,EPIN);
	gpio_set(WRPORT,WRPIN);
	gpio_set(RSPORT,RSPIN);

	gpio_set(DB0PORT,DB0PIN);
	gpio_set(DB1PORT,DB1PIN);
	gpio_set(DB2PORT,DB2PIN);
	gpio_set(DB3PORT,DB3PIN);
	gpio_set(DB4PORT,DB4PIN);
	gpio_set(DB5PORT,DB5PIN);
	gpio_set(DB6PORT,DB6PIN);
	gpio_set(DB7PORT,DB7PIN);
}


