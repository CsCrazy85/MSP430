/*
 * 	MSP430G2452 USI I2C-Master Example
 *
 *	Eng: This code is an simple I2C-master example for MSP430G2452 MCU
 *		 for reading distance values from SFR02 ultrasonic rangefinder.
 *		 Chip is connected as shown below and the communication is done
 *		 with I2C-bus.
 *
 * 	Fin: Yksinkertainen esimerkki, kuinka SFR02 ultra‰‰nianturilta voidaan lukea
 * 		 et‰isyystietoa I2C-v‰yl‰n avulla.
 *
 * Note:
 * 		Change user settings whatever is needed and use the functions provided. It is recommended
 * 		to set the correct device in the project settings, because underlying USI-I2C includes only
 * 		<msp430.h> -library rather than a specific device.
 *
 *		32kHz crystal is required!
 * About:
 * 		This code is written on Windows 7 using Code Composer Studio v5. When
 * 		you change the code, it is recommended to ReBuild instead of Building or
 * 		do Clean -> Build.
 *
 *
 *		MASTER 						  			SLAVE
 *				               GND
 *				            	|
 *		MSP430G2452	           | | 22k            SFR02
 * 	 	 ______                | |                ______
 * 		|      |             	|		+5V------|      |
 * 		|  P1.7|----------------^---- SDA -------|      |
 * 		|  P1.6|----------------,---  SCL -------|      |
 *	    |      | 	            |	  GND -------|      |
 *	 	|	   |	           | |   			 |______|
 *	 	|      |               | | 22k
 *	 	|      |                |
 *	 	|      |                 `-----GND
 *		|______|
 *
 *	Note! Resistors are important since they protect the MCU from 5V. 22k resistors cause
 *		  datalines to go about 3,3 volts when not pulled to ground. +5V is taken
 *		  from Launchpad (USB-Voltage).
 *
 *	Huom! Vastukset ovat t‰rkeit‰, koska SFR02 k‰ytt‰‰ 5 voltin logiikkaj‰nnitett‰.
 *	      N‰ill‰ vastuksilla lasketaan j‰nnite 3,3 voltin tasolle ja suojataan MCU:ta. 5 voltin
 *	      j‰nnite on otettu Launchpadilta (USB-j‰nnite).
 *
 *
 * 	Tarkemmat tiedot / More information:
 *	http://www.hutasu.net/index.php?sivu_id=119&parent=8
 *
 **/

#include <msp430.h> 
#include "Launchpad/launchpad.h"
#include "USI/USI.h"
#include "Launchpad/UART.h"
#include "printf/printf.h"
#include "SFR02/SFR02.h"

// extern sanalla kerrotaan k‰‰nt‰j‰lle, ett‰ struktuuri sijaitsee toisessa tiedostossa
extern t_SFR02		SFR02_firstDevice;		// luodaan struktuurista ik‰‰nkuin muuttuja

