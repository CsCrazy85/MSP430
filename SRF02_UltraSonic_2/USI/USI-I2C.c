/*
 * USI-I2C.c
 *
 *	### For description see USI-I2C.h and USI.h!!!
 *
 *  	Created on: 	14.10.2013
 *  	Modified:		20.10.2012
 *  	Contact:		mika_va@hotmail.com / www.hutasu.net
 *      Author: 		Mika Väyrynen
 *      Version:		0.1 Created. Code is based on Texas instruments example. But this is better/cleaner code ;).
 */
#include "USI.h"

#if COMMUNICATION == I2C

unsigned char *MST_Data;                       	// Variable for transmitted data
unsigned char *MSR_Data;						// Variable for received data
unsigned char repeated_start = 0;				// repeated start condition, needed in register read operations
char SLV_Addr = 0;
int number_of_bytes = 0;
int Bytecount, Transmit = 0;
I2C_States I2C_State = START;					// State variable
static char slavePresent = 0;
static char onlyCheckForSlave = 0;

//------------------------------------------------------------------------------
// Initialize I2C-pins and set device as MASTER transmitter.
//------------------------------------------------------------------------------
void InitUSI()
{
	InitUSIGpioI2C();
}

//------------------------------------------------------------------------------
// Check if slave device is present. Slave address is needed.
// Function returns 1 if slave with given address is on the bus.
//------------------------------------------------------------------------------
char CheckForSlaveExists(char address)
{
	slavePresent = 0;
	onlyCheckForSlave = 1;
	Transmit = 1;
	SLV_Addr = address;
	Setup_USI_Master();
	I2C_State = START;
	USICTL1 |= USIIFG;                      // Set flag and start communication
	LPM0;                                   // CPU off, await USI interrupt
	return slavePresent;
}

//------------------------------------------------------------------------------
// Setup USI as MASTER for 250 kHz operation
//------------------------------------------------------------------------------
void Setup_USI_Master(void)
{
	__disable_interrupt();
	USICTL0 = USIPE6+USIPE7+USIMST+USISWRST;  // Port & USI mode setup
	USICTL1 = USII2C+USIIE;                   // Enable I2C mode & USI interrupt
	USICKCTL = USIDIV_2 + USISSEL_2+USICKPL;    // USI clk: SCL = SMCLK/4 = 250 kHz @ 1MHz
	USICNT |= USIIFGCC;                       // Disable automatic clear control
	USICTL0 &= ~USISWRST;                     // Enable USI
	USICTL1 &= ~USIIFG;                       // Clear pending flag
	__enable_interrupt();
}

//------------------------------------------------------------------------------
// MASTER I2C transmit function.
// Master transmits a number of bytes (bytecount) into a device from the data-
// buffer (*data).
// Parameters:	address = device address
//				*data	= data to send
//				bytecount = number of bytes to write
// Returns:		Zero (0) if slave is not present, one (1) if slave is present and write was succesfull.
//------------------------------------------------------------------------------
unsigned char I2CMasterWrite(unsigned char address, unsigned char *data, unsigned int bytecount)
{
	slavePresent = 0;
	Bytecount = 0;
	Transmit = 1;
	SLV_Addr = address;
	number_of_bytes = bytecount;
	MST_Data = data;
	Setup_USI_Master();
	USICTL1 |= USIIFG;                      // Set flag and start communication
	LPM0;                                   // CPU off, await USI interrupt
	return slavePresent;
}

//------------------------------------------------------------------------------
// MASTER I2C transmit without address. Actually address is zero.
// Master transmits a number of bytes (bytecount) from the data-
// buffer (*data).
// Parameters:	*data		= data to send
// 				bytecount 	= number of bytes to write
// Returns:		none
//------------------------------------------------------------------------------
void I2CMasterTransmitWithoutAddress(unsigned char *data, unsigned int bytecount)
{
	Bytecount = 0;
	Transmit = 1;
	SLV_Addr = 0;
	number_of_bytes = bytecount;
	MST_Data = data;
	Setup_USI_Master();
	USICTL1 |= USIIFG;                      // Set flag and start communication
	LPM0;                                   // CPU off, await USI interrupt
}
//------------------------------------------------------------------------------
// MASTER I2C receive function.
// Master receives a number of bytes (bytecount) from a device and puts the data
// into buffer (*data).
// Parameters:	address 	= device address
//				*data		= buffer where data is received, any existing data is overwritten
//				bytecount 	= number of bytes to read
// Returns:		none
//------------------------------------------------------------------------------
void I2CMasterReceive(unsigned char address, unsigned char *data, unsigned int bytecount)
{
	__disable_interrupt();
	Bytecount = 0;
	Transmit = 0;
	SLV_Addr = address;
	number_of_bytes = bytecount;
	MSR_Data = data;
	Setup_USI_Master();
	USICTL1 |= USIIFG;                        // Set flag and start communication
	LPM0;                                     // CPU off, await USI interrupt
}

