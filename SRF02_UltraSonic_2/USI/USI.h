/*
 * USI.h
 *
 *	This folder contains the functions needed for using the USI module when
 *	using communication over SPI or I2C-bus.
 *
 * 	About:
 * 	It is recommended to set the correct device in the project settings,
 * 	because USI.h includes only <msp430.h> -library rather than a spesific device.
 *
 * 	This code is written on Windows 7 using Code Composer Studio v5. When
 * 	you change the code, it is recommended to ReBuild instead of Building or
 * 	do Clean -> Build.
 *
 *	Pinout for I2C:
 *	Master:
 *		 ______
 * 		|  P1.7|----- SDA
 * 		|  P1.6|----- SCL
 * 		|	   |
 *
 *
 * 	Pinout for SPI:
 * 	MASTER 			   			   SLAVE
 * 	 	 ______                        ______
 * 		|  P1.5|--------- SCLK -------|P1.5  |
 * 		|  P1.6|--------- MOSI -------|P1.7  |
 * 		|  P1.7|--------  MISO -------|P1.6	 |
 *	    |  P1.4|--------  CS# ------- |P1.4  | (CS# is user selectable)
 *
 *  	Created: 		18.1.2013
 *  	Modified: 		11.6.2014
 *      Author: 		Mika Väyrynen
 *      Contact:		mika_va@hotmail.com / www.hutasu.net
 * 		Version:		0.1	 		SPI: Functions for initialization and sending data created.	18.1.2013.
 *						0.2			SPI: Added SLAVE_SELECT bit to synchronize the communication.
 *						0.3			Added I2C Master. Separated the I2C and SPI into different files.
 *						0.4			Corrected some bugs, removed some unnecessary functions.
 *						0.5			Changes (11.6.2014):
 *						 			- Functionality of I2CMasterReadRegister() and I2CMasterWrite(). These functions
 *									now return a value if write was succesfull or not.
 *									- Added/improved some of the comments.
 *
 */
#include <msp430.h>

//------------------------------------------------------------------------------
// User: Select whether to use SPI or I2C
//------------------------------------------------------------------------------
#define COMMUNICATION		I2C


//------------------------------------------------------------------------------
// DON'T TOUCH BELOW THIS LINE --- unless you know what you are doing
//------------------------------------------------------------------------------

#define SPI		1
#define I2C		2

#ifndef USI_H_
#define USI_H_

#if COMMUNICATION == SPI
#include "USI-SPI.h"
#endif

#if COMMUNICATION == I2C
#include "USI-I2C.h"
#endif

#endif /* USI_H_ */
