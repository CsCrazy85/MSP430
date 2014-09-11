/*
 * Driver for MMA8452 10-bit/8-bit Digital Accelerometer using I2C.
 *
 * MMA8452Q (No HPF Data)
 * 	10-bit data:
 * 	2g (256 counts/g = 3.9 mg/LSB) 4g (128 counts/g = 7.8 mg/LSB) 8g (64 counts/g= 15.6 mg/LSB)
 *
 *	8-bit data:
 *	2g (64 counts/g = 15.6 mg/LSB) 4g (32 counts/g = 31.25 mg/LSB) 8g (16 counts/g = 62.5 mg/LSB)
 *
 *
 * This code is written on Windows 7 using Code Composer Studio v5. When
 * you change the code, it is recommended to ReBuild instead of Building or
 * do Clean -> Build.
 *
 *
 * HW Connection:
 *
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
 *
 *
 *
 *  Created on: 	18.1.2014
 *  Author: 		Mika Väyrynen
 *  Version:		1.0	/ 20.3.2014			First version
 *	Contact:		mika_va@hotmail.com / www.hutasu.net
 */

#include "accelerometer.h"
#include "../Launchpad/UART.h"

#define DEBUG

typedef enum
{
	Mode8bit,
	Mode10bit
} e_bitMode;

typedef struct
{
	INT16	raw_X;	// X-axis
	INT16	raw_Y; // Y-axis
	INT16	raw_Z;	// Z-axis
	UINT8 	status;	// status register
	e_bitMode	bitMode;
} s_accelerometer;

typedef union
{
	INT16	dataWord;
	struct
	{
		UINT8 highByte;
		UINT8 lowByte;
	} byte;
}u_acceleroData;

u_acceleroData	axisValue;

s_accelerometer	accelerometer;
s_accelerometer *acceldata;

// Set accelerometer to either 8-bit or 10-bit datamode and select range 2/4/8g.
void SetDataMode(BOOL bitMode10, UINT8 range)
{
	UINT8 reg,dynRange = 0;
	SetAcceleroStandby();

	switch(range)
	{
	case 2:
		dynRange = 0;
		break;
	case 4:
		dynRange = FS_RANGE_FS0_BIT;
		break;
	case 8:
		dynRange = FS_RANGE_FS0_BIT + FS_RANGE_FS1_BIT;
	break;
	default:
		#ifdef DEBUG
			//TimerA_UART_print("\n\rInvalid range!");
		#endif
		break;
	}
	RegisterWrite(XYZ_DATA_CFG,dynRange + HPF_OUT_BITS);

	//reg = RegisterRead(CTRL_REG1,0,1);

	I2CMasterReadRegister(ACCELEROMETER_ADDRESS,CTRL_REG1,&reg,1);

	if(bitMode10)
	{
		RegisterWrite(CTRL_REG1, (reg & ~F_READ));
		//acceldata -> bitMode = Mode10bit;
	}
	else
	{
		/*
		 * The most significant 8-bits of each axis are stored in OUT_X (Y,Z)_MSB, so applications needing only
		 * 8-bit results can use these 3 registers and ignore OUT_X,Y, Z_LSB. To do this, the
		 * F_READ bit in CTRL_REG1 must be set. When the F_READ bit is cleared,
		 * the fast read mode is disabled.
		 * */
		RegisterWrite(CTRL_REG1, (reg  | F_READ)); // Set F_READ_BIT
		//acceldata -> bitMode = Mode8bit;
	}

	SetAcceleroActive();
}
// Reads accelerometer ID and verifies it, if everything is ok, TRUE is returned, otherwise FALSE
INT8 GetAccelID(UINT8 compare)
{
	if(RegisterRead(WHO_AM_I,0,1) == compare)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}
void PrintAccelerometerValues()
{
	INT16 x,y,z;
	x = acceldata->raw_X;
	y = acceldata->raw_Y;
	z = acceldata->raw_Z;
	printf("\n\rX:%i Y:%i Z:%i",x,y,z);
}

// Initialize accelerometer for use in the main program
void InitializeAccelerometer()
{

	// Reset device and it's internal registers
	RegisterWrite(CTRL_REG2, RESET_SW_BIT);

	// Set accelerometer to standby in order to do changes into the configuration registers
	SetAcceleroStandby();

	// Set Output Data Rate, ASLP-bits, LNOISE and F_READ -bit.
	RegisterWrite(CTRL_REG1, ODR_BITS + AUTO_SLEEP_RATE_BITS + LNOISE_BITS + F_READ_BITS);


	// Set XYZ_DATA_CFG bits
	RegisterWrite(XYZ_DATA_CFG, FS_RANGE + HPF_OUT_BITS);

	// set transient function to react with acceleration more than 0,126 g
	// 0,126g / 0,063g = 2
	// Note!: this number cannot be greater than 127!!!
	RegisterWrite(TRANSIENT_THS,2);

	// For more information about the transient configuration, check out: http://cache.freescale.com/files/sensors/doc/app_note/AN4071.pdf
	// ELE = Latch enable: interrupt source should be read from TRANSIENT_SRC register until it's set again
	// ZTEFE, XTEFE, YTEFE = enable transient detection on all axis
	RegisterWrite(TRANSIENT_CFG, ELE + ZTEFE + XTEFE + YTEFE);

	// Because is set to be ODR = 100 Hz
	// then timestep for one count is 10ms. Refer to AN4071 for details.
	// Transient must be active for more than this time before interrupt occurrs
	RegisterWrite(TRANSIENT_COUNT,10);	// 100 ms

	// Set Sleep-Mode power scheme, Active-Mode power scheme, and auto-sleep bits
	RegisterWrite(CTRL_REG2, SLEEP_POWER_MODE_BITS + ACTIVE_POWER_MODE_BITS + AUTO_SLEEP_BITS);

	// Interrupt polarity active high = interrupt pin sets to high when interrupt occurred
	RegisterWrite(CTRL_REG3, IPOL);

	// Set transient interrupt enable (default routing to INT2-pin)
	RegisterWrite(CTRL_REG4, INT_EN_TRANS);

	// Route wanted interrupts to pin INT1.
	RegisterWrite(CTRL_REG5,INT_CFG_DRDY);	// Route data-ready interrupt to INT1-pin

	// Activate accelerometer
	SetAcceleroActive();

	// Read interrupt register to reset interrupts
	RegisterRead(INT_SOURCE,0,1);
}

