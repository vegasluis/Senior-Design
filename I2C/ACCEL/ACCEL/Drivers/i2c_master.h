#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#ifdef __cpluspls
extern "C" {
#endif


#define I2C_READ 1
#define I2C_WRITE 0

//use TW_READ and TW_WRITE
//the two methods just initate and stop communication

void i2c_init(void);
uint8_t i2c_start(uint8_t address);
void i2c_start_wait(unsigned char address);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length);
uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length);
uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
uint8_t i2c_readRegByte(uint8_t devaddr, uint8_t regaddr, uint8_t* data); 
uint8_t i2c_writeRegByte(uint8_t devaddr, uint8_t regaddr, uint8_t data);
void i2c_stop(void);

#ifdef __cpluspls
}
#endif

#endif // I2C_MASTER_H