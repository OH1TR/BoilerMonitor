#ifndef _SETTINGS_H
#define _SETTINGS_H

#define F_CPU 72000000

#define SETTINGS_ADDRESS 0x08004000

#define DEBUGSTR(str) console_SendString(str);
#define DEBUGHEX(val,digits) console_SendHex(val,digits);


#endif
