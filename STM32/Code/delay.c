#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/stm32/usb.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/systick.h>
#include "delay.h"

void delay_us(uint16_t delay)
{
        uint32_t now=systick_get_value();
        uint32_t target=now - delay * 9; // 9 000 000/1000000
        target &= 0x00FFFFFF;

        if(target>now)
                while(systick_get_value()<now);

        while(systick_get_value()>target);
}

void delay_ms(uint16_t delay)
{
        uint32_t now=systick_get_value();
        uint32_t target=now - delay * 9000; // 9 000 000/1000
        target &= 0x00FFFFFF;

        if(target>now)
                while(systick_get_value()<now);

        while(systick_get_value()>target);
}

/*
void delay_ms_init(uint16_t delay,uint32_t *handle)
{
        uint32_t now=systick_get_value();
        uint32_t target=now - delay * 9000; // 9 000 000/1000
        target &= 0x00FFFFFF;
        if(target>now)
        	target|=0x80000000;
        *handle=target;


int delay_is_done(uint32_t *handle)
{
	if(target & 0x80000000)
	{
		if(systick_get_value()>now)
			*handle&=0x7fffffff;
		else
			return(0);
	}
	if(systick_get_value()<target)
		return(1);
	else
		return(0);
	}
}
*/
