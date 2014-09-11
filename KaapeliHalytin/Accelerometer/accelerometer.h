/*
 * This code is written on Windows 7 using Code Composer Studio v5. When
 * you change the code, it is recommended to ReBuild instead of Building or
 * do Clean -> Build.
 *
 * Please refer to: http://cache.freescale.com/files/sensors/doc/app_note/AN4071.pdf and other documents for details
 *
 *  Created on: 	18.1.2014
 *  Author: 		Mika Väyrynen
 *  Version:		1.0
 *	Contact:		mika_va@hotmail.com / www.hutasu.net
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "../USI/USI.h"
#include "../printf/printf.h"

typedef unsigned char   BOOL;
typedef signed char		INT8;
typedef signed int		INT16;
typedef unsigned char   UINT8;
typedef unsigned int    UINT16;

#define FALSE 0
#define TRUE 1
#define HIGH 1
#define LOW	0

// TODO: Muuta // -kommentit parempiin kommenttilohkoihin että käyttäjä voi lukea selvemmin säädettävistä asioista.

// Select which accelerometer to use
//#define MMA8451Q
#define MMA8452Q
// #define MMA8453Q

// Select whether SA0-pin is in high or low state
#define SA0_STATE		LOW			// Use LOW or HIGH only!

// Accelerometer ID (don't edit)
#define MMA8453Q_ID			0x3A	// don't change
#define MMA8452Q_ID			0x2A	// don't change
#define MMA8451Q_ID			0x1A

// Define output data rate (ODR). Select only one of these!
//#define ODR_800HZ		// 800 Hz / 1.25 ms
//#define ODR_400HZ		// 400 Hz / 2.5 ms
//#define ODR_200HZ		// 200 Hz / 5 ms
#define ODR_100HZ		// 100 Hz / 10 ms
//#define ODR_50HZ		// 50 Hz / 20 ms
//#define ODR_12_5HZ	// 12.5 Hz / 80 ms
//#define ODR_6_25HZ	// 6.25 Hz / 160 ms
//#define ODR_1_563HZ	// 1.563 Hz / 640 ms

// Define dynamic range 2g/4g/8g
#define ACCELMODE_2G
//#define ACCELMODE_4G
//#define ACCEL_MODE_8G

// Define if low noise mode is used. Comment it out if it's not used.
// Note: In Low Noise mode, the maximum signal that can be measured is 4g.
#define LOW_NOISE_ENABLED				// Default: in use.

// Define if transient detection interrupt pin is enabled
#define TRANSIENT_INTERRUPT_PIN_ENABLED

// Define if to use Fast Read Mode (8-bit).
// When defined, the auto-increment counter will skip over the LSB data bytes.
// #define FAST_READ_ENABLED			// Default: not used.

// Define High-Pass Filtered Data or Low-Pass Filtered Data
//#define HPF_OUT_ENABLE				// Default: not used.

// TODO: implement feature below
// Define High-Pass Filter CUT-off frequency. Refer to a table 8 in document "AN4076 Data Manipulation and Basic settings of the MMA845x"
// #define HPF_FILTER_CUTOFF_FREQ		2

// Define Sleep-mode power. The difference between these are the amount of averaging of the sampled data,
// which is done internal to the device. There is a trade-off between the oversampling and the current consumption at each ODR value.
// Note:	Select only one option!
//#define SLEEP_POWER_MODE_NORMAL				// Default: Normal mode
#define SLEEP_POWER_MODE_LOW_POWER_LOW_NOISE
//#define SLEEP_POWER_MODE_HIGH_RESOLUTION
//#define SLEEP_POWER_MODE_LOW_POWER

// Define Active-mode power mode.
// Note: Select only one option!
//#define ACTIVE_POWER_MODE_NORMAL
#define ACTIVE_POWER_MODE_LOW_POWER_LOW_NOISE
//#define ACTIVE_POWER_MODE_HIGH_RESOLUTION
//#define ACTIVE_POWER_MODE_LOW_POWER

// Define auto-sleep data rate. If none of the rates are activated, default value of 0 is used (50 Hz).
//#define AUTO_SLEEP_RATE_50Hz					// Default 50 Hz
#define AUTO_SLEEP_RATE_12Hz	// 12,5 Hz
//#define AUTO_SLEEP_RATE_6Hz		// 6,25 Hz
//#define AUTO_SLEEP_RATE_2Hz		// 1,56 Hz

// Define if auto-sleep is enabled at startup
// #define AUTO_SLEEP_ENABLED

// ########## DO NOT EDIT BELOW THIS LINE #####################
// Write/Read registers
typedef enum
{
	STATUS = 0x00,
	OUT_X_MSB,
	OUT_X_LSB,
	OUT_Y_MSB,
	OUT_Y_LSB,
	OUT_Z_MSB,
	OUT_Z_LSB,
	// 0x07 reserved
	// 0x08 reserved
	SYSMOD = 0x0B,
	INT_SOURCE,
	WHO_AM_I,
	XYZ_DATA_CFG,
	HP_FILTER_CUTOFF,
	PL_STATUS, // 0x10
	PL_CFG,
	PL_COUNT,
	PL_BF_ZCOMP,
	P_L_THS_REG,
	FF_MT_CFG,	// 0x15
	FF_MT_SRC,
	FF_MT_THS,
	FF_MT_COUNT,	// 0x18
	// 0x19 reserved
	// 0x1A reserved
	// 0x1B reserved
	// 0x1C reserved
	TRANSIENT_CFG = 0x1D,
	TRANSIENT_SRC,
	TRANSIENT_THS,
	TRANSIENT_COUNT, // 0x20
	PULSE_CFG,
	PULSE_SRC,
	PULSE_THSX,
	PULSE_THSY,
	PULSE_THSZ,
	PULSE_TMLT,
	PULSE_LTCY,
	PULSE_WIND,
	ASLP_COUNT,
	CTRL_REG1,	// 0x2A
	CTRL_REG2,
	CTRL_REG3,
	CTRL_REG4,
	CTRL_REG5,
	OFF_X,
	OFF_Y,
	OFF_Z
	// 0x40 - 0x7F reserved (do not modify)
}t_RegisterAddressMap;


#define X_AXIS	0
#define Y_AXIS	1
#define Z_AXIS	2

// CTRL_REG1 flags
#define ACTIVE_MODE		0x01
#define F_READ			0x02
#define LNOISE			0x04
#define DR_0_BIT		0x08
#define DR_1_BIT		0x10
#define DR_2_BIT		0x20
#define ASLP_RATE_0_BIT	0x40
#define ASLP_RATE_1_BIT	0x80

// CTRL_REG2 flags:
#define MODS0_BIT				0x01
#define MODS1_BIT				0x02
#define SLEEP_ENABLE_BIT		0x04
#define SMODS0_BIT				0x08
#define SMODS1_BIT				0x10
// BIT5 unused
#define RESET_SW_BIT			0x40
#define SELFTEST_ENABLE_BIT		0x80

// CTRL_REG3 flags:						// 0 = function is bypassed in SLEEP mode.
// Bit 7 not used
#define WAKE_TRANS				0x40	// 1: Transient function interrupt can wake up system
#define WAKE_LNDPRT				0x20	// 1: Orientation function interrupt can wake up system
#define WAKE_PULSE				0x10	// 1: Pulse function interrupt can wake up system
#define WAKE_FF_MT				0x08	// 1: Freefall/Motion function interrupt can wake up
// bit 2 not used
#define IPOL					0x02	// Interrupt polarity ACTIVE high, or ACTIVE low (default).
#define PP_OD					0x01	// Push-Pull/Open Drain selection on interrupt pad. 0: Push-Pull; 1: Open Drain


// CTRL_REG4 flags:
#define INT_EN_ASLP_BIT			0x80 // 0: Auto-SLEEP/WAKE interrupt disabled; 1: Auto-SLEEP/WAKE interrupt enabled.
// bit 6 not used
#define INT_EN_TRANS			0x20 // 0: Transient interrupt disabled; 1: Transient interrupt enabled.
#define INT_EN_LNDPRT			0x01 // 0: Orientation (Landscape/Portrait) interrupt disabled.
									 // 1: Orientation (Landscape/Portrait) interrupt enabled.
#define INT_EN_PULSE			0x08 // 0: Pulse Detection interrupt disabled; 1: Pulse Detection interrupt enabled
#define INT_EN_FF_MT			0x04 // 0: Freefall/Motion interrupt disabled; 1: Freefall/Motion interrupt enabled
// bit 1 not used
#define INT_EN_DRDY				0x01 // 0: Data Ready interrupt disabled; 1: Data Ready interrupt enabled

// CTRL_REG5 flags:				--- INT1/INT2 Configuration. ---
#define INT_CFG_ASLP			0x80 // 0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
#ifdef MMA8451Q
#define INT_CFG_FIFO			0x40 // --- used only in MMA8451 ---
#endif
#define INT_CFG_TRANS			0x20 // 0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
#define INT_CFG_LNDPRT			0x10 // 0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
#define INT_CFG_PULSE			0x08 // 0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
#define INT_CFG_FF_MT			0x04 // 0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin
// bit 1 not used
#define INT_CFG_DRDY			0x01 // 0: Interrupt is routed to INT2 pin; 1: Interrupt is routed to INT1 pin

// INT_SOURCE flags:
#define SRC_ASLP				INT_CFG_ASLP
#ifdef MMA8451Q
#define SRC_FIFO				INT_CFG_FIFO	// --- used only in MMA8451 ---
#endif
#define SRC_TRANS				INT_CFG_TRANS
#define SRC_LNDPRT				INT_CFG_LNDPRT
#define SRC_PULSE				INT_CFG_PULSE
#define SRC_FF_MT				INT_CFG_FF_MT
#define SRC_DRDY				INT_CFG_DRDY

// XYZ_DATA_CFG flags:
#define FS_RANGE_FS0_BIT	0x01	// Range configuration flags
#define FS_RANGE_FS1_BIT	0x02

// TRANSIENT_CFG flags:
#define ELE					0x10	// Note that if ELE = 1, the latch is enabled and the status will remain until it is read
#define ZTEFE				0x08
#define YTEFE				0x04
#define XTEFE				0x02
#define HPF_BYP				0x01	// if the HPF_BYP bit is set then the Transient function will behave like the Motion function

/*
 * Example: X or Y > 6g with Latch enabled for time period of 60 ms at 50 Hz ODR
 * Code Example:
 * IIC_RegWrite(0x1D, 0x16); //This enables the X-axis and the Y-axis only with the Latch
 *
 * */

