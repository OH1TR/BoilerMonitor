#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/f1/flash.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/spi.h>
#include "enc28j60.h"
#include "console.h"
#include "serial.h"
#include "main.h"
#include "ds18x20.h"
#include "onewire.h"
#include "crc8.h"
#include "delay.h"
#include "settings.h"
#include "flash.h"

char command[100];
unsigned int cmdPos=0;

int echo=0;

serial_port_t sp_console;

void console_Setup(uint16_t bps)
{
	serial_Setup(&sp_console,USART1,bps,USART_FLOWCONTROL_NONE,8,USART_PARITY_NONE);
}

void IntToHex(char *buffer,uint32_t val,int digits)
{
        int i;
        int tmp;

        for(i=0;i<digits;i++)
        {
                tmp=val & 0x0f;
                if(tmp<10)
                        tmp+='0';
                else
                        tmp+='A'-10;

                buffer[digits-i-1]=(char)tmp;
                val=val>>4;
        }
}

void IntToDec(char *buffer,uint32_t val,int digits)
{
	int i;
	unsigned char tmp;

	for(i=digits-1 ; i>=0 ; i--)
	{
		tmp=val % 10;
        	buffer[i]='0'+tmp;
        	val=val / 10;
	}
}

void console_SendHex(uint32_t val,int digits)
{
	char SendHexBuf[10];
	IntToHex(SendHexBuf,val,digits);
	SendHexBuf[digits]=0;
	console_SendString(SendHexBuf);
}


int StrBeginsWith(char *begin,char *str)
{
	int i=0;
	while(begin[i]!=0)
	{
		if(begin[i]!=str[i])
			return(0);
		i++;
	}
	return(1);
}


void console_SendString(char *msg)
{
	unsigned int i = 0;
	while(msg[i]!=0)
	{
		serial_SendCh(&sp_console,msg[i]);
		i++;
	}
}

uint32_t ParseHex(char *str,int count,int *ok)
{
	int i;
	uint32_t retval=0;
	uint8_t part;

	for(i=0;i<count;i++)
	{
		part=255;
		if(str[i]>='0' && str[i]<='9')
			part=str[i]-'0';

		if(str[i]>='A' && str[i]<='F')
			part=str[i]+10-'A';

		if(str[i]>='a' && str[i]<='f')
			part=str[i]+10-'a';

		if(part==255)
		{
			*ok=0;
			return(0);
		}
		retval=retval<<4;
		retval|=part & 0x0f;
	}
	*ok=1;
	return(retval);
}

void DumpMem(uint32_t address)
{
	int x,y;

	for(y=0;y<2;y++)
	{
		console_SendHex(address,8);
		console_SendString(":");
		for(x=0;x<16;x++)
		{
			console_SendHex((*(volatile uint8_t *)address),2);
			console_SendString(" ");
			address++;
		}
		console_SendString("\r\n");
	}
}



