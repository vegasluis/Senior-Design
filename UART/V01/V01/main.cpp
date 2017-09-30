/*
 * V01.cpp
 *
 * Created: 8/14/2017 3:10:18 PM
 * Author : Luis
 */ 

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <util/delay.h>
#include "Drivers/uart0.h"
#include <avr/interrupt.h>

using namespace std;

int main(void)
{
	//interrupts part
	uart0_int();
	//set interrupt 
	//RX_INTEN_0();
	//enable global interrupts
	//sei();

	//while(1);

    unsigned char ch = 'A';

	uart0_int();
	while (1) {
	  putChar_0(ch);
	  ch++;
	  if (ch > 'Z')
	    ch = 'A';
	}

	return 0;
}

ISR (USART0_RX_vect)
{
	unsigned char receivedChar;

	receivedChar = UDR0;							// Read data from the RX buffer
	UDR0 = receivedChar;							// Write the data to the TX buffer
}

