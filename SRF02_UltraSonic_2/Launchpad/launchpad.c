/*
 * launchpad.c
 *
 * Initialization of the Launchpad and GPIO settings. Select defines what is used in
 * launchpad.h (leave them be or comment/uncomment selections).
 *
 *  Created on: 	26.12.2012
 *  Modified on:	12.6.2014
 *      Author: 	Mika Väyrynen / www.hutasu.net
 *      Version		1.0		(created)
 *      			1.1		Updates (12.6.2014):
 *      					- Found a bug when using XTAL. P2SEL was configured incorrectly then - fixed.
 *      					- Created a CheckOsc() function which is used to check the oscillator.
 */

#include <msp430.h>

#include "launchpad.h"

// Launchpad GPIO-init function.
void InitGPIO()
{
	// LED init
	#ifdef LEDS
		INIT_LEDS(LED_GREEN,LED_RED);
	#endif

#ifdef __MSP430_HAS_PORT1_R__
	P1DIR = BIT7 + BIT6 + BIT5 + BIT4 + BIT3 + BIT2 + BIT1 + BIT0;	// All outputs
	P1OUT = 0;
	P1REN = 0;
	P1IE = 0;
	P1IFG = 0;
	P1IES = 0;
	P1SEL = 0;

#endif

#ifdef __MSP430_HAS_PORT2_R__
	P2DIR =  BIT7 + BIT6 + BIT5 + BIT4 + BIT3 + BIT2 + BIT1 + BIT0;	// All outputs
	P2OUT = 0;
	P2IE = 0;
	P2REN = 0;
	P2IFG = 0;
	P2IES = 0;

	#ifndef XTAL_32KHZ	// Configure XTAL PINS to IO if crystal not used
	P2SEL = 0;
	#endif

#endif

#ifdef __MSP430_HAS_PORT3_R__
	P3DIR = 0xFF;
	P3OUT = 0;
	P3SEL = 0;
#endif

	// CPU Initialization
#ifndef CUSTOM_CPU_SPEED
	#if CPU_SPEED == 1
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_1MHZ + XT2OFF;
		DCOCTL = CALDCO_1MHZ;
	#elif CPU_SPEED == 8
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_8MHZ + XT2OFF;
		DCOCTL = CALDCO_8MHZ;
	#elif CPU_SPEED == 12
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_12MHZ + XT2OFF;
		DCOCTL = CALDCO_12MHZ;
	#elif CPU_SPEED == 16
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_16MHZ + XT2OFF;
		DCOCTL = CALDCO_16MHZ;
	#endif
#endif
		// Custom CPU speed
#ifdef CUSTOM_CPU_SPEED
		#error "Place YOUR CPU speed in this block. Remove #error after that, thank you :)"
#endif

	#ifdef XTAL_32KHZ
	BCSCTL3 = XCAP_3;
	IE1 |= OFIE;			// Enable Osc Fault -> causes an interrupt

	#endif

	#ifdef SWITCH
		 SWITCH_INIT(S2);
	#endif
}


#pragma vector=NMI_VECTOR
__interrupt void NMI(void)
{
	#ifdef XTAL_32KHZ
	int i;

	do
	{
		IFG1 &= ~OFIFG;                         // Clear OSCFault flag
		for (i = 0xFFF; i > 0; i--);            // Time for flag to set
		#ifdef LEDS
			LED_RED_TOGGLE;
		#endif
	}
	while (IFG1 & OFIFG);                     // OSCFault flag still set?
	IE1 |= OFIE;                              // Enable Osc Fault

	#ifdef LEDS
	LED_RED_OFF;
	#endif
	#endif
}


