/*
 * 	USI-SPI.c
 *
 *	This file contains the functions needed for using the USI module when
 *	creating communication over SPI-bus.
 *
 *
 *  Version:	0.1	 		SPI: Functions for initialization and sending data created.	18.1.2013.
 *				0.2			SPI: Added SLAVE_SELECT bit to synchronize the communication.
 *				0.3			Added I2C.
 *
 *	Created on: 18.1.2013
 *	Author: Mika Väyrynen
 *  www.hutasu.net
 *  Finland
 */
#include "USI.h"
#include "../Launchpad/launchpad.h"

#if COMMUNICATION == SPI
// Init USI for SPI MASTER mode
void InitUSI()
{
	P1DIR |= SLAVE_SELECT;			// CS-pin output
	P1OUT |= SLAVE_SELECT;			// set to "1"
	USICTL0 |= USISWRST;				// Reset module
	USICTL0 |= USIPE5	+ USIMST + USILSB + USIOE;	// enable output, MISO,MOSI and SCLK, SPI master & LSB first
	USICTL1 = USIIE;					// Enable interrupt for low power, Clock Phase = 0, Clock Polarity = 0
	USICKCTL = USISSEL_2 + USIDIV_0;	// SMCLK, DIV=1
	USICTL0 &= ~USISWRST; 				// release reset
	// MSP430G2231 Errata SPI-problem fix
	USICNT = 1;	 			// clear first transmit
	__delay_cycles(50);		// wait
	USICTL0 |= USIPE7 + USIPE6;	// enable MOSI,MISO
	USICTL1 &=  ~USIIFG;		// clear flag
}
// Sends and receives one byte
unsigned char SPI_SendByte(unsigned char byte)
{
	USISRL = byte;				// load byte
	P1OUT &= ~SLAVE_SELECT;			// set to "0"
	__delay_cycles(30);			// wait for slave
	USICNT = 8;					// start transmission
	__bis_SR_register(LPM0_bits + GIE);	// Enter LPM0 w/ interrupt
	P1OUT |= SLAVE_SELECT;			// set to "1"
	__delay_cycles(20);			// wait between bytes
	return USISRL;				// return received byte
}
void SPI_Write(unsigned char *string)
{
	LED_RED_ON;
	while(*string)
	{
		SPI_SendByte(*string++);
	}
	LED_RED_OFF;
}
void SPI_Read(unsigned char *readBuffer)
{
	unsigned char rxByte = 0;
	char rxCount = MAX_RX_BYTES - 1;
	LED_GREEN_ON;
	do
	{
		// read until ending character is received
		rxByte = SPI_SendByte(0);
		*readBuffer++ = rxByte;
		if(rxCount-- <= 0)
		{
			return;
		}
	}
	while(rxByte != ENDING_CHARACTER_DATA);

	LED_GREEN_OFF;
}
// Interrupt handler
#pragma vector=USI_VECTOR
__interrupt void universal_serial_interface(void)
{
	// Clear interrupt flag and exit
	USICTL1 &= ~USIIFG;
	__bic_SR_register_on_exit(LPM3_bits);
}
#endif
