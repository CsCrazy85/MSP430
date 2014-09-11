/*
 * USI-SPI.h
 *
 * 	### For description see USI-SPI.c and USI.h!!!
 *
 *  	Created on: 14.10.2013
 *      Author: Mika Väyrynen
 */
#include <msp430.h>

#ifndef USI_SPI_H_
#define USI_SPI_H_

// Precompiler check if MSP has USI module
#ifdef __MSP430_HAS_USI__
// Define slave select pin	(called SS or CS etc...)
#define SLAVE_SELECT			BIT4

// Character which ends the data transmission
#define ENDING_CHARACTER_DATA	'!'

// SPI maximum read count when no ending character is received.
#define MAX_RX_BYTES			15

// Function prototypes
void InitUSI();
unsigned char SPI_SendByte(unsigned char byte);
void SPI_Write(unsigned char *string);
void SPI_Read(unsigned char *readBuffer);


#else
	#error "Chip does not have USI module!"
#endif	// end of #ifdef __MSP430_HAS_USI__

#endif /* USI_SPI_H_ */
