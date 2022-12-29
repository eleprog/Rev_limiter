//Δ
#define F_CPU 4400000
#include <avr/io.h>
#include <avr/interrupt.h>

#define RPM_INPUT_PIN	1
#define RPM_OUTPUT_PIN	4
#define SHIFT_LIGHT		3

#define RPM_CUT_OFF		4500
const uint16_t ticksCutOff = F_CPU / RPM_CUT_OFF * 30;
volatile uint16_t ticksCounter = 0;

#define SHIFT_LIGHT_OFF_DELAY 300	// (15мс - 3800мс)
const uint16_t shiftLightOff = F_CPU / 1000 * SHIFT_LIGHT_OFF_DELAY / 256;
volatile uint16_t shiftLightCounter = 0;


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


static uint8_t timer0Counter = 0;
ISR(TIM0_OVF_vect) {
	if(timer0Counter < 255)
		timer0Counter++;
		
	if(shiftLightCounter)
		shiftLightCounter--;
} 

ISR(INT0_vect) {
	static uint8_t mapMask = 1;
	
	if(PINB & 1<<RPM_INPUT_PIN) {
		uint8_t tmp = TCNT0;
		TCNT0 = 0;
		
		if(mapSpark[mapSparkPointer] & mapMask)
			PORTB |= 1<<RPM_OUTPUT_PIN;
		
		mapMask <<= 1;
		
		if(mapMask == 0b10000000)
			mapMask = 1;
		
		ticksCounter = (timer0Counter << 8) + tmp;
		
		timer0Counter = 0;
		TIFR0 = 0;	// сброс флага прерывания timer0 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	
	else
		PORTB &= ~(1<<RPM_OUTPUT_PIN);
}

int main(void) {
		
	/* GPIO Settings */
	DDRB	= 0b011000;
	PORTB	= 0b000010;
   
	/* INT0 Settings */
	MCUCR |= (0<<ISC01) | (1<<ISC00);	// 10 – нисходящий фронт 11 – восходящий фронт
	GIMSK = 1<<INT0;					// разрешение прерываний INT0*/

	/* TIMER0 Settings */
	TCCR0B = (0<<CS02)|(0<<CS01)|(1<<CS00);	// установка предделителя (0) таймера 0
	TIMSK0 |= 1<<TOIE0;						// разрешение прерываний по совпадению
	TCNT0 = 0;								// обнуление счетного регистра таймера 0
	
	asm("sei");
	
    while (1) {
		if(ticksCounter) {
			if(ticksCounter > ticksCutOff)
				mapSparkPointer = 7;
			else {
				mapSparkPointer = 3;
				shiftLightCounter = shiftLightOff;
			}
			ticksCounter = 0;
		}
		
		//if(shiftLightCounter)
		//PORTB |= 1<<SHIFT_LIGHT;
		//else
		//PORTB &= ~(1<<SHIFT_LIGHT);
	}
}