void ProcessCommand(char *command)
{
	if(StrBeginsWith("HELLO",command))
	{
		console_SendString("Hello\r\n");
		return;
	}

	if(StrBeginsWith("EXIT",command))
	{
		console_SendString("Exit!\r\n");
		scb_reset_system();
		return;
	}

	if(StrBeginsWith("DUMPMEM",command))
	{
		int ok;
		uint32_t addr=ParseHex(command+8,8,&ok);
		if(ok)
			DumpMem(addr);
		else
			console_SendString("Hex parse fails");
		return;
	}

	if(StrBeginsWith("ERASEPAGE ",command))
	{
		int ok;
		uint32_t addr=ParseHex(command+10,8,&ok);
		if(ok)
			ErasePage(addr);
		else
			console_SendString("Hex parse fails");
		return;
		console_SendString("Erase ok\r\n");
	}

	if(StrBeginsWith("SPI R ",command))
	{
		int ok;
		uint16_t data;
		uint32_t addr=ParseHex(command+6,2,&ok);
		if(ok)
		{
			gpio_clear(GPIOB,GPIO0);
			addr &= 0x1f;
			spi_xfer(SPI1, addr);
			data=spi_xfer(SPI1, 0);
			gpio_set(GPIOB,GPIO0);
			console_SendString("Value:");
			console_SendHex(data,2);
			console_SendString("\r\n");
		}
		else
			console_SendString("Hex parse fails\r\n");
		return;
	}

	if(StrBeginsWith("ENC R ",command))
	{
		int ok;
		uint16_t data;
		uint32_t addr=ParseHex(command+6,2,&ok);
		if(ok)
		{
			data=enc28j60Read(addr);
			console_SendString("Value:");
			console_SendHex(data,2);
			console_SendString("\r\n");
		}
		else
			console_SendString("Hex parse fails\r\n");
		return;
	}

	if(StrBeginsWith("ENC W ",command))
	{
		int ok,ok2;

		uint32_t addr=ParseHex(command+6,2,&ok);
		uint32_t data=ParseHex(command+9,2,&ok2);
		if(ok&ok2)
		{
			enc28j60Write(addr,(uint8_t)data);
			console_SendString("OK\r\n");
		}
		else
			console_SendString("Hex parse fails\r\n");
		return;
	}

	if(StrBeginsWith("DS1820 SCAN",command))
	{
		uint8_t id[OW_ROMCODE_SIZE], sp[DS18X20_SP_SIZE], diff;
		uint8_t i;
		uint16_t meas;
		uint8_t subzero, cel, cel_frac_bits;


		if(DS18X20_start_meas(0,NULL)!=DS18X20_OK)
		{
			console_SendString( "Measure start failed\r\n" );
			return; // <--- early exit!
		}

		while(DS18X20_conversion_in_progress());

		for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE; )
		{
			diff = ow_rom_search( diff, &id[0]);

			if( diff == OW_PRESENCE_ERR )
			{
				console_SendString( "No sensors\r\n" );
				return; // <--- early exit!
			}

			if( diff == OW_DATA_ERR )
			{
				console_SendString( "Bus error\r\n" );
				return;     // <--- early exit!
			}

			console_SendString("Found sensor:");
			for(i=0;i<OW_ROMCODE_SIZE;i++)
				console_SendHex(id[i],2);
			console_SendString("\n\r");

			if( id[0] == DS18B20_FAMILY_CODE || id[0] == DS18S20_FAMILY_CODE || id[0] == DS1822_FAMILY_CODE )
			{
				console_SendString("Is DS1820 family\r\nSP=");

				ow_byte_wr( DS18X20_READ );           // read command

				for ( i=0 ; i< DS18X20_SP_SIZE; i++ )
				{
					sp[i]=ow_byte_rd();
				}

				for(i=0;i<DS18X20_SP_SIZE;i++)
				 console_SendHex(sp[i],2);

				if ( crc8( &sp[0], DS18X20_SP_SIZE ) )
				{
					console_SendString( "(FAIL)\r\n");
				}
				else
				{
					console_SendString( "(OK)\r\n");
				}

				meas = sp[0]; // LSB Temp. from Scrachpad-Data
				meas |= (uint16_t) (sp[1] << 8); // MSB

				console_SendString( "T_raw = 0x");
				console_SendHex( meas,4 );
				console_SendString("\r\n");

				if( id[0] == DS18S20_FAMILY_CODE )
				{ // 18S20
					console_SendString( "(S20/09) " );
				}
				else if ( id[0] == DS18B20_FAMILY_CODE || id[0] == DS1822_FAMILY_CODE )
				{ // 18B20 or 1822
					i=sp[DS18B20_CONF_REG];
					if ( (i & DS18B20_12_BIT) == DS18B20_12_BIT )
					{
						console_SendString( "(B20/12) ");
					}
					else if ( (i & DS18B20_11_BIT) == DS18B20_11_BIT )
					{
						console_SendString( "(B20/11) ");
					}
					else if ( (i & DS18B20_10_BIT) == DS18B20_10_BIT )
					{
						console_SendString( "(B20/10) ");
					}
					else
					{ // if ( (i & DS18B20_9_BIT) == DS18B20_9_BIT ) {
						console_SendString( "(B20/09) ");
					}
				}

				DS18X20_meas_to_cel( id[0], sp, &subzero, &cel, &cel_frac_bits );
				console_SendHex(cel,4);
				console_SendString(".");
				console_SendHex(cel_frac_bits*DS18X20_FRACCONV,4);
				console_SendString("\r\n");

			} // if meas-sensor
		} // loop all sensors

		return;
	}

	if(StrBeginsWith("SPIMAX",command))
	{
		uint16_t data;
		console_SendString("Start");
		gpio_clear(GPIOB,GPIO12);
		delay_us(1);
		console_SendString("1");
		data=spi_xfer(SPI2,0);
		console_SendString("2");
		gpio_set(GPIOB,GPIO12);
		console_SendString("Value:");
		console_SendHex(data,4);
		console_SendString("\r\n");
		return;
	}

	if(StrBeginsWith("DS1820 CONF ",command))
	{
		int ok,i;
		uint8_t rom[OW_ROMCODE_SIZE];

		uint32_t addr=ParseHex(command+12,2,&ok);

		if(ok)
		{
			for(i=0;i<OW_ROMCODE_SIZE;i++)
			{
				rom[i]=(uint8_t)ParseHex(command+15+i*2,2,&ok);
				if(ok==0)
					break;
			}
			if(ok)
			{
				for(i=0;i<OW_ROMCODE_SIZE;i++)
					settings.ids[addr].id[i]=rom[i];

				ErasePage(SETTINGS_ADDRESS);
				WriteBlock(SETTINGS_ADDRESS,&settings,sizeof(settings));
				console_SendString("OK\r\n");
				return;
			}
		}
			console_SendString("Hex parse fails\r\n");
		return;
	}
	if(StrBeginsWith("DS1820 READ SENSORS",command))
	{
		int s,i;
		DS18X20ReadSensors(ds1820Values,3);
		console_SendString("Values:\r\n");
		for(s=0;s<3;s++)
		{
			for(i=0;i<OW_ROMCODE_SIZE;i++)
				console_SendHex(ds1820Values[s].id[i],2);
			console_SendString(":");
			console_SendHex(ds1820Values[s].celcius,4);
			console_SendString("\r\n");
		}
		return;
	}

	if(StrBeginsWith("IP ADDRESS ",command))
	{
		int ok,i;		

		for(i=0;i<4;i++)
		{
			uint8_t addr=(uint8_t)ParseHex(command+11+i*2,2,&ok);
			if(!ok)
			{
				console_SendString("Parse error\r\n");
				return;
			}
			settings.ip[i]=addr;
		}
		ErasePage(SETTINGS_ADDRESS);
		WriteBlock(SETTINGS_ADDRESS,&settings,sizeof(settings));
		console_SendString("OK\r\n");
		return;
	}

	if(StrBeginsWith("MAX6674 SET CALIBRATION ",command))
	{
		int ok;

		uint16_t val=(uint16_t)ParseHex(command+24,4,&ok);

		if(!ok)
		{
			console_SendString("Parse error\r\n");
			return;
		}
		settings.Max6674Divider=val;

		val=(uint16_t)ParseHex(command+29,4,&ok);

		if(!ok)
		{
			console_SendString("Parse error\r\n");
			return;
		}
		settings.Max6674Offset=val;
		
		ErasePage(SETTINGS_ADDRESS);
		WriteBlock(SETTINGS_ADDRESS,&settings,sizeof(settings));
		console_SendString("OK\r\n");
		return;
	}

	console_SendString("Cannot parse command:");
	console_SendString(command);
	console_SendString("\r\n");
}


void console_Poll()
{
	unsigned char ch;

	while (serial_HasCh(&sp_console))
	{
		ch=serial_GetCh(&sp_console);
		if(ch==13)
		{
			command[cmdPos]=0;
			ProcessCommand(command);
			cmdPos=0;
			continue;
		}
		command[cmdPos]=ch;
		cmdPos++;

		if(echo)
			serial_SendCh(&sp_console,ch);
	}
}