//------------------------------------------------------------------------------
// MASTER I2C read register function.
// Master reads a register value from the slave. User has to provide slave
// address, register where the read operation takes place and a buffer where to
// store the bytes. Finally a number of bytes to be read has to be provided.
// Parameters:	address 	= device address
//				reg			= device register which is supposed to be read
//				*data		= buffer where data is received, any existing data is overwritten
// 				bytes		= number of bytes to read
//
// Example usage (main.c for example):
// unsigned char dBuf[3] = {0};
// I2CMasterReadRegister(0x92, 0x00, dBuf, 3);
//------------------------------------------------------------------------------
unsigned char I2CMasterReadRegister(unsigned char address, unsigned char reg, unsigned char *data, unsigned int bytes)
{
	repeated_start = 1;
	if(I2CMasterWrite(address,&reg,1) == 0)
	{
		// Abort read and return 0
		return 0;
	}

	I2CMasterReceive(address,data,bytes);
	repeated_start = 0;

	return slavePresent;
}

//------------------------------------------------------------------------------
// ############## PRIVATE FUNCTIONS NOT TO BE USED ELSEWHERE ###################
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Initialize I2C-pins only.
//------------------------------------------------------------------------------
static inline void InitUSIGpioI2C()
{
	// activate pull-up resistors for I2C
	P1OUT |= BIT6 + BIT7;
	P1REN |= BIT6 + BIT7;
}

//------------------------------------------------------------------------------
// Helper function to load stop bit and set the state machine to STOP.
//------------------------------------------------------------------------------
static void LoadStopBitAndSetStateSTOP()
{
	USISRL = 0x00;
	USICNT |=  0x01;            // Bit counter=1, SCL high, SDA low
	I2C_State = STOP;             // Go to next state: generate Stop
}

//------------------------------------------------------------------------------
// Helper function to send data.
//------------------------------------------------------------------------------
static void Data_TX (void)
{
	USISRL = *MST_Data++;          // Load data byte
	USICNT |=  0x08;              // Bit counter = 8, start TX
	I2C_State = RECEIVE_DATA_NACK;               // next state: receive data (N)Ack
	Bytecount++;
}

