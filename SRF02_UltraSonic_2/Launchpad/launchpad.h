/*
 * launchpad.h
 *
 *	#### SEE launchpad.c for more information ####
 *
 *  Created on: 18.01.2013
 *      Author: Mika Väyrynen
 */

#ifndef LAUNCHPAD_H_
#define LAUNCHPAD_H_

// ================== USER SELECTABLE =============================

// =================== 32 kHz XTAL usage ==================
// Comment out XTAL_32KHZ if you don't want to use XTAL - this sets P2.6 & P2.7 as GPIO-pins.
// Note: In LED USAGE is activated, RED LED is used to flash error if XTAL is not working properly.
//		It may flash during the initialization, but that is normal. If LED keeps flashing,
// 		check XTAL hardware and CAPS (or XCAP-setting).

#define XTAL_32KHZ


// ============== LED USAGE =========================
// Comment out LEDS if you don't want to use LP leds

#define LEDS


// ============== SWITCH USAGE =========================
// Comment out SWITCH if it's not used

#define SWITCH


// ============== SELECT CALIBRATED CPU SPEED =========================
// Use values 1, 8, 12 or 16. If you want to set CPU-speed by yourself,
// you have to set this value to 0 and write your own initialization routine for that.

#define CPU_SPEED		1		// Default 1 MHz




// ####################### DO NOT EDIT BELOW THIS LINE ########################

// Launchpad LED defines
#define LED_RED                     BIT0
#define LED_GREEN                   BIT6
#define LEDS_OFF                    (P1OUT &= ~(LED_RED + LED_GREEN))
#define LED_RED_ON					(P1OUT |= LED_RED)
#define LED_RED_OFF					(P1OUT &= ~LED_RED)
#define LED_GREEN_ON				(P1OUT |= LED_GREEN)
#define LED_GREEN_OFF				(P1OUT &= ~LED_GREEN)
#define LED_RED_TOGGLE				(P1OUT ^= LED_RED)
#define LED_GREEN_TOGGLE			(P1OUT ^= LED_GREEN)
#define INIT_LEDS(L1,L2) 			{						\
										P1DIR |= (L1+L2);		\
										P1OUT &= ~(L1+L2);	\
									}

// Launchpad switch S2
#define S2							BIT3
#define SWITCH_INIT(s) 				{					\
									P1OUT |= s;		\
									P1REN |= s;		\
									P1DIR &= ~s;	\
									}

#define SWITCH_PRESSED				(!(P1IN & S2))

// Do some definition checking
#if CPU_SPEED==0
#define CUSTOM_CPU_SPEED
#endif

// Launchpad functions
void InitGPIO();
void CheckOsc();

#endif /* LAUNCHPAD_H_ */
