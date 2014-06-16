/*
 * UART.c	(TimerA)
 *
 * This file contains the minimum functions needed to send and receive data
 * via timer serial port. These functions are tested on Launchpad (v1.4), and they should
 * work with newer Launchpads (v1.5) as well. Only jumpers for TXD and RXD are needed
 * to switch so that MSP430G2231 TXD outputs to RXD and vice versa.
 *
 * Different modes (TX, RX or both) can be chosen by uncommenting/commenting
 * ACTIVATE_RX and ACTIVATE_TX flags in UART.h file.
 * Below is code/RAM consumption
 * in different modes:
 *
 * 	Application:	Echo (one char)
 * 	Description: 	Echoes received character back to UART.
 * 	Mode:			TX and RX
 * 	Code/RAM:		510/54	(no optimizations)
 *
 *  Application:	Print (string)
 * 	Description: 	Prints '\n\rHello ' string to UART once every second.
 * 	Mode:			TX only + delay function
 * 	Code/RAM:		546/54	(no optimizations)
 *
 *  Application:	Led toggle
 * 	Description: 	Toggles led when character is received from UART.
 * 	Mode:			RX only
 * 	Code/RAM:		422/44	(no optimizations)
 *
 *  Source code is absolutely free to use with any project. Source codes' origin is
 * 	from TI example code and from web.
 *
 *	Known bugs:		Printing doesn't necessarely work when TimerA_UART_tx() is called
 *					For the first time. I have tried to modify the files but I haven't find
 *					out what's wrong. It may be that the software UART in the launchpad doesn't
 *					get the timings right and message is broken therefore. If someone finds a fix
 *					for this, please contact me: mika_va@hotmail.com
 *
 *  	Created on: 	14.6.2012
 *  	Modified:		18.10.2012
 *  	Contact:		mika_va@hotmail.com / www.hutasu.net
 *  	Version:		1.0	Created, source in working state
 *  					1.1 Added delay functions
 *  					1.2 Added function TimerA_UART_printHex()
 *
 */
#include "UART.h"
#include "launchpad.h"

//------------------------------------------------------------------------------
// Global variables used for UART communication
//------------------------------------------------------------------------------
#ifdef ACTIVATE_TX
	unsigned int txData;                        // UART internal variable for TX
    unsigned char txBitCnt = 0;					// bit counter variable
#endif
#ifdef ACTIVATE_RX
	unsigned char rxBuffer = 0;                     // Received UART character
#endif
#ifdef LP_DELAY
	unsigned char mode = UART_MODE;					// variable for separating the delay operation with uart-operationm default = UART
	unsigned int delay = 0;							// variable for delay operation
#endif

#ifdef LP_DELAY

//------------------------------------------------------------------------------
// Function waits in sleep mode while character is being received
//------------------------------------------------------------------------------
void TimerA_WaitForCharacter()
{
	TimerA_UART_init();
	_BIS_SR(LPM0_bits + GIE);	// wait in sleep mode for received data
}
#endif

#ifdef LP_DELAY

//------------------------------------------------------------------------------
// Function produces n second delay with Timer_A. After delay, UART-mode is
// set to be active again.
//------------------------------------------------------------------------------
void TimerA_Delay(unsigned char seconds)
{
	mode = DELAY_MODE;
	delay = seconds;
	//TACCTL1 &= ~(SCS + CM1 + CAP + CCIE); // UART deconfig
	#ifdef XTAL_32KHZ
		CCTL0 = CCIE;   // enable interrupt
		CCR0 = 32767;    // value for 1 second interrupt
		TACTL = MC_1 + TACLR + ID_0 + TASSEL_1;      // timer start, divider 1, upto CCR0 and ACLK.
		__bis_SR_register(LPM3_bits + GIE);// Enter LPM3
	#else
		CCTL0 = CCIE;   // enable interrupt
	#if CPU_SPEED == 1
			CCR0 = 1000;    // value for 1 millisecond interrupt
		#elif CPU_SPEED == 8
			CCR0 = 8000;    // value for 1 millisecond interrupt
		#elif CPU_SPEED == 12
			CCR0 = 12000;    // value for 1 millisecond interrupt
		#elif CPU_SPEED == 16
			CCR0 = 16000;	// value for 1 millisecond interrupt
		#else
		#error "Can't know what the CPU speed is, therefore CCR0 can't be set. Please modify this function."
	#endif
	TACTL = MC_2 + TACLR + ID_0 + TASSEL_2;      // timer start, divider 1, compare and SMCLK.
	__bis_SR_register(LPM0_bits + GIE);// Enter LPM0
#endif
	TimerA_UART_init();		// set UART active again
}

