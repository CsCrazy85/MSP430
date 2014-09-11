
/*
 *		MASTER 						  MMA8452Q
 *		(Launchpad)
 * 	 	 ______                       ___________________
 * 		|  VCC |--------- VCC -------|1,14  VDDIO + VDD  |
 * 		|  P1.7|--------- SDA -------|6     SDA          |
 * 		|  P1.6|--------  SCL -------|4     SCL          |
 * 		|  P2.0|-------  INT1 ------ |11    INT1         |
 *	    |  P2.1|-------- INT2 -------|9     INT2         |
 *	 	|	GND|--------- GND -------|5,10,12 GND        |
 *	 	|      |--------- GND  ------|7,     SA0         |
 *	 	|      |                     |___________________|
 *	 	|      |
 *	 	|      |
 *		|______|
 */


#include <msp430.h> 

#include "Launchpad/launchpad.h"
#include "Launchpad/UART.h"
#include "printf/printf.h"
#include "USI/USI.h"
#include "Accelerometer/accelerometer.h"

typedef enum
{
	VLO,
	NORMAL
} t_CPUspeed;


// Tilakoneen tilat:
#define KAAPELI_KYTKETTY		1
#define KAAPELI_EI_KYTKETTY		2
#define KAAPELI_NOLLASSA				3
#define KAAPELI_VAIHEESSA				4
#define KAAPELI_EI_MITATTU				5
#define KAAPELI_NOLLASSA_JA_VAIHEESSA	6

// montako AD-muunnosta otetaan yhdell‰ lukukerralla
#define AD_MUUNNOKSET_LKM		10

// TODO: tarkista ja kommentoi tai poista
#define AD_KYNNYSARVO			100		// 350 mV

#define VAIHE_KYTKETTY					600		// raja-arvo jolla m‰‰ritell‰‰n onko vaihe kytketty vai ei
#define NOLLA_JA_VAIHE_KYTKETTY			750		// raja-arvo jolla m‰‰ritell‰‰n onko nolla + vaihe kytketty vai ei
#define NOLLA_KYTKETTY					830		// raja-arvo jolla m‰‰ritell‰‰n onko nolla kytketty vai ei

#define KIIHTYVYYSANTURIN_KYNNYSARVO	200		//
#define AIKA_MUUNNOSTEN_VALILLA_MS		6			// aikav‰li joka pidet‰‰n muunnosten v‰lill‰

volatile unsigned int globalSecondsCounter = 0;

// AD-muuntimeen liittyv‰t
unsigned int ADCvalues[AD_MUUNNOKSET_LKM];
volatile unsigned char ADCvaluesIndex = 0;

void SetCPUspeed(t_CPUspeed speed);
void SleepUntilWakenVLO(unsigned int secondsCounter);
int GetADCvalue(void);

