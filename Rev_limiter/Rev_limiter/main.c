//Δ
#define F_CPU 4400000
#include <avr/io.h>
#include <avr/interrupt.h>

#define RPM_INPUT_PIN	1
#define RPM_OUTPUT_PIN	4

#define SHIFT_LIGHT_PIN	3
#define BUTTON_PIN		2

#define RPM_CUT_OFF		4000
const uint16_t ticksCutOff = F_CPU / RPM_CUT_OFF * 30;
volatile uint16_t ticksCounter = 0;

#define SHIFT_LIGHT_OFF_DELAY 300	// (15мс - 3800мс)
const uint16_t shiftLightOff = F_CPU / 1000 * SHIFT_LIGHT_OFF_DELAY / 256;
volatile uint16_t shiftLightCounter = 0;

#define TICKS_CUT_OFF_STEP 300;

volatile uint8_t mapSparkPointer = 7;
uint8_t mapSpark[] = {
	0b0000000,
	0b0100000,
	0b0100010,
	0b0110010,
	0b0110110,
	0b1110110,
	0b1111110,
	0b1111111};


volatile static uint8_t timer0Counter = 0;

ISR(TIM0_OVF_vect) {
	if(timer0Counter < 255)
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
		
		if(mapSpark[mapSparkPointer] & mapMask)
			PORTB |= 1<<RPM_OUTPUT_PIN;
		
		mapMask <<= 1;
		if(mapMask == 0b10000000)
		mapMask = 1;
		
		ticksCounter = (timer0Counter << 8) + tmp;
		
		timer0Counter = 0;
		TIFR0 |= 1<<TOV0;	// сброс флага OVF timer0
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
	TCCR0B	= 0b001<<CS00;	// установка предделителя (0) таймера 0
	TIMSK0	= 1<<TOIE0;		// разрешение прерываний по совпадению
	TCNT0	= 0;			// обнуление счетного регистра таймера 0
	
	asm("sei");
	
	
	static uint16_t ticksArr[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
	static uint32_t	ticksPerRotation = 0x3FFFC;
	static uint8_t ticksArrCounter = 0;
	
    while (1) {
		if(ticksCounter) {
			if(++ticksArrCounter > 3)
				ticksArrCounter = 0;
				
			ticksPerRotation -= ticksArr[ticksArrCounter];
			ticksPerRotation += ticksCounter;
			ticksArr[ticksArrCounter] = ticksCounter;
					
			uint16_t ticks = ticksPerRotation / 4;
			if(ticks < ticksCutOff) {
				uint16_t delta = ticksCutOff - ticks;
				
				if(delta < 82)			mapSparkPointer = 6;
				else if(delta < 246)	mapSparkPointer = 5;
				else if(delta < 488)	mapSparkPointer = 4;
				else if(delta < 805)	mapSparkPointer = 3;
				else if(delta < 1193)	mapSparkPointer = 2;
				else if(delta < 1571)	mapSparkPointer = 1;
				else					mapSparkPointer = 0;
				
				shiftLightCounter = shiftLightOff;
				PORTB |= 1<<SHIFT_LIGHT_PIN;
			}
			else
				mapSparkPointer = 7;
			
			ticksCounter = 0;
		}

		if(!shiftLightCounter)
			PORTB &= ~(1<<SHIFT_LIGHT_PIN);
			
			
	}
}	
		