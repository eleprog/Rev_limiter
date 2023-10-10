#define F_CPU 4500000 //Δ
#include <avr/io.h>
#include <avr/interrupt.h>

#define RPM_CUT_OFF		4000

#define RPM_INPUT_PIN	1
#define RPM_OUTPUT_PIN	4
#define SHIFT_LIGHT_PIN	3
#define BUTTON_PIN		2

const uint16_t ticksPerRotationCutOff = F_CPU / RPM_CUT_OFF * 15;
volatile uint16_t ticksCounter = 0;

volatile uint8_t shiftLightCounter = 0;
volatile uint8_t timer0Counter = 0;

volatile uint8_t mapSpark = 0b1111111;

ISR(TIM0_OVF_vect) {
	if(timer0Counter < 0x3F)
		timer0Counter++;
			
	if(shiftLightCounter)
		shiftLightCounter--;
} 

ISR(INT0_vect) {
	static uint8_t mapMask = 1;
	
	if(PINB & 1<<RPM_INPUT_PIN)
		PORTB &= ~(1<<RPM_OUTPUT_PIN);
				
	else {
		uint8_t tmp = TCNT0;
		TCNT0 = 0;
		
		if(mapSpark & mapMask)
			PORTB |= 1<<RPM_OUTPUT_PIN;
		
		mapMask <<= 1;
		if(mapMask == 0b10000000)
			mapMask = 1;
		
		ticksCounter = (timer0Counter << 8) + tmp;
		
		timer0Counter = 0;
		TIFR0 = 0;	// reset flag OVF timer0
	}
}

int main(void) {
	/* GPIO Settings */
	DDRB	= 0b011001;
	PORTB	= 0b000000;
   
	/* INT0 Settings */
	MCUCR	= 0b01<<ISC00;	// 01 - любое изменение INT0
	GIMSK	= 1<<INT0;		// разрешение прерываний INT0

	/* TIMER0 Settings */
	TCCR0B	= 0b010<<CS00;	// установка предделителя (8) таймера 0
	TIMSK0	= 1<<TOIE0;		// разрешение прерываний по совпадению
	TCNT0	= 0;			// обнуление счетного регистра таймера 0
	asm("sei");
	
	static uint16_t	ticksPerRotation = 0xFFFC;
	static uint16_t ticksArr[4] = {0x3FFF, 0x3FFF, 0x3FFF, 0x3FFF};
	static uint8_t	ticksArrCounter = 0;
	
    while (1) {
		if(ticksCounter) {
			ticksPerRotation -= ticksArr[ticksArrCounter];
			ticksPerRotation += ticksCounter;
			ticksArr[ticksArrCounter] = ticksCounter;
			
			if(++ticksArrCounter > 3)
				ticksArrCounter = 0;
			
			if(ticksPerRotation > ticksPerRotationCutOff) {
				mapSpark = 0b1111111;
			}
			else {
				uint16_t delta = ticksPerRotationCutOff - ticksPerRotation;
				
				if(delta < 42)			mapSpark = 0b1111110;
				else if(delta < 126)	mapSpark = 0b1110110;
				else if(delta < 249)	mapSpark = 0b0110110;
				else if(delta < 412)	mapSpark = 0b0110010;
				else if(delta < 610)	mapSpark = 0b0100010;
				else if(delta < 804)	mapSpark = 0b0100000;
				else					mapSpark = 0b0000000;
				
				shiftLightCounter = 0xFF;
				PORTB |= 1<<SHIFT_LIGHT_PIN;
			}
			ticksCounter = 0;
		}

		if(!shiftLightCounter)
			PORTB &= ~(1<<SHIFT_LIGHT_PIN);
			
	}
}