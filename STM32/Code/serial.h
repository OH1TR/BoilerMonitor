#ifndef _SERIAL_H
#define _SERIAL_H

// Only 2^x numbers allowed. 64,128,256..etc.
#define RING_BUFFER_SIZE 256

typedef struct
{
	uint32_t Port;
	volatile unsigned char RxRingBuffer[RING_BUFFER_SIZE];
	volatile unsigned char RxRingBufferHead;
	volatile unsigned char RxRingBufferTail;

	volatile unsigned char TxRingBuffer[RING_BUFFER_SIZE];
	volatile unsigned char TxRingBufferHead;
	volatile unsigned char TxRingBufferTail;
} serial_port_t;

void serial_Setup(serial_port_t *handle, uint32_t port, uint16_t bps,uint32_t flowcontrol,uint8_t databits,uint32_t parity);
int serial_HasCh(serial_port_t *handle);
unsigned char serial_GetCh(serial_port_t *handle);
void serial_SendCh(serial_port_t *handle,char ch);
void serial_GetBufferStates(serial_port_t *handle,uint32_t *tx,uint32_t *rx);

void usart1_isr(void);
void usart2_isr(void);
void usart3_isr(void);
void usart_isr_general(volatile serial_port_t *handle,uint32_t port);

#endif

