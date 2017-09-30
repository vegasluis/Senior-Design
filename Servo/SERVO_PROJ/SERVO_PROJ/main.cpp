/*
 * SERVO_PROJ.cpp
 *
 * Created: 8/18/2017 4:47:09 PM
 * Author : Luis
 */ 

#define F_CPU 16000000UL
#ifndef BAUD
#define BAUD 9600
#endif
#include <avr/io.h>
#include "Drivers/timer1_16b.h"
#include "Drivers/uart0.h"
#include <stdlib.h>
#include <stdio.h>
#include <util/setbaud.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <inttypes.h>

using namespace std;
//*******************************************************
//SERVO
/*	
	TC1 Clk = 16e6/64 = 250000UL (4us period)
	3001HB: SERVO_MIN 97(388us/4us), SERVO MAX 535 (2140us/4us)
	TGY-R5180MG: MAX RANGE 595(2380us) MIN RANGE 177 (708us)
		Values Used: SERVO_MIN will be our 0 deg and SERVO MAX will be our 180 deg 
 */
 #define SERVO_MIN 245	
 #define SERVO_MAX 505

 uint8_t position[6] = {0};

 int getInt(volatile unsigned char *position);
 int getServoposition(int);


int main(void)
{
	//initiate UART0
	uart0_int();

	//initiate Timer1
	initateTimer1();
	//**************************************************
	//SERVO 

	//intial value 		
	OCR1A = SERVO_MIN;
	OCR1B = SERVO_MIN;
	OCR1C = SERVO_MIN;

    while (1) 
    {
		uint8_t servo;
    	int position_int = 0;

    	printm_0("\r\nPlease Enter Servo you Wish to Move(1-3):");
    	servo = getChar_0();
		print_int_0((servo - 0x30));
		UART_0_flush();

    	printm_0("\r\nPlease Enter at what angle to move it to (between 0 - 180):");
    	scanm_0(position);
		putChar_0(' ');
		print_int_0(getInt(position));

    	position_int = getServoposition(getInt(position));

		printm_0("\r\nAngle Converted to a desired Duty Cycle:");
		print_int_0(position_int);

    	printm_0("\r\n");

    	switch(servo)
    	{
    		case '1':
    		{
    			if(position_int < SERVO_MIN)
					OCR1A = SERVO_MIN;
				else if(position_int > SERVO_MAX)
					OCR1A = SERVO_MAX;
				else 
					OCR1A = (uint16_t)position_int;
					
    			break;
    		}
    		case '2':
    		{
    			if(position_int < SERVO_MIN)
					OCR1B = SERVO_MIN;
				else if(position_int > SERVO_MAX)
					OCR1B = SERVO_MAX;
				else
					OCR1B = position_int;

    			break;
    		}
    		case '3':
    		{
    			if(position_int < SERVO_MIN)
					OCR1C = SERVO_MIN;
				else if(position_int > SERVO_MAX)
					OCR1C = SERVO_MAX;
				else
					OCR1C = position_int;

    			break;
    		}
    		default: printm_0("\r\nPlease enter a valid Servo\r\n");break;
    	}

    }
}
//Get the integer value of a 3 digit number 
int getInt(volatile unsigned char *position)
{
	int sum = 0;
	sum =  (position[0]-0x30)*100;
	sum += (position[1]-0x30)*10;
	sum += (position[2]-0x30);
	return sum;
}//getInt

//Based on the angle passed in return a duty cycle value
int getServoposition(int val){
	int servoVal = 0;
	servoVal = (int) ((((float)val)/180.0)*260.0)+245.0;
	return servoVal;
}//getServoposition

