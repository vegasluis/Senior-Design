/*
 * V01.c
 *
 * Created: 8/30/2017 7:31:48 PM
 * Author : Luis
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "Drivers/timer1_16b.h"
#include "Drivers/uart0.h"
#include "Drivers/i2c_master.h"
#include "Drivers/mpu6050_reg.h"
#include "Drivers/mpu6050.h"
#include "Drivers/adc.h"
#include "Drivers/TMP102.h"

//*******************************************************
//Initalize before main 
void startUp(void)__attribute__((naked))__attribute__((section(".init3")));
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

//*******************************************************
//Watchdog 
#define soft_reset()		\
do                  		\
{							\
	cli();					\
	wdt_disable();			\
	wdt_enable(WDTO_15MS);	\
	while(1);				\
}while(0)		
		

//*******************************************************
//SERVO
/*	
	TC1 Clk = 16e6/64 = 250000UL (4us period)
	3001HB: SERVO_MIN 97(388us/4us), SERVO MAX 535 (2140us/4us)
	TGY-R5180MG: MAX RANGE 595(2380us) MIN RANGE 177 (708us)
		Values Used: SERVO_MIN will be our 0 deg and SERVO MAX will be our 180 deg 
 */
	 #define SERVO_MIN 150	
	 #define SERVO_MAX 640

	//-----------------
	//Functions 
	void setServos(volatile char* readString);
	volatile uint16_t getAngle (char firstInt, char secondInt, char thirdInt);
	volatile uint16_t getServoPos(volatile  uint16_t angle);
	void LED_state (char servoNum, char on);
	void clearReadString(void);

	//------------------
	//Variables need for Servo 
	volatile uint8_t charCount = 0;
	volatile char readString[20] = {0}; 

//*******************************************************
//Accelerometer 
	//-------------
	//Functions 
	const int readAccel();

//*******************************************************
//Tempature 
	volatile int temp = 0;

//********************************************************
//Watchdog 
void initWatchdog(void); 

//Sleep Function
void enterSleep(void);
void aboutToSleep(void);

//Use to count a certain number of ticks
volatile int countTick  __attribute__ ((section (".noinit")));

//ADC
volatile double valueADC0,valueADC1,valueADC2;


//********************************************************
//MAIN 

int main(void)
{
	printm_0("Welcome!!!\r\n");
	
	//initate global interrupts
	sei(); 
	
	while(1);

	return 0;
}

//-------------------------------------------------------
//UART0_ ISR 
ISR (USART0_RX_vect)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		unsigned char receivedChar;
		receivedChar = UDR0;							// Read data from the RX buffer
		putChar_0(receivedChar);
		//_delay_ms(2);
		_delay_us(700);

		
		//READ in char and append to end char array
		/*
		 * Format to be read in:
		 * 1 1 000 2 1 000 3 1 000
		 * S L ANG S L ANG S L ANG
		 * S: Servo, L: LED State, ANG: Angle
		 */
		
		readString[charCount] = receivedChar;
		++charCount;
		 
		if(charCount >= 15 || receivedChar == '\0')
		{	
			printm_0("\r\n");   
			printm_0("Read String: ");
			printm_0(readString);
			printm_0("\r\n");

			if(receivedChar != '\0')
			{
				temp = getTemp();
				printm_0("Temp: ");
				print_int_0(temp);
				printm_0(" C*\r\n");
				if(temp > 50 || temp < -10)	
				{
					printm_0("AYY Caliente!!\r\n");
					aboutToSleep();
					initWatchdog();
					enterSleep();
				}	

				if(readAccel() != 1) setServos(readString); 
			}
			
			charCount = 0;
			clearReadString();
		}
		UART_0_flush();
	}
}

ISR(WDT_vect)
{
	soft_reset();
}

