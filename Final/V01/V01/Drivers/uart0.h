#ifndef UART0_H
#define UART0_H

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>


#ifdef __cpluspls
extern "C" {
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif 

/*SETTINGS*/
#ifndef BAUD
#define BAUD  9600						//BAUDRATE (9600 is default)
#endif
#define BUFF	 256

#define ASYNCH_NORM	((F_CPU/16)/BAUD - 1)
#define ASYNCH_DUB	((F_CPU/8)/BAUD - 1)
#define SYNC_MASTER ((F_CPU/2)/BAUD - 1)

/*
 * Power Reduction 
 */
#define USART0_ON() PRR0 &= ~_BV(PRUSART0);

/*MACROS USEFUL TO DISABLE AND ENABLE*/
#define TX_START_0()	UCSR0B |= _BV(TXEN0)	// Enable TX
#define TX_STOP_0()		UCSR0B &= ~_BV(TXEN0)	// Disable TX
#define RX_START_0()	UCSR0B |= _BV(RXEN0)	// Enable RX
#define RX_STOP_0()		UCSR0B &= ~_BV(RXEN0)	// Disable RX
#define COMM_START_0()	TX_START_0(); RX_START_0()	// Enable communicationsF
#define COMM_STOP_0()	TX_STOP_0();RX_STOP_0()		// Disable Communication	

//Frame Size to be Transmitted 
#define CHAR5_0()	UCSR0B &= ~ _BV(UCSZ02); UCSR0B &= (0<<UCSZ01) | (0<<UCSZ02)	
#define CHAR6_0()	UCSR0C |= _BV(UCSZ00) 
#define CHAR7_0()	UCSR0C |= _BV(UCSZ01) 
#define CHAR8_0()	UCSR0C |= _BV(UCSZ01)|_BV(UCSZ00) 
#define CHAR9_0()	UCSR0B |= _BV(UCSZ02);UCSR0C |= _BV(UCSZ01)|_BV(UCSZ00)

/* Interrupt macros; Remember to set the GIE bit in SREG before using (see datasheet) */
#define RX_INTEN_0()	UCSR0B |= _BV(RXCIE0)	// Enable interrupt on RX complete
#define RX_INTDIS_0()	UCSR0B &= ~_BV(RXCIE0)	// Disable RX interrupt
#define TX_INTEN_0()	UCSR0B |= _BV(TXCIE0)	// Enable interrupt on TX complete
#define TX_INTDIS_0()	UCSR0B &= ~_BV(TXCIE0)	// Disable TX interrupt

/*Stop Bit*/
#define STOPBIT_1_0()	UCSR0C &= (0<<USBS0)
#define STOPBIT_2_0()	UCSR0C |= (1<<USBS0)

/*Parity Mode*/
#define DisParity_0() 	UCSR0C &= ~(1<<UPM01);UCSR0C &= ~(1<<UPM00)
#define EvenParity_0()	UCSR0C |= (1<<UPM01)
#define OddParity_0()	UCSR0C |= (1<<UPM01)|(1<<UPM00)

/*MODE*/
#define ASYNCH_MODE_0()	UCSR0C &= ~(1<<UMSEL01);UCSR0C &= ~(1<<UMSEL00)
#define SYNCH_MODE_0()	UCSR0C |= (1<<UMSEL00)
#define MASTER_MODE_0()	UCSR0C |= (1<<UMSEL01)|(1<<UMSEL00)

/*FUNCTION DECLERATION*/

/*
 * Procedure to initialize USART0 asynchronous with enabled RX/TX, 8 bit data,
 * no parity, and 1 stop bit. 
 */
void uart0_int(void)
{
	//make sure USART0 is on
	USART0_ON();
	
	// To set baud rate
	unsigned int baudrate = ASYNCH_NORM;
	UBRR0H = (unsigned char) ((baudrate) >> 8);	//top nibble
	UBRR0L = (unsigned char) ((baudrate)) ;		//lower byte		

	COMM_START_0();									// enable transmit/receive

	// asynchronous, 8N1, disable parity, 1 stop bit
	ASYNCH_MODE_0();
	STOPBIT_1_0();
	CHAR8_0();
}

/* 
 *  Return a char from the serial buffer
 *  Use this function if the RX interrupt is not enabled.
 * 	Returns 0 on empty buffer
 */

unsigned char getChar_0(void)
{
	//Check if something was received and then 
	//return the item 
	while(!(UCSR0A & _BV(RXC0)));
	return (unsigned char) UDR0;
}

/*
 * Transmits a byte
 *
 * 	Use this function if the TX interrupt is not enabled.
 * 	Blocks the serial port while TX completes
 */
void putChar_0(unsigned char data)
{
	//Wait until the buffer is empty 
	if(data == '\n')
		putChar_0('\r');
			
	while(!(UCSR0A & _BV(UDRE0)));
	UDR0 = data;
}

/* A string print called printm that uses a 
 * char array and your putchar clone to transmit
 * strings
 */

void printm_0(char *str)
{
	//While it's not NULL
	while(*str != '\0')
	{
		putChar_0(*str);
		++str;
	}
}

/*
 * uses an uninitialized char array and your getchar clone to
 * construct a string for your ATmega328P 
 */
void scanm_0(uint8_t* buffer)
{
	int i = 0;
	while(buffer[i] != '\n'){
		buffer[i] = getChar_0();
		if (buffer[i]=='\r')
			break;
		putChar_0(buffer[i]);
		i++;
	}
	buffer[i] = '\0';
}

void print_int_0(int data)
{
	char datastring[30] = {0};
	sprintf(datastring, "%d", data);
	printm_0(datastring);
}

void print_double_0(double data)
{
	char datastring[30] = {0};
	sprintf(datastring, "%f", data);
	printm_0(datastring);
}

void UART_0_flush(void)
{	
	unsigned char dummy;
	while(UCSR0A & (1 << RXC0))dummy = UDR0;
}

#ifdef __cpluspls
}
#endif

#endif //UART_H