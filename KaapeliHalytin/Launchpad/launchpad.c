/*
 * launchpad.c
 *
 *  Created on: 26.12.2012
 *      Author: Mika Väyrynen
 */

#include <msp430.h>

#include "launchpad.h"

// Launchpad alustusfunktio alustaa vain ledit, kiteen ja kytkimen jos ne on valittu launchpad.h tiedostosta
inline void InitGPIO()
{
	#ifdef LEDS
		INIT_LEDS(LED_GREEN,LED_RED);
	#endif

#ifdef __MSP430_HAS_PORT1_R__
	// Portti 1 alustus
	P1DIR = 0;
	P1DIR |= BIT7 + BIT6 + BIT4 + BIT1 + BIT0;	// BIT3 INT1-input, BIT2 AD-input
	P1OUT = BIT5;	// P1.5 on summeri
	// alustetaan myös nämä rekisterit
	P1REN = 0;
	P1IE = 0;
	P1IFG = 0;
	P1IES = 0;
	P1SEL = 0;

#endif

#ifdef __MSP430_HAS_PORT2_R__
	P2DIR = BIT6 + BIT7;
	P2OUT = BIT6 + BIT7;
	P2IE = 0;	//
	P2REN = 0;
	P2IFG = 0;
	P2IES = 0;	// nousevan reunan keskeytykset
	P2SEL = 0;
#endif

#ifdef __MSP430_HAS_PORT3_R__
	P3DIR = 0xFF;
	P3OUT = 0;
	P3SEL = 0;
#endif

	// alustetaan CPU:n kello 1 Mhz
	DCOCTL = 0x00;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	#ifdef XTAL_32KHZ
		 BCSCTL3 = XCAP_3;
	#endif

	#ifdef SWITCH
		 SWITCH_INIT(S2);
	#endif
}

