/*
 * 	SFR02.h
 * 
 *	### See SFR02.h.c for more information!!!
 *
 * 			Created on:		10.6.2014
 *			Modified on:	10.6.2014
 * 			Contact:		mika_va@hotmail.com / www.hutasu.net
 *			Version:		0.1 (Created).
 */

#ifndef SFR02_H_
#define SFR02_H_

// Required library for communication
#include "../USI/USI-I2C.h"

// ===================== USER SELECTABLE OPTIONS ==================================
// Basic information:
// When changing these values please note, that there should usually
// be only one setting enabled/selected (uncommented).
// in each group of settings.

// ========================== ADDRESS SELECTION	===================================
// Select which address the device uses. If you don't know the address, leave the address
// commented and use function ReadDevices

#define SFR02_ADDRESS		0xE0			// Default is E0




// ########## DO NOT EDIT BELOW THIS LINE #####################

typedef unsigned char   BOOL;
typedef signed char		INT8;
typedef signed int		INT16;
typedef unsigned char   UINT8;
typedef unsigned int    UINT16;

typedef enum
{
	INCHES = 0x50,
	CENTIMETERS,
	MICROSECONDS
} e_RangingResult;

typedef struct
{
	UINT8	SW_revision;
	// UINT8	unused;
	UINT8 	RangeHighByte;
	UINT8 	RangeLowByte;
	UINT8 	AutotuneHighByte;
	UINT8 	AutotuneLowByte;
} t_SFR02;

#define SFR02_MAX_ADDRESS	0xFE
#define SFR02_MAX_DEVICES	16

// ########## EDIT RESTRICTION ENDS HERE #####################

// ===================== FUNCTION PROTOTYPES ========================================
BOOL SFR02_CheckDevice(UINT8 address);
UINT8 SFR02_CollectDevices();
void SFR02_CollectDeviceData(UINT8 address);
BOOL SFR02_StartRanging(e_RangingResult format, UINT8 address);
UINT16 SFR02_GetRange(e_RangingResult format, UINT8 address);

#endif /* SFR02_H_ */
