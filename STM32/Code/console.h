#ifndef _CONSOLE_H
#define _CONSOLE_H

void IntToDec(char *buffer,uint32_t val,int digits);
void IntToHex(char *buffer,uint32_t val,int digits);
void console_SendString(char *msg);
void console_Setup(uint16_t bps);
void console_SendString(char *msg);

void console_SendHex(uint32_t val,int digits);
int StrBeginsWith(char *begin,char *str);

void console_Poll();

#endif