// TRANSIENT_THS flags:
#define DBCNTM 			0x80		// Debounce counter mode selection. Default value: 0. 0: increments or decrements debounce; 1: increments or clears counter.
#define THS6 			0x40
#define THS5 			0x20
#define THS4 			0x10
#define THS3 			0x08
#define THS2 			0x04
#define THS1 			0x02
#define THS0			0x01

/* AN4071:
 *
 * The TRANSIENT_THS register shown in Table 4 sets the threshold limit for the
 * detection of the transient acceleration. The value in the TRANSIENT_THS register
 * corresponds to a g value which is compared against the values of the high-pass filtered
 * data. If the high-pass filtered acceleration exceeds the threshold limit, an event
 * flag is raised and an interrupt is generated assuming the interrupts have been configured and enabled.
 *
 * The threshold THS[6:0] is a 7-bit unsigned number, 0.063g/LSB. The maximum threshold is 8g.
 * Even if the part is set to full scale at 2g or 4g this function will still operate up to 8g.
 * If the Low-Noise bit is set in Register 0x2A, the maximum threshold to be reached is 4g.
 *
 * The threshold THS[6:0] is a 7-bit unsigned number. The minimum threshold resolution is 0.063g
 * and the threshold register has a range of 0 to 127counts.
 * The maximum threshold is 8g. The threshold range is not
 * dependent on the set Full Scale in Register 0x0E.
 *
 * Example: X or Y > 6g with Latch enabled for time period of 60 ms at 50 Hz ODR
 * Since the minimum time step is 0.063g per count, the calculation is as follows:
 * 6g/ 0.063g = 95.2 counts, round to 96 counts
 *
 * Code Example: IIC_RegWrite(0x1F, 0x60); //Threshold set to 96 counts (6.048g)
 * */

