/*
 * avr_usart.c
 *
 *  Created on: Mar 27, 2018
 *      Author: Renan Augusto Starke
 *      Instituto Federal de Santa Catarina
 */

#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "avr_usart.h"
#include "bits.h"

#define BUFFER_RX_SIZE 128

static int usart_putchar(char c, FILE *fp);
volatile uint8_t *buffer_tx, bytes_tx;
volatile uint8_t buffer_rx[BUFFER_RX_SIZE];
volatile uint8_t buffer_rx_rd;		// current read position
volatile uint8_t buffer_rx_wr;		// current write position

/* Stream init for printf  */
FILE usart_str = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

/* Return stream pointer  */
FILE * get_usart_stream(){
	return &usart_str;
}

void USART_Init(uint16_t bauds){

	USART_0->UBRR_H = (uint8_t) (bauds >> 8);
	USART_0->UBRR_L = bauds;

	/* Disable double speed  */
	USART_0->UCSR_A = 0;
	/* Enable TX and RX */
	//USART_0->UCSR_A |= SET(TXC0);								// necessary?
	USART_0->UCSR_B = SET(RXEN0) | SET(TXEN0) | SET(RXCIE0);
	/* Asynchronous mode:
	 * - 8 data bits
	 * - 1 stop bit
	 * - no parity 	 */
	USART_0->UCSR_C = SET(UCSZ01) | SET(UCSZ00);
}

void USART_rx_reset() {
	memset(buffer_rx, 0, BUFFER_RX_SIZE);
}

/* Send one byte: busy waiting */
void USART_tx_busy(uint8_t data) {

	/* Wait until hardware is ready */
	while (!(USART_0->UCSR_A & (1 << UDRE0)));

	USART_0->UDR_ = data;
}

/* Send n bytes: interruption */
void USART_tx(uint8_t *data, uint8_t size){
	/*uint8_t i;
	for (i=0; i<=size; i++){
		buffer_tx[i] = data[i];
	}*/
	
	buffer_tx = data;
	fprintf(&usart_str,"Teste de USART_tx: data = %x \n\r", *data);
	bytes_tx = size;
	/* Enable tx interrupt */
	SET_BIT(USART_0->UCSR_B, TXCIE0);
	//SET_BIT(USART_0->UCSR_B, UDRIE0);
}

ISR(USART_TX_vect){
	
	fprintf(&usart_str,"Teste de ISR_TX: byte = %x \n\r", *buffer_tx);
	USART_0->UDR_ = *buffer_tx;
	buffer_tx++;
	bytes_tx--;
	
	//disable interruption when all bytes sent
	if (bytes_tx <= 0)
		CLR_BIT(USART_0->UCSR_B, TXCIE0);
}


/* Receive one byte: busy waiting */
uint8_t USART_rx_busy() {

	/* Wait until something arrive  */
	while (!(USART_0->UCSR_A & (1 << RXC0)));

	return USART_0->UDR_;
}

/* Receive n bytes: interruption (not complete)*/
uint8_t USART_rx(uint8_t* buffer, uint8_t size){
	
	/* return 0 if rx interruption is writing in the position (interval) to be read */
	if ((buffer_rx_wr >= buffer_rx_rd) && (buffer_rx_wr < (buffer_rx_rd + size))){
		return 0;
	}
	
	uint8_t i;
	for (i=0; i<size; i++){
		buffer[i] = buffer_rx[buffer_rx_rd];
		buffer_rx[buffer_rx_rd] = 0;
		buffer_rx_rd++;
		if (buffer_rx_rd >= BUFFER_RX_SIZE){
			buffer_rx_rd = 0;
		}
	}
	return 1;
}

/* Receive 1 byte from buffer */
uint8_t USART_rx_byte(){
	
	/* return NULL if rx is writing in the position (interval) to be read */
	if (buffer_rx_wr == buffer_rx_rd)
		return 0xff;
	
	uint8_t data = buffer_rx[buffer_rx_rd];
	buffer_rx[buffer_rx_rd] = 0;
	buffer_rx_rd ++;
	if (buffer_rx_rd >= BUFFER_RX_SIZE){		//USART_rx is not responsible if size is out of bounds
		buffer_rx_rd = 0;
	}
	return data;
}

ISR(USART_RX_vect){
	buffer_rx[buffer_rx_wr] = USART_0->UDR_;
	buffer_rx_wr++;
	if (buffer_rx_wr >= BUFFER_RX_SIZE){
		buffer_rx_wr = 0;
	}
}


static int usart_putchar(char c, FILE *fp){
	USART_tx_busy(c);

	return 0;
}

/*
Notes:
Verify for error on RX


SSTD: EDR_xxx
pass: esp-open-rtos
browser: 172.16.0.1
	wifi: config
	wifi: statio

download MQTT Explorer
Broker:
	ip: 150.162.29.25
	user: alunos
	pass: iFsC@2018

*/