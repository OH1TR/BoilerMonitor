#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "settings.h"
#include "enc28j60.h"
#include "ip_arp_udp_tcp.h"
#include "net.h"
#include "onewire.h"
#include "console.h"
#include "main.h"

static uint8_t mac[6] = {0x54, 0x55, 0x58, 0x10, 0x00, 0x24};
static uint8_t ip[4] = {192, 168, 98, 50};
static uint16_t port = 80;

#define BUFFER_SIZE 500
static uint8_t buf[BUFFER_SIZE+1];

#define STR_BUFFER_SIZE 32
static char strbuf[STR_BUFFER_SIZE+1];

//#define LOG(x) console_SendString(x)
#define LOG(x) 

void webserverInit()
{
	int i;
	for(i=0;i<4;i++)
		ip[i]=settings.ip[i];

	// SPI setup
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, GPIO0); //This is SS
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, GPIO4); //This is Reset

	/* Configure GPIOs: SS=PA4, SCK=PA5, MISO=PA6 and MOSI=PA7.  !!PA4(SS) not really used.*/
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4 | GPIO5 | GPIO7 );

	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO6);

	/* Reset SPI, SPI_CR1 register cleared, SPI is disabled */
	spi_reset(SPI1);

	/* Set up SPI in Master mode with:
	* Clock baud rate: 1/64 of peripheral clock frequency
	* Clock polarity: Idle Low
	* Clock phase: Data valid on 2nd clock pulse
	* Data frame format: 8-bit
	* Frame format: MSB First
	*/
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_8, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
				  SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

	/*
	* Set NSS management to software.
	*
	* Note:
	* Setting nss high is very important, even if we are controlling the GPIO
	* ourselves this bit needs to be at least set to 1, otherwise the spi
	* peripheral will not send any data out.
	*/
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);

	/* Enable SPI1 periph. */
	spi_enable(SPI1);

	// Initialize enc28j60
	gpio_set(GPIOB,GPIO0);
	gpio_clear(GPIOC,GPIO4);
	delay_ms(100);
	gpio_set(GPIOC,GPIO4);
	delay_ms(200);
	enc28j60Init(mac);
    enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
	enc28j60PhyWrite(PHLCON,0x880);
	delay_ms(500);
	enc28j60PhyWrite(PHLCON,0x990);
	delay_ms(500);
	enc28j60PhyWrite(PHLCON,0x880);
	delay_ms(500);
	enc28j60PhyWrite(PHLCON,0x990);
	delay_ms(500);
    enc28j60PhyWrite(PHLCON,0x476);

    init_ip_arp_udp_tcp(mac, ip, port);
}

void webserverPoll()
{
	uint16_t dat_p;

	uint16_t plen=enc28j60PacketReceive(BUFFER_SIZE, buf);
	if(plen!=0)
	{
		LOG("Packet\r\n");
		// arp is broadcast if unknown but a host may also verify the mac address by sending it to a unicast address.
		if (eth_type_is_arp_and_my_ip(buf, plen))
		{
			LOG("ARP\r\n");
			make_arp_answer_from_request(buf);
			return;
		}
		// check if ip packets are for us:
		if (eth_type_is_ip_and_my_ip(buf, plen) == 0)
		{
			LOG("Not me\r\n");
			return;
		}
		if (buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V)
		{
			LOG("reply\r\n");
			make_echo_reply_from_request(buf, plen);
			return;
		}

		// tcp port www start, compare only the lower byte
		if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P] == 80)
		{
		    	if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
			{
			 	make_tcp_synack_from_syn(buf); // make_tcp_synack_from_syn does already send the syn,ack
				LOG("syn\r\n");
				return;
		      	}

			if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
			{
				LOG("ack_v\r\n");
				init_len_info(buf); // init some data structures
				dat_p=get_tcp_data_pointer();
				if (dat_p==0)
				{ // we can possibly have no data, just ack:
		  			if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
					{
						LOG("make_tcp_ack_from_any\r\n");
		    				make_tcp_ack_from_any(buf);
		  			}
					LOG("ex\r\n");
					return;
				}
				//console_SendString(":");
				//console_SendString((char *)&(buf[dat_p]));
				if (StrBeginsWith("GET ",(char *)&(buf[dat_p])))
				{
					LOG("get\r\n");
		  			// head, post and other methods for possible status codes see:
					// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
		    			plen=fill_tcp_data_p(buf,0,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
					plen=fill_tcp_data_p(buf,plen,webStr);
					make_tcp_ack_from_any(buf); // send ack for http get
					make_tcp_ack_with_data(buf,plen); // send data
				}
				if (StrBeginsWith("/",(char *)&(buf[dat_p+4]))) // was "/ " and 2
				{
					// Copy the request action before we overwrite it with the response
					int i = 0;
					while (buf[dat_p+5+i] != ' ' && i < STR_BUFFER_SIZE)
					{
						strbuf[i] = buf[dat_p+5+i];
						i++;
					}
					strbuf[i] = '\0';
					plen=fill_tcp_data_p(buf,0,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
					LOG(strbuf);
					return;
		 		}
			}
		}
	}
}
