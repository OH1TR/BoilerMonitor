#include <libopencm3/stm32/f1/flash.h>
#include "flash.h"

void ErasePage(uint32_t address)
{
	flash_unlock();
	flash_erase_page(address);
	flash_lock();
}

void WriteBlock(uint32_t address,void *data,uint16_t len)
{
	uint16_t i;
	uint16_t *src=(uint16_t *)data;

	flash_unlock();

	for(i=0;i<len;i+=2)
		flash_program_half_word(address+i,src[i/2]);

	flash_lock();
}
