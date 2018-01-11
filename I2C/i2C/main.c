/*
 * i2C.c
 */ 


#include <avr/io.h>
#include <util/twi.h>

#include "i2c_master.h"

#define F_CPU 16000000UL
#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

void i2c_init(void)
{
	TWBR = (uint8_t)TWBR_val;
}

uint8_t i2c_start(uint8_t address)
{
	// reset TWI control register
	TWCR = 0;

	/*
	  Send START condition
	*/
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	/*
	  Wait for TWINT Flag set. This indicates
	  that the START condition has been
	  transmitted.
	*/
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	/*
	 *Check value of TWI Status Register. Mask
	  prescaler bits. If status different from
	  START go to ERROR.
	*/

	// check if the start condition was successfully transmitted
	if((TWSR & 0xF8) != TW_START){ return 1; }
	
	/*Load SLA_W into TWDR Register. Clear
	  TWINT bit in TWCR to start transmission of
	  address.*/

	// load slave address into data register
	TWDR = address;
	// start transmission of address
	TWCR = (1<<TWINT) | (1<<TWEN);

	/* Wait for TWINT Flag set. This indicates
	that the SLA+W has been transmitted, and
	ACK/NACK has been received.*/
	while( !(TWCR & (1<<TWINT)) );
	

	/*
	  Check value of TWI Status Register. Mask
      prescaler bits. If status different from
      MT_SLA_ACK go to ERROR.
	*/

	// check if the device has acknowledged the READ / WRITE mode
	if ( ((TW_STATUS & TW_NO_INFO) != TW_MT_SLA_ACK) 
	&& ((TW_STATUS & TW_NO_INFO) != TW_MR_SLA_ACK)) 
	{return 1;}
	
	//NO ERROR
	return 0;
}

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

uint8_t i2c_read_ack(void)
{
	
	// start TWI module and acknowledge data after reception
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
}

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
	if (i2c_start(address | TW_WRITE)) return 1;
	
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
	if (i2c_start(address | TW_READ)) return 1;
	
	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();
	
	i2c_stop();
	
	//NO ERROR
	return 0;
}

uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c_start(devaddr | 0x00)) return 1;

	i2c_write(regaddr);

	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}

	i2c_stop();

	//NO ERROR
	return 0;
}

uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c_start(devaddr)) return 1;

	i2c_write(regaddr);

	if (i2c_start(devaddr | 0x01)) return 1;

	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();

	i2c_stop();

	//NO ERROR
	return 0;
}

void i2c_stop(void)
{
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}


int main(void)
{
	/* Replace with your application code */
	while (1);
}


