/*
 * USI-I2C.h
 *
 * About:
 * It is recommended to set the correct device in the project settings,
 * because USI-I2C includes only <msp430.h> -library rather than a spesific device.
 *
 * This code is written on Windows 7 using Code Composer Studio v5. When
 * you change the code, it is recommended to ReBuild instead of Building or
 * do Clean -> Build.
 *
 *  	Created on: 	14.10.2013
 *      Author: 		Mika Väyrynen
 *      Version:		0.1
 *      Contact:		mika_va@hotmail.com / www.hutasu.net
 */
#include <msp430.h>

#ifndef USI_I2C_H_
#define USI_I2C_H_

// Precompiler check if MSP has USI module
#ifdef __MSP430_HAS_USI__


typedef enum e_I2C_States
{
	START = 0,
	RECEIVE_ADDR_NACK = 2,
	PROCESS_ADDR_NACK = 4,
	SEND_DATA_NACK = 6,
	PREPARE_STOP = 8,
	RECEIVE_DATA_NACK = 10,
	PROCESS_DATA_NACK = 12,
	STOP = 14

}I2C_States;


void InitUSI();

char CheckForSlaveExists(char address);
static void Setup_USI_Master(void);

unsigned char I2CMasterWrite(unsigned char address, unsigned char *data, unsigned int bytes);
void I2CMasterTransmitWithoutAddress(unsigned char *data, unsigned int bytecount);
void I2CMasterReceive(unsigned char address, unsigned char *data, unsigned int bytes);
unsigned char I2CMasterReadRegister(unsigned char address, unsigned char reg, unsigned char *data, unsigned int bytes);


static void InitUSIGpioI2C();
static void LoadStopBitAndSetStateSTOP();
static void Data_TX (void);
#else
	#error "Chip does not have USI module!"
#endif	// end of #ifdef __MSP430_HAS_USI__

#endif /* USI_I2C_H_ */