int main(void)
{
	unsigned char test = 0x00;
	unsigned char anturinKeskeytys = 0;
	unsigned int ohjelmanTila = KAAPELI_EI_KYTKETTY;	// aloitetaan startista
	unsigned int adJannite = 0;
	unsigned int janniteMin = 1023;			// alustetaan 1023 jolloin automaattisesti etsit‰‰n t‰t‰ pienemp‰‰ lukua
	unsigned int janniteMax = 0;			// alustetaan 0 jolloin automaattisesti etsit‰‰n t‰t‰ suurempaa lukua

	//signed int xyz_anturit[3] = {0,0,0};

	signed int x_anturi = 0;
	signed int y_anturi = 0;
	signed int z_anturi = 0;

	unsigned char reg = 0;

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Alustetaan portit
    InitGPIO();

    // alustetaan USI
    InitUSI();

    // sallitaan globaali keskeytys
    __enable_interrupt();

	// alustetaan UART l‰hetys jos se on valittu launchpad.h tiedostossa aktiiviseksi
	TimerA_UART_init();


    while(1)
    {
    	// tarkistetaan onko AD-inputissa j‰nnitett‰. T‰h‰n on kytketty testik‰ytˆss‰ potentiometri, jolla voidaan
    	// s‰‰t‰‰ j‰nnitett‰ 0 - VCC v‰lill‰. T‰h‰n voi kytke‰ oikean kytkenn‰n joka ilmaisee kaapelin.
    	// HUOM1: T‰m‰n input on P1.4, mutta sen voi vaihtaa halutessaan!
    	// HUOM2: GetADCvalue() ottaa AD_MUUNNOKSET_LKM verran n‰ytteit‰ ja pit‰‰ v‰liss‰
    	// AIKA_MUUNNOSTEN_VALILLA_MS pituisen tauon LPM0 -tilassa.

    	/*
    	while(SWITCH_PRESSED == 0)			// kytkimell‰ p‰‰st‰‰n eteenp‰in. t‰m‰n luupin voi poistaa (testin vuoksi t‰ss‰)
    	{
    		adJannite = GetADCvalue();	// otetaan 10 muunnosta...
    		printf("%i\n\r",adJannite);	// tulostetaan viimeisin lukema
    	}
    	*/

    	// otetaan 10 muunnosta... (j‰tetty kommenttiin tarkoituksella)
    	GetADCvalue();

    	// Otetaan maksimi ja minimi talteen luetuista AD-muuntimen arvoista.
    	for(reg = 0; reg < AD_MUUNNOKSET_LKM; reg++)
    	{
    		// hyv‰ksyt‰‰n tarkastelun kohteeksi vain nollaa suuremmat luvut
    		if(ADCvalues[reg] > 0)
    		{
				// otetaan minimi talteen
				if(janniteMin > ADCvalues[reg])
				{
					// janniteMin suurempi kuin meneill‰‰n oleva arvo
					janniteMin = ADCvalues[reg];
				}
				// otetaan maksimi talteen
				if(janniteMax < ADCvalues[reg])
				{
					janniteMax = ADCvalues[reg];
				}
    		}
    	}

    	// v‰litulostus
    	//printf("Min: %i, Max: %i\n\r",janniteMin,janniteMax);


    	// P‰‰tet‰‰n mik‰ tila tulee kyseeseen AD-muuntimen j‰nnitteen perusteella
    	if(janniteMin < VAIHE_KYTKETTY)	// VAIHE_KYTKETTY = 600
    	{
    		// Kaapelin vaihe on kytketty --> kaapeli on kiinni mutta ei s‰hkˆiss‰
    		ohjelmanTila = KAAPELI_VAIHEESSA;

    		printf("KAAPELI_VAIHEESSA\n\r");
    		if(janniteMax < NOLLA_JA_VAIHE_KYTKETTY)	// NOLLA_JA_VAIHE_KYTKETTY = 750
    		{
    			// Kaapeli on t‰ysin kytketty eli l‰mmitys on p‰‰ll‰
    			ohjelmanTila = KAAPELI_NOLLASSA_JA_VAIHEESSA;
    			printf("KAAPELI_NOLLASSA_JA_VAIHEESSA\n\r");
    		}
    	}
    	else
    	{
    		// j‰nnite on suurempi kuin VAIHE_KYTKETTY.

    		if(janniteMin < NOLLA_KYTKETTY)	// NOLLA_KYTKETTY = 830
    		{
    			// Kaapelin nolla on kytketty --> kaapeli on kiinni mutta ei s‰hkˆiss‰
    			ohjelmanTila = KAAPELI_NOLLASSA;
    			printf("KAAPELI_NOLLASSA\n\r");
    		}
    		else
    		{
    			// janniteMin on suurempi kuin 830 joka tarkoittaa ett‰ ???
    			ohjelmanTila = KAAPELI_EI_KYTKETTY;
    			printf("KAAPELI_EI_KYTKETTY\n\r");
    		}

    	}

    	// Aktivoidaan 1 MHz kello
		SetCPUspeed(NORMAL);

		// Kiihtyvyysanturi p‰‰lle
		P1OUT |= BIT4;


		while( CheckForSlaveExists(0x38) == 0);

		// alustetaan kiihtyvyysanturi
		InitializeAccelerometer();

		while(1)
		{
			ReadAxisAll(&x_anturi,&y_anturi,&z_anturi); // vie aikaa: 2,59 ms kun I2C kello on 250 kHz
		}
    	// P‰‰ohjelman tilakone joka p‰‰tt‰‰ mit‰ tehd‰‰n
		switch(ohjelmanTila)
		{
		case KAAPELI_NOLLASSA:
			// kaapeli on kiinni, mutta ei s‰hkˆss‰
		case KAAPELI_VAIHEESSA:
			// kaapeli on kiinni, mutta ei s‰hkˆss‰

			// TODO: tutki kiihtyvyytt‰: voidaanko hyˆdynt‰‰ kiihtyvyysanturin transient
			// h‰lytyst‰ kiihtyvyyden toteamiseen, vai mitataanko kaikki akselit k‰sin ja vertaillaan
			// johonkin arvoon? J‰lkimm‰isess‰ se huono puoli, ett‰ v‰ltt‰m‰tt‰ ei tiedet‰ miten
			// p‰in laite on asennettu, joten yhdell‰ akselilla voi olla kiihtyvyytt‰ 1g ja muilla 0g.
			// T‰m‰n vuoksi tarvittaisiin ensin kalibroida laite sis‰isesti, eli ottaa asento talteen
			// ja vertailla asentotietoihin kiihtyvyysanturin nykyisi‰ arvoja. Eli tehd‰ k‰sipelill‰
			// t‰rin‰n tunnistus. T‰m‰ ei kai ole j‰rkev‰‰ jos kiihtyvyysanturissa itsess‰‰n on t‰m‰
			// ominaisuus???

			// odotellaan keskeytyst‰
			LPM3;

			LED_RED_ON;

			// Luetaan mik‰ anturin keskeytys oli kyseess‰, jotta INT-pinnin tila nollautuu
			anturinKeskeytys = RegisterRead(INT_SOURCE,0,1);

			/*
			if(anturinKeskeytys & SRC_TRANS)
			{
				printf("Transient!\n\r");
			}
			*/

			LED_RED_OFF;
			// K‰sitell‰‰n mik‰ anturin keskeytys oli kyseess‰

			switch(anturinKeskeytys)
			{
			case BIT0: 	// SRC_DRDY
				break;
			case BIT2: 	// SRC_FF_MT
				break;
			case BIT3:	// SRC_PULSE
				break;
			case BIT4:	// SRC_LNDPRT
				break;
			case BIT5:	// SRC_TRANS
				printf("Transient!\n\r");
				break;
			case BIT6:	// SRC_FIFO
				break;
			case BIT7:	// SRC_ASLP
				break;
			}

			// TODO: INT2 ei nollaudu vaan LED j‰‰ palamaan???



			break;
		case KAAPELI_NOLLASSA_JA_VAIHEESSA:
				// kun kaapeli on kytketty, voidaan tarkastella kiihtyvyysanturin arvoa.
				// Nyt ei ole huolta s‰hkˆn kulutuksesta kun otetaan tˆpselist‰ elektroneja

				// TODO: FET-ohjaus kiihtyvyysanturille
				// Aloittaa kiihtyvyyden mittauksen. Vaihtoehtoisesti t‰ss‰ kohtaa voisi menn‰ nukkumaan ja
				// her‰t‰ kun kiihtyvyysanturi on havainnut liikett‰ (porttipinnin keskeytys)
				SetAcceleroActive();

				while(1)
				{


					ReadAxisAll(&x_anturi,&y_anturi,&z_anturi); // vie aikaa: 2,59 ms kun I2C kello on 250 kHz
					LED_RED_ON;
					printf("%i,%i,%i\n\r",x_anturi,y_anturi,z_anturi); // t‰m‰ kest‰‰ 22,5 ms
					LED_RED_OFF;

					/*
				// tarkastetaan onko uusi validi arvo rekisteriss‰.
				reg = RegisterRead(STATUS,0,1);
				// printf("STATUS: %i\n\r",reg);


				if( (reg & 0x08) )	// TODO: poista maagiset numerot
				{

					 +4 bytea: 3496. Kokonaisaika lukuun ja tulostukseen: 2,67 + 22,25 ms
					LED_RED_ON;
					ReadAxisAll2(xyz_anturit);	// Vie aikaa: 2,67 ms kun I2C kello on 250 kHz
					LED_RED_OFF;
					LED_RED_ON;
					printf("%i,%i,%i\n\r",xyz_anturit[0],xyz_anturit[1],xyz_anturit[2]); // t‰m‰ kest‰‰ 22,5 ms
					LED_RED_OFF;

				} */
				}

				SetAcceleroStandby();	// laitetaan kiihtyvyysanturi standby-tilaan
			break;

			case KAAPELI_KYTKETTY:

				break;

			case KAAPELI_EI_KYTKETTY:

				LED_RED_ON;

				// Aktivoi 12 kHz:n kello ACLK:n ja CPU:n l‰hteeksi
				SetCPUspeed(VLO);

				// nukutaan noin 5 sekuntia ja toistetaan luuppia uudestaan.
				// VLO:n taajuus voi heitell‰ hyvinkin paljon 4 - 20 kHz: v‰lill‰, joten t‰m‰
				// ei ole mik‰‰n tarkka. VLO:N taajuuteen vaikuttaa l‰mpˆtila ja k‰yttˆj‰nnite.
				SleepUntilWakenVLO(5);

				// yll‰ olevasta funktiosta tullaan tietyn ajan kuluttua pois ja toistetaan p‰‰ohjelman luuppi

				LED_RED_OFF;

				break;
			default:
				break;
		}

    }

}

