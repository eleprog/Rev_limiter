#include <avr/io.h>
#include <avr/interrupt.h>

#define RPM_INPUT_PIN	1
#define RPM_OUTPUT_PIN	4
#define SHIFT_LIGHT		3

#define RPM_CUT_OFF	3500

uint8_t rpmFlag

ISR(INT0_vect) {
	
}




int main(void) {
	/* GPIO Settings */
	DDRB	= 0b010000;
	PORTB	= 0b000000;
   
   
	/* INT0 Settings */
/*	00†Ц Ќизкий уровень на ножке INT0 или INT1
	01†Ц Ћюбое изменение на ножке INT0 или INT1
	10†Ц нисход€щий фронт на ножке INT0 или INT1
	11†Ц восход€щий фронт на ножке INT0 или INT1 */
	MCUCR |= 0<<ISC01 | 1<<ISC00;
	
	
	/* TIMER0 Settings */
	TCCR0B = (1<<CS02)|(0<<CS01)|(1<<CS00);	// установка предделител€ (1024) таймера 0
	TCNT0 = 0;								// обнуление счетного регистра таймера 0
	OCR0A = 2000;							// установка регистра сравнени€ таймера 0
	//TIMSK |= (1<<OCIEA);					// разрешение прерываний по совпадению*/
	
	
	
	
	
	asm("sei");
    while (1) {
		
    }
}