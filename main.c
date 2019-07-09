/*
 * Sensores.c
 *
 * Created: 07/07/2019 16:43:44
 * Author : User
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "display/lcd_i2c.h"
#include "lib/avr_usart.h"
#include "lib/modbus_ESP01.h"
#include "lib/avr_timer.h"

#include "lib/avr_twi_master.h"

volatile uint16_t y = 0x400;
volatile uint16_t j;

void config_timer0(){
	
	/* 
	 * config timer0 1kHz
	 * WGM01 -> CTC mode
	 * top = 249
	 * 64 prescaler
	 * Indirect access using avr_timer.h
	 */
	TIMER_0->TCCRA |= SET(WGM01);
	TIMER_0->TCCRB |= SET(CS00) | SET(CS01);
	TIMER_0->OCRA = 0xF9;
	TIMER_IRQS->TC0.MASK |= SET(OCIE0A);
	
	/*
	 * config echo port and interruption
	 * 
	 */
	//PORT_SONIC->DDR |= SET(PIN_ECHO);
	fprintf(get_usart_stream(),"Teste de config_timer0 \n\r");
}

int main(){
	uint8_t i = 0;
	FILE *lcd_stream = inic_stream_i2c();
	FILE *usart_stream = get_usart_stream();

	/* Debug */
	USART_Init(B9600);

	/* Inicializa modo líder */
	TWI_Master_Initialise();
	sei();

	/* Inicializa LCD através do expansor de E/S */
	inic_LCD_4bits_i2c();

	modbus_init();
	
	config_timer0();
	//fprintf(get_usart_stream(),"Teste de main \n\r");
	for(;;) {
		/*
		obtém dados dos sensores
		*/
		
		/*
		média móvel
		*/
		
		/*
		atuadores (display + motor + ?)
		*/
		
		/*
		modbus
		*/
		
//		uint8_t *x = &y;
		
		//USART_tx(&y, 2);
		//USART_tx_busy(3);
		modbus_write(&y, 1);
		_delay_ms(1000);
		y++;
		
		fprintf(lcd_stream,"Teste %u \n\r", i++);
	}


}

ISR(TIMER0_COMPA_vect){
	modbus_rx_timeout_inc();
}