int GetADCvalue(void)
{
	// asetetaan P1.4 analog 4 -kanavaksi
	ADC10AE0 = BIT2;

	// Asetetaan kanava A2, valitaan kellosignaaliksi ADC10OSC (~5 MHz) ja asetetaan yksi muunnos
    ADC10CTL1 = INCH_2 + ADC10SSEL_0 + CONSEQ_0;

	// Oletuksena referenssij‰nnitteet ovat Vcc (1) ja Vss (14)
	// Asetetaan n‰ytteistysaika 64 kellojaksoa ja laitetaan ydin p‰‰lle.
    // Sallitaan keskeytys
    ADC10CTL0 = ADC10SHT_3 + ADC10ON + ADC10IE ;		// 64 us n‰ytteistysaika

    //TimerA_msDelay(1);
    // __delay_cycles(1000);

    // k‰ynnistet‰‰n muunnos
    ADC10CTL0 |= ENC + ADC10SC;

    // odotetaan sleepiss‰ ett‰ muunnos on valmis
	LPM0;

	return ADC10MEM;	// palautetaan mitatun kanavan arvo
}

void SetCPUspeed(t_CPUspeed speed)
{
	if(speed == VLO)
	{
		// VLO speed
		BCSCTL3 |= LFXT1S_2;                      // ACLK = VLO
		BCSCTL2 |= SELM_3 + SELS;                 // SMCLK  = MCLK = VLO = 12Khz
		__bis_SR_register(SCG1 + SCG0);           // Stop DCO
	}
	else
	{
		// Normal speed
		BCSCTL3 &= ~LFXT1S_2;                      // ACLK = SMCLK
		BCSCTL2 = SELM_0;                 		// SMCLK  = MCLK = DCO = 1 MHz
		__bic_SR_register(SCG1 + SCG0);           // Start DCO
	}
	IFG1 &= ~OFIFG;                           // Clear OSCFault flag
}