//------------------------------------------------------------------------------
// USI interrupt service routine
// Data Transmit : state START -> RECEIVE_ADDR_NACK -> PROCESS_ADDR_NACK -> RECEIVE_DATA_NACK -> PROCESS_DATA_NACK -> STOP
// Data Receive  : state START -> RECEIVE_ADDR_NACK -> PROCESS_ADDR_NACK -> SEND_DATA_NACK -> PREPARE_STOP -> STOP
//------------------------------------------------------------------------------
#pragma vector = USI_VECTOR
__interrupt void USI_TXRX (void)
{
  switch(__even_in_range(I2C_State,14))
    {
	  case START: // Generate Start Condition & send address to slave
			//P1OUT |= 0x01;                // LED on: sequence start
			Bytecount = 0;
			USISRL = 0x00;                // Generate Start Condition...
			USICTL0 |= USIGE+USIOE;
			USICTL0 &= ~USIGE;
			if (Transmit == 1)
			{
				USISRL = SLV_Addr;              // Address + 0 (rw)
			}
			if (Transmit == 0)
			{
				USISRL = SLV_Addr + 1;          // Address + 1 (rw)
			}
			USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, TX Address
			I2C_State = RECEIVE_ADDR_NACK;                // next state: rcv address (N)Ack
			break;

      case RECEIVE_ADDR_NACK: // Receive Address Ack/Nack bit
              USICTL0 &= ~USIOE;            // SDA = input
              USICNT |= 0x01;               // Bit counter=1, receive (N)Ack bit
              I2C_State = PROCESS_ADDR_NACK;                // Go to next state: check (N)Ack
              break;

      case PROCESS_ADDR_NACK: // Process Address Ack/Nack & handle data TX

		if(Transmit == 1)
		{
			USICTL0 |= USIOE;             // SDA = output

			if (USISRL & 0x01)            // If Nack received...
			{
				slavePresent = 0;
				// Send stop...
				LoadStopBitAndSetStateSTOP();
			}
			else
			{
				slavePresent = 1;
				// Ack received, TX data to slave...
				if(onlyCheckForSlave == 1)
				{
					onlyCheckForSlave = 0;
					LoadStopBitAndSetStateSTOP();
					break;
				}
				else
				{
					Data_TX();
				}
				break;
			}

		}
		if(Transmit == 0) // Master is reading data from the bus
		{
			if (USISRL & 0x01)            // If Nack received
			{
				// Prep Stop Condition
				USICTL0 |= USIOE;
				USISRL = 0x00;
				USICNT |=  0x01;            // Bit counter= 1, SCL high, SDA low
				I2C_State = PREPARE_STOP;              // Go to next state: generate Stop
			}
			else
			{
				// DATA RX
				USICTL0 &= ~USIOE;                  // SDA = input --> redundant
				USICNT |=  0x08;                    // Bit counter = 8, RX data
				I2C_State = SEND_DATA_NACK;                      // Next state: Test data and (N)Ack
			}

		}
              break;

      case SEND_DATA_NACK: // Send Data Ack/Nack bit
			USICTL0 |= USIOE;             // SDA = output

			if (Bytecount <= number_of_bytes - 2)
			{
				// If this is not the last byte
				*MSR_Data = USISRL;							// receive character to buffer
				*MSR_Data++;								// increment buffer pointer
				USISRL = 0x00;              				// Send Ack
				I2C_State = PROCESS_ADDR_NACK;              // Go to next state: data/rcv again
				Bytecount++;
			}
			else //last byte: send NACK
			{
				*MSR_Data = USISRL;							// receive last character to buffer
				USISRL = 0xFF;              // Send NAck
				I2C_State = PREPARE_STOP;              // stop condition
			}
				USICNT |= 0x01;               // Bit counter = 1, send (N)Ack bit
         break;

      case PREPARE_STOP: // Prep Stop Condition
			USICTL0 |= USIOE;             // SDA = output
			LoadStopBitAndSetStateSTOP();
			break;

      case RECEIVE_DATA_NACK: // Receive Data Ack/Nack bit
			USICTL0 &= ~USIOE;            // SDA = input
			USICNT |= 0x01;               // Bit counter = 1, receive (N)Ack bit
			I2C_State = PROCESS_DATA_NACK;               // Go to next state: check (N)Ack
			break;

      case PROCESS_DATA_NACK: // Process Data Ack/Nack & send Stop
			USICTL0 |= USIOE;				// SDA = output
			if (Bytecount == number_of_bytes )
			{
				// If last byte
				if(repeated_start == 1)
				{
					USISRL = 0xFF;              // this will prevent a stop cond
					USICTL0 |= USIOE;           // SDA = output
					I2C_State = STOP;             // Go to next state: generate Stop
					USICNT |=  0x01;            // set count=1 to trigger next state
				}
				else
				{
					// read final byte
					if(MSR_Data != 0)
					{
						// Fixed crash when transmitting bytes without slave address
						*MSR_Data = USISRL;
					}
					LoadStopBitAndSetStateSTOP();
				}
			}        // set count=1 to trigger next state
			else
			{
				Data_TX();                  // TX byte
			}
              break;

      case STOP: // Generate Stop Condition
			USISRL = 0x0FF;               // USISRL = 1 to release SDA
			USICTL0 |= USIGE;             // Transparent latch enabled
			USICTL0 &= ~(USIGE+USIOE);    // Latch/SDA output disabled
			I2C_State = START;                // Reset state machine for next xmt
			LPM0_EXIT;                    // Exit active for next transfer
			break;
    }

  USICTL1 &= ~USIIFG;                       // Clear pending flag
}
#endif
