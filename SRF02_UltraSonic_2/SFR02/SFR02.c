/* 
 * 	SFR02.c
 * 
 *	Description: 	This file contains functions to operate with the SFR02 Ultrasonic Rangefinder module.
 *					Communication is based on I2C and USI-library is needed for this.
 *
 * 	Note:			Change user settings (notified as "User: xxxx" in SFR02.h) whatever is needed and use
 * 					the functions provided.
 * 					It is recommended to set the correct device in the project settings,
 * 					because underlying USI-I2C includes only <msp430.h> -library rather than spesific device.
 * 
 *
 * 			Created on:		10.6.2014
 *			Modified on:	13.6.2014
 * 			Contact:		mika_va@hotmail.com / www.hutasu.net
 *			Version:		0.1 (Created).
 *							0.2	Bug fix:
 *								- SFR02_GetRange() didn't wait long enough after triggering the range detection.
 *								  this was fixed by adding 70ms delay function in between triggering and reading.
 *								- Added some clarifying comments in function descriptions.
 */

#include "SFR02.h"
#include "../Launchpad/UART.h"

// Create at least one SFR02
t_SFR02		SFR02_firstDevice;

// Function used to check if there's a device at known address.
//
// Parameters:	address = address of the slave device
// Returns:		TRUE (1) if device at that address was found, otherwise FALSE (0).
//
// Example: 	online = SFR02_CheckDevice(0xE0);
//
BOOL SFR02_CheckDevice(UINT8 address)
{
	return CheckForSlaveExists(address);
}

// Function which collects information of how many SFR02 devices are on the bus. This can be used to
// count the device on a bus and also to collect information about those devices. Information such as
// Software revision, range high/low byte and autotune high/low byte.Results are stored
// in SFR02_firstDevice. If more than one device is connected, then the last one which was found is stored
// in SFR02-structure.
//
// Parameters:	none.
// Returns:		Number of devices found from the bus.
//
// Example: 	deviceCount = SFR02_CheckDevice();
//
UINT8 SFR02_CollectDevices()
{
	UINT8* strAddr = (UINT8 *) &(SFR02_firstDevice.SW_revision);
	UINT8 reg = 0;
	UINT8 maxCount = SFR02_MAX_DEVICES;
	UINT8 devCount = 0;
	UINT8 address = SFR02_MAX_ADDRESS;

	while( maxCount-- > 0)
	{
		if( CheckForSlaveExists(address) == 1 )
		{
			devCount++;
			// Collect information from the device
			for(reg = 0; reg < 6; reg++)
			{
				if(reg == 1)
				{
					// skip unused byte
					continue;
				}
				I2CMasterReadRegister(address,reg,strAddr,1);
				strAddr++;
			}
		}
		address -= 2;
	}
	return devCount;
}

// Function which collects information from SFR02, such as
// Software revision, range high/low byte and autotune high/low byte. Results are stored
// in SFR02_firstDevice.
//
// Parameters:	Device address
// Returns:		none.
//
// Example: 	SFR02_CollectDeviceData(0xE2);
//
void SFR02_CollectDeviceData(UINT8 address)
{
	UINT8* strAddr = (UINT8 *) &(SFR02_firstDevice.SW_revision);
	UINT8 reg = 0;
	// Collect information from the device
	for(reg = 0; reg < 6; reg++)
	{
		if(reg == 1)
		{
			// skip unused byte
			continue;
		}
		I2CMasterReadRegister(address,reg,strAddr,1);
		strAddr++;
	}
}

// Set SFR02 to ranging mode. After completing this command, SFR02 starts to measure
// distance. Result can be read after 66 mS have passed.
//
// Parameters:	format = define what format is used, following are allowed value:
//				INCHES,
//				CENTIMETERS,
//				MICROSECONDS
//
// Returns:		TRUE (1) if device responded to it's address (ranging started). Otherwise false.
//
// Example: 	started1 = SFR02_StartRanging(INCHES, 0xE0);
//				started2 = SFR02_StartRanging(CENTIMETERS, 0xE2);
//				started3 = SFR02_StartRanging(MICROSECONDS, 0xE4);
//
BOOL SFR02_StartRanging(e_RangingResult format, UINT8 address)
{
	UINT8 data[] = {0x00,format};
	return (BOOL) I2CMasterWrite(address,data,2);
}

// Read single range value from the sensor.
// Parameters:	format = define what format is used, following are allowed value:
//				INCHES,
//				CENTIMETERS,
//				MICROSECONDS
//
// Returns:		Result in selected format
//
// Example: 	range_in = SFR02_GetRange(INCHES, 0xE0);
//				range_cm = SFR02_GetRange(CENTIMETERS, 0xE2);
//				range_us = SFR02_GetRange(MICROSECONDS, 0xE4);
//
UINT16 SFR02_GetRange(e_RangingResult format, UINT8 address)
{
	UINT8 data[] = {0x00,format};
	I2CMasterWrite(address,data,2);
	TimerA_msDelay(70);
	SFR02_CollectDeviceData(address);
	return (SFR02_firstDevice.RangeHighByte << 8 | SFR02_firstDevice.RangeLowByte);
}

