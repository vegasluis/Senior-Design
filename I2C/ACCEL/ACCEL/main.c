/*
 * ACCEL.cpp
 *
 * Created: 8/27/2017 9:05:14 AM
 * Author : Luis
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#include <inttypes.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <avr/interrupt.h>
#include <math.h>
#include "Drivers/i2c_master.h"
#include "Drivers/mpu6050_reg.h"
#include "Drivers/uart0.h"
#include "Drivers/mpu6050.h"

int main(void){
  
	uart0_int();
	i2c_init();

	DDRD &= (0<<PD0)|(0<<PD1); 
	PORTD |= (1<<PD0)|(1<<PD1); 

	int16_t accel_buff[3];
	double accelX, accelY, accelZ;

	// initialize & test MPU5060 availability
	mpu6050_init();
	i2c_start(MPU6050_ADDRESS+I2C_WRITE);
	mpu6050_read_accel_ALL(accel_buff);

	while(1)
	{
		mpu6050_read_accel_ALL(accel_buff);
		
		// acceleration (m/s^2)
		// Sensitivity 16384 LSB/g
		// +/- 2g full scale range
		accelX = accel_buff[0]*9.8*2/32768;
		accelY = accel_buff[1]*9.8*2/32768;
		accelZ = accel_buff[2]*9.8*2/32768;

		
		printm_0("AccelX: ");
		print_double_0(accelX);
		printm_0("\r\n");
		printm_0("AccelY: ");
		print_double_0(accelY);
		printm_0("\r\n");
		printm_0("AccelZ: ");
		print_double_0(accelZ);
		printm_0("\r\n");
		_delay_ms(1000);
	}
	
}