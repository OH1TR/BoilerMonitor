#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "max6674.h"
#include "delay.h"

static int Max6674Divider=100;
static int Max6674Offset=0;

void Max6674Init()
{
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, GPIO12); //This is SS

	/* Configure GPIOs: SS=PB12, SCK=PB13, MISO=PB14 and MOSI=PB15. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO13 );

	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO14);

	/* Reset SPI, SPI_CR1 register cleared, SPI is disabled */
	spi_reset(SPI2);

	/* Set up SPI in Master mode with:
	* Clock baud rate: 1/64 of peripheral clock frequency
	* Clock polarity: Idle Low
	* Clock phase: Data valid on 2nd clock pulse
	* Data frame format: 8-bit
	* Frame format: MSB First
	*/
	spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_256, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
				  SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_16BIT, SPI_CR1_MSBFIRST);

	/*
	* Set NSS management to software.
	*
	* Note:
	* Setting nss high is very important, even if we are controlling the GPIO
	* ourselves this bit needs to be at least set to 1, otherwise the spi
	* peripheral will not send any data out.
	*/
	spi_enable_software_slave_management(SPI2);
	spi_set_nss_high(SPI2);

	/* Enable SPI1 periph. */
	spi_enable(SPI2);
}

void Max6674SetCalibration(int divider,int offset)
{
	Max6674Divider=divider;
	Max6674Offset=offset;

}

int Max6674Read()
{
	uint32_t data;

	gpio_clear(GPIOB,GPIO12);
	delay_us(1);
	data=spi_xfer(SPI2,0);
	gpio_set(GPIOB,GPIO12);

	if(data & 4)
		return(-1); // Thermocouple not connected

	data=data>>3;
	return(((data*100)/Max6674Divider)+Max6674Offset);
}

