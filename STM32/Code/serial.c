#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/f1/flash.h>
#include <stdlib.h>
#include "settings.h"
#include "serial.h"
#include "console.h"

static volatile serial_port_t *USART1_handle=NULL;
static volatile serial_port_t *USART2_handle=NULL;
static volatile serial_port_t *USART3_handle=NULL;

void serial_Setup(serial_port_t *handle, uint32_t port, uint16_t bps,uint32_t flowcontrol,uint8_t databits,uint32_t parity)
{
	unsigned int i;

	for(i=0;i<sizeof(serial_port_t);i++)
		((unsigned char*)handle)[i]=0;

	handle->Port=port;

	switch(port)
	{
		case USART1:
			USART_CR1(USART1) &= ~USART_CR1_RXNEIE;
			USART_CR1(USART1) &= ~USART_CR1_TXEIE;

			USART1_handle=handle;

			// Enable USART1 interrupts
			nvic_enable_irq(NVIC_USART1_IRQ);
			nvic_set_priority(NVIC_USART1_IRQ, 1);
			/* Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1. */
			rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
			rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART1EN);

			/* Setup GPIO pin GPIO_USART1_TX/GPIO9 on GPIO port A for transmit. */
			gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
				 GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

			/* Setup GPIO pin GPIO_USART1_TX/GPIO10 on GPIO port A for recieve. */
			gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
				 GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

			/* Setup UART parameters. */
			USART_BRR(USART1) = (uint16_t)((F_CPU << 4) / (bps * 16));

			usart_set_databits(USART1, databits);
			usart_set_stopbits(USART1, USART_STOPBITS_1);
			usart_set_mode(USART1, USART_MODE_TX_RX);
			usart_set_parity(USART1, parity);
			usart_set_flow_control(USART1, flowcontrol);

			// Enable USART1
			usart_enable(USART1);

			// Enable USART1 RX & TX interrupt
			USART_CR1(USART1) |= USART_CR1_RXNEIE;
			USART_CR1(USART1) |= USART_CR1_TXEIE;
			break;

		case USART2:
			USART_CR1(USART2) &= ~USART_CR1_RXNEIE;
			USART_CR1(USART2) &= ~USART_CR1_TXEIE;		

			USART2_handle=handle;

			// Enable USART2 interrupts
			nvic_enable_irq(NVIC_USART2_IRQ);
			nvic_set_priority(NVIC_USART2_IRQ, 1);
			/* Enable clocks for GPIO port A (for GPIO_USART2_TX) and USART2. */
			rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
			rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USART2EN);

			/* Setup GPIO pin GPIO_USART2_TX on GPIO port A for transmit. */
			gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
				 GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);

			/* Setup GPIO pin GPIO_USART2_RX on GPIO port A for recieve. */
			gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
				 GPIO_CNF_INPUT_FLOAT, GPIO_USART2_RX);

			if(flowcontrol & USART_CR3_CTSE)
			{
				gpio_set_mode(GPIO_BANK_USART2_CTS, GPIO_MODE_INPUT,GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_USART2_CTS);
				gpio_clear(GPIO_BANK_USART2_CTS, GPIO_USART2_CTS);
			}

			// Enable USART2
			usart_enable(USART2);

			/* Setup UART parameters. F_CPU/2 because USART2 uses APB1 and it is divided by 2*/
			USART_BRR(USART2) = (uint16_t)(( (F_CPU/2) << 4) / (bps * 16));

			if(parity)
				databits++;

 			DEBUGSTR("COM2:");
                        DEBUGHEX(databits,4);
			DEBUGHEX(parity,4);
			DEBUGHEX(flowcontrol,4);
			DEBUGSTR("\n");

			usart_set_databits(USART2, databits);
			usart_set_stopbits(USART2, USART_STOPBITS_1);
			usart_set_parity(USART2, parity);
			usart_set_flow_control(USART2, flowcontrol);
			usart_set_mode(USART2, USART_MODE_TX_RX);


			// Enable USART2 RX & TX interrupt
			USART_CR1(USART2) |= USART_CR1_RXNEIE;
			USART_CR1(USART2) |= USART_CR1_TXEIE;
			break;

		case USART3:
			USART_CR1(USART3) &= ~USART_CR1_RXNEIE;
			USART_CR1(USART3) &= ~USART_CR1_TXEIE;

			USART3_handle=handle;

			// Enable USART3 interrupts
			nvic_enable_irq(NVIC_USART3_IRQ);
			nvic_set_priority(NVIC_USART3_IRQ, 1);
			/* Enable clocks for GPIO port B (for GPIO_USART3_TX) and USART3. */
			rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
			rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USART3EN);

			/* Setup GPIO pin GPIO_USART3_TX on GPIO port B for transmit. */
			gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
				 GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_TX);

			/* Setup GPIO pin GPIO_USART3_RX on GPIO port B for recieve. */
			gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
				 GPIO_CNF_INPUT_FLOAT, GPIO_USART3_RX);

			if(flowcontrol & USART_CR3_CTSE)
			{
				gpio_set_mode(GPIO_BANK_USART3_CTS, GPIO_MODE_INPUT,GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_USART3_CTS);
				gpio_clear(GPIO_BANK_USART3_CTS, GPIO_USART3_CTS);
			}

			/* Setup UART parameters. */
			USART_BRR(USART3) = (uint16_t)((F_CPU << 4) / (bps * 16));

			usart_set_databits(USART3, databits);
			usart_set_stopbits(USART3, USART_STOPBITS_1);
			usart_set_mode(USART3, USART_MODE_TX_RX);
			usart_set_parity(USART3, parity);
			usart_set_flow_control(USART3, flowcontrol);

			// Enable USART3
			usart_enable(USART3);

			// Enable USART3 RX & TX interrupt
			USART_CR1(USART3) |= USART_CR1_RXNEIE;
			USART_CR1(USART3) |= USART_CR1_TXEIE;
			break;
		}
}

