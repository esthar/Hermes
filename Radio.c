/*
 * Radio.c
 *
 * Amherst College Electronics Club - Nov 2013
 *  Author: Matthew Kurek
 */ 

#include "Radio.h"

void RadioSetup(void)
{

	USARTPrint("+++");				// Enters command mode
	USARTPrint("ATBR0");				// Sets RF data rate to 9600
	USARTWriteChar(0x0d);		// Return
	USARTPrint("ATPL01");			// Sends PL command to 01 (10 mW).
	USARTWriteChar(0x0d);		// Return
	USARTPrint("ATWR");				// Writes to non-volatile memory
	USARTWriteChar(0x0d);		// Return
	USARTPrint("ATCN");				// Exits command mode
	USARTWriteChar(0x0d);		// Return
			
}

void RadioSendCmd(const char *data)	// Sends an AT command with its parameters
{			
	
	USARTPrint("+++");
	USARTPrint(data);				// Sends data
	USARTWriteChar(0x0d);		// Return
	USARTPrint("ATCN");
	USARTWriteChar(0x0d);

}

void RadioSetPower(const uint8_t p)		// Change power (0-4 where 4 corresponds to 1W)
{		
		
	USARTPrint("+++");				// Enter command mode
	USARTPrint("ATPL");				// Sends PL command
	USARTWriteChar(p+48);		//Sends ASCII number parameter
	USARTWriteChar(0x0d);		// Return
	USARTPrint("ATWR");				// Write to non-volatile memory
	USARTWriteChar(0x0d);		// Return
	USARTPrint("ATCN");				// Exit command mode
	USARTWriteChar(0x0d);		// Return
}

void RadioTransmit(const char *data)	// Generic transmission command.
{ 
	USARTPrint(data);				// Sends data via USART.
}

