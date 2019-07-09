/*
 * modbus_ESP01.h
 *
 * Created: 04/06/2019 08:32:48
 *  Author: Aluno
 */ 


#ifndef MODBUS_ESP01_H_
#define MODBUS_ESP01_H_

#include <avr/io.h>

#ifndef _IO
#define _IO volatile uint8_t
#endif

#ifndef _IO16
#define _IO16 volatile uint16_t
#endif

typedef struct
{
	_IO addr;
	_IO cmd;
	_IO reg_h;
	_IO reg_l;
	_IO data_h;
	_IO data_l;
	_IO crc_h;
	_IO crc_l;
} modbus_type;

void modbus_init();
uint8_t modbus_write(uint16_t *data, uint8_t sensor_n);
uint16_t modbus_read(uint8_t actuator_n);
uint16_t CRC16_2(uint8_t *buf, int len);
uint8_t modbus_answer();
void modbus_rx_timout_init();
void modbus_rx_timeout_inc();

#endif /* MODBUS_ESP01_H_ */