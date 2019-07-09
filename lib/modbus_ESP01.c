/*
 * modbus_ESP01.c
 *
 * Created: 04/06/2019 08:29:11
 *  Author: Aluno
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <util/delay.h>
#include <avr/io.h>

#include "modbus_ESP01.h"
#include "avr_usart.h"

volatile modbus_type modbus_tx, modbus_rx;
volatile uint8_t modbus_timer_flag;
volatile uint16_t modbus_timer;

#define MODBUS_TIMER_MAX 5000 //set according to timer (around 1s)

void modbus_init()
{
	modbus_tx.addr = 0x15;
}

uint8_t modbus_write(uint16_t *data, uint8_t sensor_n)
{
	uint8_t *x = (uint8_t *) data;
	modbus_tx.data_h = *(x+1);
	modbus_tx.data_l = *x;
	modbus_tx.cmd = 0x01;
	modbus_tx.reg_h = 0;
	modbus_tx.reg_l = (0x04 + sensor_n);		//registers 0x05 to 0x08 (up to 4 sensors)
	uint16_t crc = CRC16_2(&modbus_tx.addr, 6);
	modbus_tx.crc_h = crc>>8;
	modbus_tx.crc_l = crc;
	
	USART_tx(&modbus_tx.addr, 8);
	if (!modbus_answer()){
		/* (CRC/register invalid error) */
		return 0;
	}
	return 1;
}

uint16_t modbus_read(uint8_t actuator_n)
{
	modbus_tx.cmd = 0x02;
	modbus_tx.reg_h = 0;
	modbus_tx.reg_l = actuator_n;
	uint16_t crc = CRC16_2(&modbus_rx.addr, 6);
	modbus_tx.crc_h = crc>>8;
	modbus_tx.crc_l = crc;
	
	USART_tx(&modbus_tx.addr, 8);
	if (!modbus_answer()){
		/* (CRC/register invalid error) */
		return 0;
	}
	
	uint16_t *data;
	uint8_t *x = (uint8_t *) data;
	x[0] = modbus_rx.data_l;
	x[1] = modbus_rx.data_h;
	
	return *data;
}

uint8_t modbus_answer(){
	
	_delay_ms(50);
	
	uint8_t buffer[8];
	modbus_rx_timout_init();
	/* wait for first 4 bytes for error verification */
	while ((modbus_timer_flag) && !(USART_rx(buffer, 4))){
		_delay_ms(100);
		//fprintf(get_usart_stream(),"Teste de modbus_answer: byte = %u, %u \n\r", modbus_timer, modbus_timer_flag);
	}
	/* reset usart rx buffer if answer is invalid CRC or invalid reg  */
	if ((buffer[1] == 0xff) || (buffer[1] == 0xfe)){
		//crc = CRC16_2(buffer, 2);
		USART_rx_reset();
		modbus_timer_flag = 0;
		return 0;
	}
	/* wait for last 4 bytes */
	while ((modbus_timer_flag) && !(USART_rx(&buffer[4], 4))){
		_delay_ms(10);
	}
	
	if (modbus_timer_flag){
		uint8_t i;
		uint8_t* p = &modbus_rx.addr;
		for (i=0; i<8; i++){
			p[i] = buffer[i];
		}
	}
	modbus_timer_flag = 0;
	return 1;
	
	/* 
	 * To do:
	 * - CRC verification
	 */
}

uint16_t CRC16_2(uint8_t *buf, int len)
{
	uint32_t crc = 0xFFFF;
	int i;
	for(i = 0; i<len; i++) {
		crc ^= (uint16_t) buf[i];				// XOR byte into least sig. byte of crc
		for(int i = 8; i != 0; i--) {			// Loop over each bit
			if((crc & 0x0001) != 0) {			// If the LSB is set
				crc>>= 1;						// Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else								// Else LSB is not set
				crc >>= 1;						// Just shift right
		}
	}
	return crc;
}

void modbus_rx_timout_init(){
	modbus_timer_flag = 1;
}

void modbus_rx_timeout_inc(){
	if (modbus_timer_flag){	
		modbus_timer++;
		if (modbus_timer >= MODBUS_TIMER_MAX){
			modbus_timer_flag = 0;
			modbus_timer = 0;
		}
	}
}
