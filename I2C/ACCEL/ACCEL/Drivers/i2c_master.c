#include <avr/io.h>
#include <util/twi.h>
#include "i2c_master.h"
#include <stdlib.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <util/delay.h>

#define F_SCL  100000L // SCL frequency
#define Prescaler 1
#define TWBR_val (((F_CPU / F_SCL) - 16 ) / 2)

//************************************************************
//************************************************************
//BODY OF Functions 

/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void)
{
	/* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */

	TWSR = 0; /*No prescalar*/
	TWBR = TWBR_val;
}
/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
uint8_t i2c_start(uint8_t address)
{
	// reset TWI control register
	TWCR = 0;
	// transmit START condition 
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN)|(1<<TWEA);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the start condition was successfully transmitted
	if((TWSR & 0xF8) != TW_START){ return 1; }
	
	// load slave address into data register
	TWDR = address;
	// start transmission of address
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
	
	return 0;
}

/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/
void i2c_start_wait(unsigned char address)
{
    uint8_t   twst;


    while (1)
    {
	    // send START condition
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    
    	// wait until transmission completed
    	while(!(TWCR & (1<<TWINT)));
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
    
    	// send device address
    	TWDR = address;
    	TWCR = (1<<TWINT) | (1<<TWEN);
    
    	// wail until transmission completed
    	while(!(TWCR & (1<<TWINT))) ;
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ) 
    	{    	    
    	    /* device busy, send stop condition to terminate write operation */
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	        
	        // wait until stop condition is executed and bus released
	        while(TWCR & (1<<TWSTO)) ;
	        
    	    continue;
    	}
    	break;
     }

}/* i2c_start_wait */

/*************************************************************************
  Send one byte to I2C device
  
  Input:    byte to be transfered
  Return:   0 write successful 
            1 write failed
*************************************************************************/
uint8_t i2c_write(uint8_t data)
{
	/*
	  Load DATA into TWDR Register. Clear
	  TWINT bit in TWCR to start transmission of
	  data.  
	*/

	// load data into data register
	TWDR = data;
	// start transmission of data
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission

	/*
	Wait for TWINT Flag set. This indicates
 	that the DATA has been transmitted, and
	ACK/NACK has been received.
	*/
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the device has acknowledged the WRITE mode
	if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
	
	//NO ERROR
	return 0;
}
/*************************************************************************
 Read one byte from the I2C device, request more data from device 
 
 Return:  byte read from I2C device
*************************************************************************/
uint8_t i2c_read_ack(void)
{
	
	// start TWI module and acknowledge data after reception
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
}
/*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition 
 
 Return:  byte read from I2C device
*************************************************************************/
uint8_t i2c_read_nack(void)
{
	
	// start receiving without acknowledging reception
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
}

uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length)
{
	i2c_start_wait(address+I2C_WRITE);
	
	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}
	
	i2c_stop();
	
	//NO ERROR
	return 0;
}

uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length)
{
	i2c_start_wait(address+I2C_READ);

	uint16_t i;
	for (i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[i] = i2c_read_nack();
	
	i2c_stop();
	
	//NO ERROR
	return 0;
}

// write multiple bytes to dev
uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	i2c_start_wait(devaddr+I2C_WRITE);
	if(i2c_write(regaddr))return 1;

	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}

	i2c_stop();

	//NO ERROR
	return 0;
}

// write multiple bytes to dev
uint8_t i2c_writeRegByte(uint8_t devaddr, uint8_t regaddr, uint8_t data)
{
	i2c_start_wait(devaddr+I2C_WRITE);
	if(i2c_write(regaddr))return 1;
	if(i2c_write(data))return 1;
	i2c_stop();
	return 0;
}
// read multiple bytes from dev
uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	i2c_start_wait(devaddr+I2C_WRITE);
	if(i2c_write(regaddr))return 1;

	if (i2c_start(devaddr + I2C_READ)) return 1;

	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();

	i2c_stop();

	//NO ERROR
	return 0;
}
// read multiple bytes from dev
uint8_t i2c_readRegByte(uint8_t devaddr, uint8_t regaddr, uint8_t* data)
{
	i2c_start_wait(devaddr+I2C_WRITE);				//start i2c to write register address
	if(i2c_write(regaddr)) return 1;				//write address of register to read
	if(i2c_start(devaddr+I2C_READ))return 1;		//restart i2c to start reading
	*data = i2c_read_nack();
    i2c_stop();	
    return 0;			
}
/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void i2c_stop(void)
{
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO)) ;
}
