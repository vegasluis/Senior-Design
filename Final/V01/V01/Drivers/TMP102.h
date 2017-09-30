#ifndef TMP102_H 
#define TMP102_H

#include <avr/io.h>
#include <inttypes.h>
#include "i2c_master.h"
#include <stdio.h>

#define TMP102_ADDRESS 0b10010000
//ADDR0 is assumed to be Ground

int getTemp()
{
	uint8_t MSB,LSB;
	MSB = LSB = 0;
	int Tempature = 0;

	//Start Communication with TMP102
	i2c_start(TMP102_ADDRESS+I2C_WRITE); 

	//Most Significant Byte
	i2c_readRegByte(TMP102_ADDRESS,TMP102_ADDRESS,&MSB);

	//Least Signifiicant Byte
	i2c_readRegByte(TMP102_ADDRESS,TMP102_ADDRESS,&LSB);

	Tempature = (MSB <<8) | LSB;
	Tempature >>= 4;

	if(Tempature &(1<<11))
		Tempature|=0xF800;

	Tempature /=16;

	return Tempature;
}

#endif 