void usart1_isr(void)
{
	usart_isr_general(USART1_handle,USART1);
}

void usart2_isr(void)
{
	usart_isr_general(USART2_handle,USART2);
}

void usart3_isr(void)
{
	usart_isr_general(USART3_handle,USART3);
}


void usart_isr_general(volatile serial_port_t *handle,uint32_t port)
{
	if(handle!=NULL && usart_get_flag(port,USART_SR_RXNE))
	{
		char c;
		c = usart_recv(port);
		handle->RxRingBuffer[handle->RxRingBufferHead]=c;
		handle->RxRingBufferHead++;
		handle->RxRingBufferHead&=(RING_BUFFER_SIZE-1);
	}

	if(usart_get_flag(port, USART_SR_TXE))
	{
		if(handle!=NULL && handle->TxRingBufferHead!=handle->TxRingBufferTail)
		{
			usart_send_blocking(port,handle->TxRingBuffer[handle->TxRingBufferTail]);
			handle->TxRingBufferTail++;
			handle->TxRingBufferTail&=(RING_BUFFER_SIZE-1);
			USART_CR1(handle->Port) |= USART_CR1_TXEIE;
		}
		else
		{
			USART_CR1(handle->Port) &= ~USART_CR1_TXEIE;
		}
	}
}


int serial_HasCh(serial_port_t *handle)
{
        int retval=0;

	uint32_t tmp=USART_CR1(handle->Port);
        USART_CR1(handle->Port) &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE);

        if( handle->RxRingBufferHead!= handle->RxRingBufferTail)
                retval=1;

        USART_CR1(handle->Port) = tmp;

        return(retval);
}

unsigned char serial_GetCh(serial_port_t *handle)
{
	unsigned char retval;

	while(!serial_HasCh(handle));

	uint32_t tmp=USART_CR1(handle->Port);
        USART_CR1(handle->Port) &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE);

	if(handle->RxRingBufferHead!=handle->RxRingBufferTail)
	{
		retval=handle->RxRingBuffer[handle->RxRingBufferTail];
		handle->RxRingBufferTail++;
	}
	else
		retval=0;

	handle->RxRingBufferTail&=(RING_BUFFER_SIZE-1);

        USART_CR1(handle->Port) = tmp;

	return(retval);
}

void serial_SendCh(serial_port_t *handle,char ch)
{
        USART_CR1(handle->Port) &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE);

	handle->TxRingBuffer[handle->TxRingBufferHead]=ch;
	handle->TxRingBufferHead++;
	handle->TxRingBufferHead&=(RING_BUFFER_SIZE-1);

	if(usart_get_flag(handle->Port, USART_SR_TXE) && handle->TxRingBufferHead!=handle->TxRingBufferTail)
	{
		usart_send_blocking(handle->Port,handle->TxRingBuffer[handle->TxRingBufferTail]);
		handle->TxRingBufferTail++;
		handle->TxRingBufferTail&=(RING_BUFFER_SIZE-1);
	}
        USART_CR1(handle->Port) |= USART_CR1_RXNEIE | USART_CR1_TXEIE;
}

void serial_GetBufferStates(serial_port_t *handle,uint32_t *tx,uint32_t *rx)
{
	uint32_t tmp=USART_CR1(handle->Port);
        USART_CR1(handle->Port) &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE);

	*tx=((handle->TxRingBufferHead | RING_BUFFER_SIZE) - handle->TxRingBufferTail) & (RING_BUFFER_SIZE-1);
	*rx=((handle->RxRingBufferHead | RING_BUFFER_SIZE) - handle->RxRingBufferTail) & (RING_BUFFER_SIZE-1);

        USART_CR1(handle->Port) = tmp;
}