// TRANSIENT_COUNT flags:
#define D7		0x80
#define D6 		0x40
#define D5 		0x20
#define D4 		0x10
#define D3 		0x08
#define D2 		0x04
#define D1 		0x02
#define D0 		0x01

/*
 * The TRANSIENT_COUNT shown inTable 5 sets the number of debounce counts (timing) required
 * for the condition to persist. Note that the timer for the transient as well as
 * for all the other embedded functions is dependent on the oversampling mode of
 * the system.
 *
 * Example: X or Y > 6g with Latch enabled for time period of 60 ms at 50 Hz ODR
 * 60 ms total time/ 20 ms (at 50 Hz) = 3 counts
 * Code Example: IIC_RegWrite(0x20, 0x03);
 * */

// Set up address
#if SA0_STATE==LOW
#define ACCELEROMETER_ADDRESS	0x1C << 1
#elif SA0_STATE==HIGH
#define ACCELEROMETER_ADDRESS	0x1D
#else
#error "SA0_STATE: Use LOW or HIGH values only!"
#endif

// Set up auto-sleep-rate bits in CTRL_REG1 register
#ifdef AUTO_SLEEP_RATE_50Hz
#define AUTO_SLEEP_RATE_BITS		0x00
#else
#ifdef AUTO_SLEEP_RATE_12Hz
#define AUTO_SLEEP_RATE_BITS		ASLP_RATE_0_BIT
#else
#ifdef AUTO_SLEEP_RATE_6Hz
#define AUTO_SLEEP_RATE_BITS		ASLP_RATE_1_BIT
#else
#ifdef AUTO_SLEEP_RATE_2Hz
#define AUTO_SLEEP_RATE_BITS		ASLP_RATE_0_BIT + ASLP_RATE_1_BIT
#endif
#endif
#endif
#endif