ISR(TIMER1_OVF_vect)
{ 
	++countTick;
	if(countTick > 1000)
	{	
		startADC0();
		waitADC();
		valueADC0 = (ADCW *5 / 1024.0);
		printm_0("Voltage 1 Level Read: ");
		print_double_0(valueADC0);
		printm_0("\r\n");

		startADC1();
		waitADC();
		valueADC1 = (ADCW *5 / 1024.0);
		printm_0("Voltage 2 Level Read: ");
		print_double_0(valueADC1);
		printm_0("\r\n");

		startADC2();
		waitADC();
		valueADC2 = (ADCW *5 / 1024.0);
		printm_0("Voltage 3 Level Read: ");
		print_double_0(valueADC2);
		printm_0("\r\n");

		//REMOVE AFTER TESTING
		//_delay_ms(1000);

		//If there is light outside the voltage leve increases as the resitance 
		//in the photo diode increases
		/*
			R2 = Resistor
			R1 = Photo Diode 
			Vin(R2/(R1+R2))
		*/
		if((valueADC0 > 1.50 && valueADC1 > 1.50) 
			|| (valueADC1 > 1.50 && valueADC2 > 1.50)
			 || (valueADC0 > 1.50 && valueADC2 > 1.50))
		{
			initWatchdog();
			aboutToSleep();
			enterSleep();
		}
		countTick = 0;
	}
}
//**********************
//Servo Function body Implementation 

/*
 *Arguments: a string consisting of 3 servos, 3 LEDs , and
 *3 distinct angles
 *Format of Argument String: 1 1 000 2 1 000 3 1 000
 */
void setServos(volatile char* readString)
{ 
	uint16_t servoPosVal = 0; 

 	//SERVO 1 
 	//readString[0] == Servo 1
 	//read String[1] == ON/OFF state
 	LED_state(readString[0],readString[1]);
 	servoPosVal = getServoPos(getAngle(readString[2],readString[3],readString[4]));
 	if(servoPosVal < SERVO_MIN) OCR1A = SERVO_MIN;
 	else if (servoPosVal > SERVO_MAX) OCR1A = SERVO_MAX;
 	else OCR1A = servoPosVal;

 	//SERVO2
 	//readString[5] == Servo 1
 	//read String[6] == ON/OFF state
 	LED_state(readString[5],readString[6]);
 	servoPosVal = getServoPos(getAngle(readString[7],readString[8],readString[9]));
 	if(servoPosVal < SERVO_MIN) OCR1B = SERVO_MIN;
 	else if (servoPosVal > SERVO_MAX) OCR1B = SERVO_MAX;
 	else OCR1B = servoPosVal;

 	//SERVO3
 	//readString[10] == Servo 1
 	//read String[11] == ON/OFF state
 	LED_state(readString[10],readString[11]);
 	servoPosVal = getServoPos(getAngle(readString[12],readString[13],readString[14]));
 	if(servoPosVal < SERVO_MIN) OCR1C = SERVO_MIN;
 	else if (servoPosVal > SERVO_MAX) OCR1C = SERVO_MAX;
 	else OCR1C = servoPosVal;
}
/* Return an integer value that represents and angle 
 * An angle between 0 - 180
 */
volatile uint16_t getAngle (char firstInt, char secondInt, char thirdInt)
{
	uint16_t sum = 0;
	sum =  (firstInt - 0x30)*100;
	sum += (secondInt - 0x30)*10;
	sum += (thirdInt - 0x30);
	return sum;
}
/* 
 * Argument: unsigned char, that represents an angle between 0-180
 * Return an integer that will represent a certain duty cycle
 */
volatile uint16_t getServoPos(uint16_t angle)
{
	uint16_t servoVal = 0;
	servoVal = (uint16_t) ((((float)angle)/180.0)*490.0)+150.0;
	return servoVal;
}
/*
 * Change the respeceted LED state
 */
void LED_state(char servoNum, char on)
{
	switch(servoNum){
		case '1': 
		{
			if(on == '1') PORTA |= (1<<PA0);
			else PORTA &= ~(1<<PA0);
			break;
		}
		case '2':
		{
			if(on == '1') PORTA |= (1<<PA1);
			else PORTA &= ~(1<<PA1);
			break;
		}
		case '3':
		{
			if(on == '1') PORTA |= (1<<PA2);
			else PORTA &= ~(1<<PA2);
			break;
		}
		default:printm_0("Servo Number does not exist\r\n");break;
	}

}

//**************
//Accelerometer Function body Implementation 

