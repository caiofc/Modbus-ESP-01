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

void ultrasonic_config_timer0(){
	
	/* 
	 * config timer0 1kHz
	 * WGM01 -> CTC mode
	 * top = 15
	 * 0 prescaler
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

	/* Inicializa modo l�der */
	TWI_Master_Initialise();
	sei();

	/* Inicializa LCD atrav�s do expansor de E/S */
	inic_LCD_4bits_i2c();

	modbus_init();
	
	ultrasonic_config_timer0();
	//fprintf(get_usart_stream(),"Teste de main \n\r");
	for(;;) {
		/*
		obt�m dados dos sensores
		*/
		
		/*
		m�dia m�vel
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
	//j++;
	//if (j == 10){							//10us trigger
		//CLR_BIT(PORT_SONIC->PORT, PIN_TRIGGER);
	//}
	//if (j >= MEASURE_TIME_uS){
		//j = 0;
		//SET_BIT(PORT_SONIC->PORT, PIN_TRIGGER);
	//}
	//fprintf(get_usart_stream(),"Teste de timer0 \n\r");
	modbus_rx_timeout_inc();
}