void ResetAcceleroSW()
{
	RegisterWrite(CTRL_REG2, RESET_SW_BIT);
	__delay_cycles(10000);
}
// Set accelerometer to standby mode (lower power)
void SetAcceleroStandby()
{
	static UINT16 reg = 0;
	reg = RegisterRead(CTRL_REG1,0,1);
	RegisterWrite(CTRL_REG1, (reg & ~ACTIVE_MODE));
}
// Set accelerometer to active mode
void SetAcceleroActive()
{
	UINT16 reg = RegisterRead(CTRL_REG1,0,1);
	RegisterWrite(CTRL_REG1,reg | ACTIVE_MODE);
}

INT16 ReadAxis(UINT8 AXIS)
{
	t_RegisterAddressMap reg;
	INT16 acceleroValue = 0;
	UINT16 temp = 0;
	switch(AXIS)
	{
	case X_AXIS:
		reg = OUT_X_MSB;
		break;
	case Y_AXIS:
		reg = OUT_Y_MSB;
			break;
	case Z_AXIS:
		reg = OUT_Z_MSB;
			break;
	}

	// swap LSB and MSB
	RegisterRead(reg,(UINT8*)&acceleroValue,2);
	temp = acceleroValue & 0x00FF; // take MSB
	acceleroValue = (acceleroValue >> 8) & 0x00FF;
	acceleroValue |= temp << 8;		// place LSB
	return acceleroValue;
}

// Read all axis values at once
void ReadAxisAll( INT16 *xData, INT16 *yData, INT16 *zData )
{
	UINT8 acceleroValues[6];
	INT16 temp;

	RegisterRead(OUT_X_MSB,(UINT8*)&acceleroValues[0],6);

	*(((UINT8*) &temp)+1) = acceleroValues[0];
	*((UINT8*) & temp) = acceleroValues[1];
	*xData = temp;

	*(((UINT8*) &temp)+1) = acceleroValues[2];
	*((UINT8*) & temp) = acceleroValues[3];
	*yData = temp;

	*(((UINT8*) &temp)+1) = acceleroValues[4];
	*((UINT8*) & temp) = acceleroValues[5];
	*zData = temp;
}

void ReadAxisAll2(INT16 *xyzData)
{
	INT16 i = 0;
	INT16 temp = 0;
	UINT8 acceleroValues[6];

	RegisterRead(OUT_X_MSB,(UINT8*)&acceleroValues[0],6);

	for(i = 0; i < 6; i+=2)
	{
		*(((UINT8*) &temp)+1) = acceleroValues[i];
		*((UINT8*) & temp) = acceleroValues[i + 1];
		*xyzData = temp;
		*xyzData++;
	}
}

// Write register value
void RegisterWrite(t_RegisterAddressMap reg, UINT8 value)
{
	unsigned char registerAndData[2];
	registerAndData[0] = (unsigned char) reg;
	registerAndData[1] = value;

	// Initialize transmitter
	//Setup_USI_Master_TX();
	//TI_USCI_I2C_transmitinit(ACCELEROMETER_ADDRESS,12);

	// wait for initialization to complete
	//while(TI_USCI_I2C_notready());

	// transmit address + W, register and data
	I2CMasterWrite(ACCELEROMETER_ADDRESS, registerAndData,2);

	//TI_USCI_I2C_transmit(2,registerAndData);
	// wait for transfer to complete
	// while(TI_USCI_I2C_notready());

}
// Function to read 8-bit register values from MMA8452. Returns last byte which
// was read, so this function can be used to return only one byte. Multibyte read
// needs a buffer address to be delivered.
UINT8 RegisterRead(t_RegisterAddressMap reg, UINT8 *dataBuffer, UINT8 count)
{
	UINT8 val=0;

	// Initialize transmitter
	//TI_USCI_I2C_transmitinit(ACCELEROMETER_ADDRESS,12);

	//Setup_USI_Master_TX();

	if(dataBuffer == 0)
	{
		I2CMasterReadRegister(ACCELEROMETER_ADDRESS,reg,&val,1);
		return val;
	}
	else
	{
		I2CMasterReadRegister(ACCELEROMETER_ADDRESS,reg,dataBuffer,count);
		return dataBuffer[0];
	}

}