//------------------------------------------------------------------------------
// Function produces n millisecond delay with Timer_A. After delay, UART-mode is
// set to be active again.
//------------------------------------------------------------------------------
void TimerA_msDelay(unsigned int ms)
{
	mode = DELAY_MODE;
	delay = ms;
	#ifdef XTAL_32KHZ
		CCTL0 = CCIE;   // enable interrupt
		CCR0 = 33;    // value for 1 millisecond interrupt
		TACTL = MC_1 + TACLR + ID_0 + TASSEL_1;      // timer start, divider 1, upto CCR0 and ACLK.
		__bis_SR_register(LPM3_bits + GIE);// Enter LPM0
	#else
		CCTL0 = CCIE;   // enable interrupt
	#if CPU_SPEED == 1
		CCR0 = 1000;    // value for 1 millisecond interrupt
	#elif CPU_SPEED == 8
		CCR0 = 8000;    // value for 1 millisecond interrupt
	#elif CPU_SPEED == 12
		CCR0 = 12000;    // value for 1 millisecond interrupt
	#elif CPU_SPEED == 16
		CCR0 = 16000;	// value for 1 millisecond interrupt
	#else
	#error "Can't know what the CPU speed is, therefore CCR0 can't be set. Please modify this function."
	#endif
	TACTL = MC_2 + TACLR + ID_0 + TASSEL_2;      // timer start, divider 1, compare and SMCLK.
	__bis_SR_register(LPM0_bits + GIE);// Enter LPM0
	#endif
	TimerA_UART_init();		// set UART active again
}
#endif
//------------------------------------------------------------------------------
// Function configures the pins for UART-operation.
//------------------------------------------------------------------------------
inline void TimerA_UART_pinInit(void)
{
#ifdef ACTIVATE_TX
	#ifdef ACTIVATE_RX	// both functions
		P1SEL |= UART_TXD + UART_RXD;            // Timer function for TXD/RXD pins
		P1DIR |= UART_TXD;           			// Set TX pin to output
		P1DIR &= ~UART_RXD;						//set RXD to input
	#else	// only TX needed
		P1SEL |= UART_TXD;            			// Timer function for TXD pin
		P1DIR |= UART_TXD;						// TX output
		P1OUT |= UART_TXD;
	#endif
#endif
#ifndef ACTIVATE_TX	// no TX needed
	#ifdef ACTIVATE_RX	// if only RX needed
		P1DIR &= ~UART_RXD;						// RX-input
		P1SEL |= UART_RXD;            			// Timer function for RXD pin
	#endif
#endif
}
//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------
void TimerA_UART_init(void)
{
	TimerA_UART_pinInit();
	mode = UART_MODE;
	CCTL0 = CCIS0 + OUTMOD0 + CCIE + OUT; 	// Set signal, intial value TXD Idle as Mark, enable interrupts
#ifdef ACTIVATE_RX
    TACCTL1 = SCS + CM1 + CAP;      // Sync, Neg Edge, Capture, Int
#endif
    TimerA_UART_tx(0);
}

