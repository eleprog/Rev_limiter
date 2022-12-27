﻿//Δ
#define F_CPU 4400000
#include <avr/io.h>
#include <avr/interrupt.h>

#define RPM_INPUT_PIN	1
#define RPM_OUTPUT_PIN	4
#define SHIFT_LIGHT		3

#define RPM_CUT_OFF	4500
uint16_t ticksCutOff = F_CPU / RPM_CUT_OFF * 30;

uint8_t timer0Counter = 0;
uint16_t ticksCounter = 0;

ISR(TIM0_OVF_vect) {
	if(timer0Counter < 255)
		timer0Counter++;
}

ISR(INT0_vect) {
	uint8_t tmp = TCNT0;
	TCNT0 = 0;
	
	ticksCounter = (timer0Counter << 8) + tmp;
	timer0Counter = 0;
	
	TIFR0 = 0;	// сброс флага прерывания timer0
}

int main(void) {
	
	/* GPIO Settings */
	DDRB	= 0b010000;
	PORTB	= 0b010000;
   
	/* INT0 Settings */
	MCUCR |= 1<<ISC01 | 1<<ISC00;	// 10 – нисходящий фронт 11 – восходящий фронт
	GIMSK = 1<<INT0;				// разрешение прерываний INT0
	
	/* TIMER0 Settings */
	TCCR0B = (0<<CS02)|(0<<CS01)|(1<<CS00);	// установка предделителя (0) таймера 0
	TIMSK0 |= 1<<TOIE0;						// разрешение прерываний по совпадению
	TCNT0 = 0;								// обнуление счетного регистра таймера 0
	
	asm("sei");
    while (1) {
		if(ticksCounter) {
			if(ticksCounter > ticksCutOff)
				PORTB |= 1<<RPM_OUTPUT_PIN;
			else
				PORTB &= ~(1<<RPM_OUTPUT_PIN);
			
			ticksCounter = 0;
		}
	}
}