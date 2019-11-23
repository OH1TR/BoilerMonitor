#ifndef _FLASH_H
#define _FLASH_H

void ErasePage(uint32_t address);
void WriteBlock(uint32_t address,void *data,uint16_t len);

#endif