int main(void)
{
	UINT8 online = 0;		// k‰ytet‰‰n t‰llaisia tyypityksi‰, jolloin ohjelman siirrett‰vyys laitealustalta toiseen kasvaa
	UINT16 range = 0;		// UINT8 = unsigned char (8-bit), UINT16 = unsigned int (16-bit)
	UINT8 deviceCount = 0;

	// vahtikoiratimerin pys‰ytys
	WDTCTL = WDTPW | WDTHOLD;

	// Pinnien alustus
	InitGPIO();

    // sallitaan globaali keskeytys
    __enable_interrupt();

    // Uartin alustus
    TimerA_UART_init();

	// Usin alustus
	InitUSI();

	// Tulostetaan ett‰ ollaan valmiita
	printf("Master Ready!\n\r");

	// Kun tiedet‰‰n osoite, voidaan tarkistaa se suoraan n‰in:
	if(SFR02_CheckDevice(SFR02_ADDRESS) == 1)
	{
		P1OUT |= BIT5;	 // t‰h‰n on kytketty vihre‰ LED hyppylangalla

		SFR02_CollectDeviceData(SFR02_ADDRESS);	// Ker‰t‰‰n t_SFR02 -struktuuriin tietoa anturista

		online = 1;	// merkit‰‰n ett‰ anturi on onlinessa
	}
	else
	{
		// t‰m‰ aiheuttaa ohjelman resetoitumisen ja alkamisen alusta jos anturia ei lˆydy
		WDTCTL = 0xFFFF;
	}

	// Toisaalta jos anturin osoitetta ei tiedet‰, niin se voidaan skannata
	// SFR02_CollectDevices() -funktion avulla. Funktio etsii v‰yl‰lt‰ kaikki mahdolliset osoitteet,
	// joita SFR02 anturiin on voitu ohjelmoida ja palauttaa montako laitetta se lˆysi.
	deviceCount = SFR02_CollectDevices();

	printf("Found %i devices.\n\r",deviceCount);

    while(1)
	{
    	// Asetetaan anturi laskemaan et‰isyys kohteeseen. T‰m‰ funktio vain "liipaisee" anturin
    	// mittauksen ja t‰st‰ poistutaan heti kun "liipaisu" on tehty. Funktio palauttaa 1 kun
    	// anturin "liipaisu" on tehty onnistuneesti, eli anturi on ollut v‰yl‰ll‰.
    	// Funktio palauttaa 0, kun anturia ei jostain syyst‰ lˆydykk‰‰n.
    	online = SFR02_StartRanging(CENTIMETERS,SFR02_ADDRESS);

    	if(online == 0)
    	{
    		// t‰m‰ aiheuttaa ohjelman resetoitumisen ja alkamisen alusta
    		WDTCTL = 0xFFFF;
    	}


    	// Odotetaan ett‰ et‰isyys mittautuu
    	TimerA_msDelay(70);

    	// Ker‰t‰‰n tiedot anturilta talteen
    	SFR02_CollectDeviceData(SFR02_ADDRESS);

    	// Tulostetaan anturin kaikki tiedot
    	printf("Address = %x\n\r",SFR02_ADDRESS);
    	printf("Software Rev. = %i\n\r", SFR02_firstDevice.SW_revision);

    	// Lasketaan kahdesta 8-bitin ryhm‰st‰ 16-bittinen luku joka kertoo et‰isyyden.
    	// Struktuurin tietoja voidaan lukea alla olevalla tavalla.
    	range = SFR02_firstDevice.RangeLowByte + SFR02_firstDevice.RangeHighByte;

    	// Et‰isyyden tulostus
    	printf("Detected range: %i cm\n\r", range );

    	// Et‰isyys toisella tavalla
    	range = SFR02_GetRange(CENTIMETERS, SFR02_ADDRESS);
    	printf("Detected range alt: %i cm\n\r", range );

    	// Autotune rekisteri, eli minimi et‰isyys joka voidaan lukea
    	range = SFR02_firstDevice.AutotuneHighByte + SFR02_firstDevice.AutotuneLowByte;
    	printf("Range min: %i cm\n\r", range);

    	// Nukutaan hieman
    	TimerA_msDelay(1000);

		// Tyhjennet‰‰n terminaaliohjelman ruutu
		TimerA_UART_tx(27); // ESC
		printf("[2J");
		TimerA_UART_tx(27);	// ESC
		printf("[H");
	}
}

// Portin 2 keskeytysvektori
#pragma vector=PORT2_VECTOR
__interrupt void ISR_PORT2(void)
{

}


// AD-muunnin keskeytysvektori
#pragma vector=ADC10_VECTOR
__interrupt void ADC_ISR(void)
{

}

// Timer_A2 Interrupt Vector (TAIV) handler
// TimerA1 keskeytykset ja CCR1 ohjautuu t‰nne. Printtaus menee
// TIMER0_A0_VECTOR keskeytykseen joka on UART.c:ss‰.
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)
{

}