#ifndef AUTO_SLEEP_RATE_BITS
#warning "AUTO_SLEEP_RATE_BITS not defined by user. Default value is used (50 Hz)."
#define AUTO_SLEEP_RATE_BITS	0x00
#endif

// Set up Output Data Rate bits in CTRL_REG1 register
#ifdef ODR_800HZ		// 800 Hz / 1.25 ms
	#define ODR_BITS					0x00
#endif
#ifdef ODR_400HZ		// 400 Hz / 2.5 ms
	#define ODR_BITS					0x08
#endif
#ifdef ODR_200HZ		// 200 Hz / 5 ms
	#define ODR_BITS					0x10
#endif
#ifdef ODR_100HZ		// 100 Hz / 10 ms
	#define ODR_BITS					0x18
#endif
#ifdef ODR_50HZ			// 50 Hz / 20 ms
	#define ODR_BITS					0x20
#endif
#ifdef ODR_12_5HZ		// 12.5 Hz / 80 ms
	#define ODR_BITS					0x28
#endif
#ifdef ODR_6_25HZ		// 6.25 Hz / 160 ms
	#define ODR_BITS					0x30
#endif
#ifdef ODR_1_563HZ		// 1.563 Hz / 640 ms
	#define ODR_BITS					0x38
#endif

#ifndef ODR_BITS
	#warning "ODR_BITS not defined by user. Default value for 12,5 Hz is set (0x28)."
	#define ODR_BITS 0x28	// default the value to 12,5 Hz if not set up
#endif


// Set up power mode bits in CTRL_REG2 register
#ifdef SLEEP_POWER_MODE_NORMAL
	// Normal power
