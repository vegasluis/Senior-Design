#ifndef TIMER1_16B_H
#define TIMER1_16B_H

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <util/delay.h>

#ifdef __cpluspls
extern "C" {
#endif
/*
 * Power Reduction 
 */
#define TC1_ON() PRR0 &= ~_BV(PRTIM1);
//Waveform Generation Mode 
/*
 * Timer/Counter 1
 * FAST PWM with ICR1 Reg as TOP update at Bottom, Flag set on Top
 */
#define FAST_PWM_ICR1() {\
	TCCR1A |= _BV(WGM11);\
	TCCR1A &= ~_BV(WGM10);\
	TCCR1B |= _BV(WGM13)|_BV(WGM12);\
}

//Compare Output Mode
/*
 * Clear OC1A/OC1B/OC1C on compare match,set OC1A/OC1B/OC1C at bottom
 * (non inverting mode)
 */
#define OC1A_NON_INV() TCCR1A |= _BV(COM1A1)
#define OC1B_NON_INV() TCCR1A |= _BV(COM1B1)
#define OC1C_NON_INV() TCCR1A |= _BV(COM1C1)

//Set Timer/Counter I/0 Pins as in/out
/*
 * Timer/Counter 1 I/O as outputs
 */

#define PIN_OC1A_OUT() DDRB |= _BV(PB5)
#define PIN_OC1B_OUT() DDRB |= _BV(PB6)
#define PIN_OC1C_OUT() DDRB |= _BV(PB7)

/*
 * Prescaling , Clock Select Bit Description 
 */
#define CS1_64() TCCR1B &= ~_BV(CS12); TCCR1B |= _BV(CS11)| _BV(CS10)

void initateTimer1(void)
{
	//ensure TIMER1 is enabled
	TC1_ON();

	/*Example*/
	  //TOP = (F_cpu/(Focnx*N))-1
	  //Desire Focnx = 50 Hz
	  //ICR1 = (uint_16)((16e6/(50*64))-1);
	ICR1 = 4999;

	//SET Timer 1 to have the top to be ICR1
	  //FAST PWM reading the OCRA in non-inverting mode
	  //A prescalar of 64 => 16MHz / 64

	  //ex: of bits being set
	  //TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<COM1C1)|(1<<WGM11);
	  //TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10);

	FAST_PWM_ICR1();
	CS1_64();
	OC1A_NON_INV();
	OC1B_NON_INV();
	OC1C_NON_INV();

	  //OC1A AND OC1B as outputs for PWM
	PIN_OC1A_OUT();
	PIN_OC1B_OUT();
	PIN_OC1C_OUT();


}

#ifdef __cpluspls
}
#endif

#endif //TIMER1_16B_H