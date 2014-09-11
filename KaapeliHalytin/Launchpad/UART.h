/*
 * UART.h
 *
 *	### For description see UART.c
 *
 *  Created on: 14.6.2012
 *      Author: Mika Väyrynen
 *      www.hutasu.net
 *      Finland
 */

#ifndef UART_H_
#define UART_H_

#include <msp430.h>

//------------------------------------------------------------------------------
// Operation-related definitions
//------------------------------------------------------------------------------
#define UART_MODE		1
#define DELAY_MODE		2

//------------------------------------------------------------------------------
// Optimization-related definitions
//------------------------------------------------------------------------------

// If low-power delay is needed uncomment line below
#define LP_DELAY
// If send operations are needed, uncomment line below
#define ACTIVATE_TX
// If receive operations are needed, uncomment line below
//#define ACTIVATE_RX
// if both above are needed, uncomment both.

//------------------------------------------------------------------------------
// Hardware-related definitions
//------------------------------------------------------------------------------

// macros for TX/RX or both
#ifdef ACTIVATE_TX
	#define UART_TXD   BIT1                     // TXD on P1.1 (Timer0_A.OUT0)
#endif
#ifdef ACTIVATE_RX
	#define UART_RXD   BIT2                     // RXD on P1.2 (Timer0_A.CCI1A)
#endif

//------------------------------------------------------------------------------
// Conditions for 9600 Baud SW UART, SMCLK = 1MHz
//------------------------------------------------------------------------------
#define UART_SPEED			9600
#define UART_TBIT_DIV_2     (1000000 / (UART_SPEED * 2))
#define UART_TBIT           (1000000 / UART_SPEED)

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
void TimerA_WaitForCharacter();
#ifdef LP_DELAY
void TimerA_msDelay(unsigned int ms);
void TimerA_Delay(unsigned char seconds);
#endif
void TimerA_UART_pinInit(void);
void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
void TimerA_UART_printHex(char hexValue);

#endif /* UART_H_ */