#define SLEEP_POWER_MODE_BITS			0x00
#else
#ifdef SLEEP_POWER_MODE_LOW_POWER_LOW_NOISE
#define SLEEP_POWER_MODE_BITS			SMODS0_BIT
#else
#ifdef SLEEP_POWER_MODE_HIGH_RESOLUTION
#define SLEEP_POWER_MODE_BITS			SMODS1_BIT
#else
#ifdef SLEEP_POWER_MODE_LOW_POWER
#define SLEEP_POWER_MODE_BITS			SMODS0_BIT + SMODS1_BIT
#endif
#endif
#endif
#endif
// Check
#ifndef SLEEP_POWER_MODE_BITS
#error "SLEEP_POWER_MODE_BITS not defined!!!"
#endif


#ifdef ACTIVE_POWER_MODE_NORMAL
	// Normal power
#define ACTIVE_POWER_MODE_BITS			0x00
#else
#ifdef ACTIVE_POWER_MODE_LOW_POWER_LOW_NOISE
#define ACTIVE_POWER_MODE_BITS			MODS0_BIT
#else
#ifdef ACTIVE_POWER_MODE_HIGH_RESOLUTION
#define ACTIVE_POWER_MODE_BITS			MODS1_BIT
#else
#ifdef ACTIVE_POWER_MODE_LOW_POWER
#define ACTIVE_POWER_MODE_BITS			MODS0_BIT + MODS1_BIT
#endif
#endif
#endif
#endif
// Check
#ifndef ACTIVE_POWER_MODE_BITS
#error "ACTIVE_POWER_MODE_BITS not defined!!!"
#endif

// Set up Auto-Sleep Enable-bit in CTRL_REG2 register
#ifdef AUTO_SLEEP_ENABLED
#define AUTO_SLEEP_BITS			SLEEP_ENABLE_BIT
#else
#define AUTO_SLEEP_BITS			0x00
#endif


// Set up HPF-bit in XYZ_DATA_CFG register
#ifdef HPF_OUT_ENABLE	// High pass filter enable
	#define HPF_OUT_BITS					0x10
#else
	#define HPF_OUT_BITS					0x00
#endif

// Set up acceleration range bits in XYZ_DATA_CFG
#ifdef ACCELMODE_2G
	#define FS_RANGE			0x00
#endif

#ifdef ACCELMODE_4G
	#define FS_RANGE			0x01
#endif

#ifdef ACCELMODE_8G
	#define FS_RANGE			0x02
#ifdef LOW_NOISE_ENABLED
#error "LOW_NOISE_ENABLED: Cannot use LNOISE-bit in 8G-mode!"
#endif
#endif

// Set up LNOISE-bit in CTRL_REG1 -register
#ifdef LOW_NOISE_ENABLED
#define LNOISE_BITS				LNOISE
#else
#define LNOISE_BITS				0x00
#endif

// Set up F_READ-bit in CTRL_REG1 -register
#ifdef FAST_READ_ENABLED
#define F_READ_BITS				F_READ
#else
#define F_READ_BITS				0x00
#endif

// set up CTRL_REG4 bits
#ifdef TRANSIENT_INTERRUPT_PIN_ENABLED
#define INTERRUPT_BITS			INT_EN_TRANS
#else
#define INTERRUPT_BITS			0x00
#endif

// ########## EDIT RESTRICTION ENDS HERE #####################

void GetAccelerationXYZ();
void SetDataMode(BOOL bitMode, UINT8 range);
INT8 GetAccelID(UINT8 compare);
void PrintAccelerometerValues();
void UpdateAccelerometer();
void InitializeAccelerometer();
void ResetAcceleroSW();
void SetAcceleroStandby();
void SetAcceleroActive();
INT16 ReadAxis(UINT8 AXIS);
void ReadAxisAll( INT16 *xData,INT16 *yData,INT16 *zData );
void ReadAxisAll2(INT16 *xyzData);
void RegisterWrite(t_RegisterAddressMap reg, UINT8 value);
UINT8 RegisterRead(t_RegisterAddressMap reg, UINT8 *dataBuffer, UINT8 count);

#endif /* ACCELEROMETER_H_ */