/* 
   Read the X,Y, and Z components off the accelerometer 
 */
const int readAccel() 
{

	int16_t accel_buff[3];
	double accelX, accelY, accelZ;
	//**********************************************
	//MPU6050

	//Read all axis of the accelerator's
	mpu6050_read_accel_ALL(accel_buff);
	mpu6050_read_accel_ALL(accel_buff);
	
	// acceleration (m/s^2)
	// Sensitivity 16384 LSB/g
	// +/- 2g full scale range
	accelX = accel_buff[0]*9.8*2/32768;
	accelY = accel_buff[1]*9.8*2/32768;
	accelZ = accel_buff[2]*9.8*2/32768;

	//printInfo
	printm_0("AccelX: ");
	print_double_0(accelX);
	printm_0("\r\n");
	printm_0("AccelY: ");
	print_double_0(accelY);
	printm_0("\r\n");
	printm_0("AccelZ: ");
	print_double_0(accelZ);
	printm_0("\r\n");


	/*
	 * Go in if not stable enough and return one 
	 * The Servo will stay a constant position until stable enough
	 * to do some image processing
	 */

	if(accelX >= 15 || accelX <= -15 || accelY >= 15 ||
		accelY <= -15 || accelZ >= 15 || accelZ <= -15)
	{
		printm_0("Put servo in standstill mode\r\n");
		PORTA |= (1<<PA0)|(1<<PA1)|(1<<PA2);
		/*
		 * Angles set later 
		 */
		OCR1A = getServoPos((uint16_t)45);
		OCR1B = getServoPos((uint16_t)90);
		OCR1C = getServoPos((uint16_t)125);
		return 1;
	}

	return 0;
}

/*
 * Function is intended to set all reg needed before main 
 */
void startUp(void)
{
	//Disable Watchdog and Reset the MCU Status Reg
	//Incase of any Resets
	mcusr_mirror = MCUSR;
	MCUSR = 0;
    wdt_disable();

	//**************************************************
	//UART0
	uart0_int();

	//If the System has been reset notify the other system
	if(mcusr_mirror > 0) printm_0("Reseting \r\n");
	
	//set interrupt flag UART0
	RX_INTEN_0();

	//**************************************************
	//ADC
	intiateADC();

	//**************************************************
	//TIMER/COUNTER1

	//initiate Timer1
	initateTimer1();

	//Initiate Timer/Counter 1 interrupts 
	OVERFLW_TC1();
 
	//**************************************************
	//SERVO

	//intial value 		
	OCR1A = SERVO_MIN;
	OCR1B = SERVO_MIN;
	OCR1C = SERVO_MIN;

	//LED
	//set as outputs
	DDRA |= (1<<PA0)|(1<<PA1)|(1<<PA2);

	//**************************************************
	//I2C

	//Iniate i2c protocol 
	i2c_init();

	//Set SCL and SDA as in Inputs
	DDRD &= (0<<PD0)|(0<<PD1); 

	//Pull UP 
	PORTD |= (1<<PD0)|(1<<PD1); 
	
	//**************************************************
	//MPU6050

	//initate MPU6050
	i2c_start(MPU6050_ADDRESS+I2C_WRITE); 
	mpu6050_init();

	//**************************************************

	countTick = 0;
} 

void initWatchdog(void)
{ 
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);  
} 

void enterSleep(void)
{
	cli();
	printm_0("About to go to Sleep\r\n");

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();

	sei();
	/* Now enter sleep mode. */
	sleep_cpu();

	sleep_disable(); /* First thing to do is disable sleep. */
	
	/* Re-enable the peripherals. */
  	power_all_enable();
}

void aboutToSleep(void)
{
	for(int i = 0; i < 10; ++i)
	{
		printm_0("inloop\r\n");
		PORTA |= (1<<PA0|1<<PA1|1<<PA2);
		_delay_ms(2500);
		PORTA = 0X0;
		_delay_ms(2500);
	}
	PORTA &= (0<<PA0)|(0<<PA1)|(0<<PA2);
}

void clearReadString(void)
{
	for(int i = 0; i < 20; ++i)
		readString[i] = "\0";
}

