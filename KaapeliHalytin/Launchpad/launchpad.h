/*
 * launchpad.h
 *
 *  Created on: 18.01.2013
 *      Author: Mika V�yrynen
 */

#ifndef LAUNCHPAD_H_
#define LAUNCHPAD_H_

// Kiteen k�ytt�, kommentoi alla oleva jos kidett� ei haluta k�ytt��
//#define XTAL_32KHZ

// Ledien k�ytt�, kommentoi alla oleva jos ledej� ei haluta k�ytt��
//#define LEDS

// Kytkimen k�ytt�, kommentoi alla oleva jos ledej� ei haluta k�ytt��
//#define SWITCH

// Launchpad LED m��rityksi�
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

// Launchpadin kytkin S2
#define S2							BIT3
#define SWITCH_INIT(s) 				{					\
									P1OUT |= s;		\
									P1REN |= s;		\
									P1DIR &= ~s;	\
									}

#define SWITCH_PRESSED				(!(P1IN & S2))

// Launchpad funktiot
void InitGPIO();

#endif /* LAUNCHPAD_H_ */