#ifdef ACTIVATE_TX
//------------------------------------------------------------------------------
// Outputs one byte using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_tx(unsigned char byte)
{
	// this site helped me a bit
	// http://www.msp430launchpad.com/2010/07/launchpads-example-project-ripped-open.html
	//CCTL0 &= ~CCIFG;
	//CCTL0 = OUT;                    // TXD Idle as Mark

	TACTL = TASSEL_2 + MC_2;            // SMCLK, continuous mode

	txBitCnt = 0xA;                     // Load Bit counter, 8 bits + ST/SP
	CCR0 = TAR;
	CCR0 += UART_TBIT;                  // Set time till first bit

	txData = byte;						// set byte which is sent
	txData |= 0x100;                 	// Add stop bit to TXByte (which is logical 1)
	txData = txData << 1;            	// Add start bit (which is logical 0)

	// Was: CCTL0 =  CCIS0 + OUTMOD0 + CCIE;
	CCTL0 = CCIS0 + OUTMOD0 + CCIE + OUT; 	// Set signal, intial value, enable interrupts
	_BIS_SR(LPM0_bits + GIE);			// Enter LPM0, Wait for TX completion
	TACTL = TASSEL_2 ;             		// SMCLK, timer off (for power consumption)
}

//------------------------------------------------------------------------------
// Prints a string over using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_print(char *string)
{
    while (*string) {
        TimerA_UART_tx(*string++);
    }
}

void TimerA_UART_printHex(char hexValue)
{
	unsigned char temp = 0;
	temp = hexValue & 0xF0;
	temp >>= 4;
	if(temp >= 10 ) // letter
	{
		temp += 55;
	}
	else
	{
		temp += 48;
	}
	TimerA_UART_tx(temp);
	temp = hexValue & 0x0F;
	//temp >>= 4;
	if(temp >= 10 ) // letter
	{
		temp += 55;
	}
	else
	{
		temp += 48;
	}
	TimerA_UART_tx(temp);
}
#endif
//------------------------------------------------------------------------------
// Timer_A UART - Transmit Interrupt Handler
//------------------------------------------------------------------------------
#ifdef ACTIVATE_TX
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
{
	#ifdef LP_DELAY
    if(mode == UART_MODE)
    {
	#endif
		TACCR0 += UART_TBIT;                    // Add Offset to CCRx
		if (txBitCnt == 0)  					// All bits TXed?
		{
			TACCTL0 &= ~CCIE;                   // All bits TXed, disable interrupt
			txBitCnt = 10;                      // Re-load bit counter
			_BIC_SR_IRQ(LPM0_bits);				// exit sleep mode
		}
		else
		{
			if (txData & 0x01)
			{
				TACCTL0 &= ~OUTMOD2;              // TX Mark '1'
			}
			else
			{
				TACCTL0 |= OUTMOD2;               // TX Space '0'
			}
			txData >>= 1;
			txBitCnt--;
		}
	#ifdef LP_DELAY
    }
    else
    {
    	//TACTL = MC_2 + TACLR + ID_0 + TASSEL_2;
    	if(--delay == 0)
    	{
    		__bic_SR_register_on_exit(LPM3_bits);
    	}
    }
	#endif
}
#endif
#ifdef ACTIVATE_RX
//------------------------------------------------------------------------------
// Timer_A UART - Receive Interrupt Handler
//------------------------------------------------------------------------------
#pragma vector = TIMERA1_VECTOR
__interrupt void Timer_A1_ISR(void)
{
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;

    switch (__even_in_range(TAIV, TAIV_TAIFG)) { // Use calculated branching
        case TAIV_TACCR1:                        // TACCR1 CCIFG - UART RX
            TACCR1 += UART_TBIT;                 // Add Offset to CCRx
            if (TACCTL1 & CAP) {                 // Capture mode = start bit edge
                TACCTL1 &= ~CAP;                 // Switch capture to compare mode
                TACCR1 += UART_TBIT_DIV_2;       // Point CCRx to middle of D0
            }
            else {
                rxData >>= 1;
                if (TACCTL1 & SCCI) {            // Get bit waiting in receive latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {             // All bits RXed?
                    rxBuffer = rxData;           // Store in global variable
                    rxBitCnt = 8;                // Re-load bit counter
                    TACCTL1 |= CAP;              // Switch compare to capture mode
                    __bic_SR_register_on_exit(LPM0_bits);  // Clear LPM0 bits from 0(SR)
                }
            }
            break;
    }
}
#endif
//------------------------------------------------------------------------------