void SleepUntilWakenVLO(unsigned int secondsCounter)
{
	globalSecondsCounter = secondsCounter;
	// asetetaan CCR1 keskeytt‰m‰‰n ~1000 ms:n v‰lein
	CCTL1 = CCIE;							// sallitaan keskeytys

	// asetetaan keskeytys sekunnin v‰lein ja menn‰‰n nukkumistilaan.
	CCR1 = 12000;	// 12 kHz kellojakso suurinpiirtein, joten 12000 tiksi‰ = 1 sekunti

	// k‰ynnistet‰‰n timer nollasta, ACLK kellottaa ja jatkuvasti juokseva moodi
	TACTL = TASSEL_1 + MC_2 + TACLR;

	// nukutaan LPM3-tilassa kunnes ollaan nukuttu pyydetty m‰‰r‰ sekunteja (suurinpiirtein)
	_BIS_SR(LPM3_bits + GIE);

	// pys‰ytet‰‰n timer
	TACTL = 0;

	CCTL1 = 0;	// nollataan keskeytysasetus ettei satu mit‰‰n outouksia
}

#pragma vector=PORT2_VECTOR
__interrupt void ISR_PORT2(void)
{
	/*
	if(P2IFG & BIT0)
	{
		// accelerometer INT1
	}
	*/
	if(P2IFG & BIT1)
	{
		// accelerometer INT2
	}

	// keskeytyslipun nollaus
	P2IFG = 0;
	// poistutaan sleepista
	__bic_SR_register_on_exit(LPM3_bits);
}


// AD-muunnin keskeytysvektori
#pragma vector=ADC10_VECTOR
__interrupt void ADC_ISR(void)
{
	// asetetaan muunnos taulukkoon ja kasvatetaan indexin arvoa. Indexin ylivuodosta pit‰‰
	// huolehtia muualla!
	ADCvalues[ADCvaluesIndex++] = ADC10MEM;

	if(ADCvaluesIndex >= AD_MUUNNOKSET_LKM)
	{
		// nollataan index-muuttuja
		ADCvaluesIndex = 0;
		// Poistutaan sleepist‰
		__bic_SR_register_on_exit(LPM3_bits);
	}
	else
	{
		// odotellaan sleepiss‰ haluttu aika, ett‰ n‰ytev‰li on oikea
		TimerA_msDelay(AIKA_MUUNNOSTEN_VALILLA_MS);

		// k‰ynnistet‰‰n uusi muunnos
		ADC10CTL0 |= ENC + ADC10SC;
	}
}

// Timer_A2 Interrupt Vector (TAIV) handler
// TimerA1 keskeytykset ja CCR1 ohjautuu t‰nne. Printtaus menee
// TIMER0_A0_VECTOR keskeytykseen joka on UART.c:ss‰.
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)
{
  switch( TAIV )
  {
  case  2:                       // CCR1
	  // V‰hennet‰‰n arvoa ja vertaillaan onko aika mennyt nollaan
	  globalSecondsCounter--;
	  if(globalSecondsCounter == 0)
	  {
		  // aika loppui, her‰tys!
		  __bic_SR_register_on_exit(LPM3_bits);
	  }
	  CCR1 += 12000;
           break;
  case 10:                    // Timer_A3 overflow
           break;
 }
}
