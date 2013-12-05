/*
 * TWI.c
 *
 * Amherst College Electronics Club - Nov 2013
 *  Author: André Lucas Antunes de Sá
	Part of the code was modified from Atmel's application notes for the TWI
 */ 

#include "TWI.h"

volatile char TWI_Busy;
volatile char TWI_Status;
volatile char TWI_RXBUFFER[164];
volatile char TWI_TXBUFFER[30];

void TWIInit(void) //Function to initialize the TWI and be ready to acknowledge call
{
	TWAR = 0xFE;
	TWCR = (1<<TWEN);						//TWI Interface enable
	TWCR |= (1<<TWIE)|(1<<TWINT);           //Enable TWI Interupt and clear the flag
	TWCR |= (1<<TWEA);						//Prepare to ACK next time Slave is addressed                          
	TWI_Busy = 0;
}

ISR(TWI_vect){								//Interrupt handler for the TWI which is triggered every time the master device call Hermes' address
	static unsigned char TWI_RX_Ptr;
	static unsigned char TWI_TX_Ptr = 0;
	
	switch (TWSR)
	{
		case TWI_STX_ADR_ACK:            //Own SLA+R has been received; ACK has been returned
		
		TWI_RX_Ptr  = 0;                 //Set buffer pointer to first data location
		
		case TWI_STX_DATA_ACK:           //Data byte in TWDR has been transmitted; ACK has been received

		TWDR = TWI_TXBUFFER[TWI_TX_Ptr]; //Send byte to Athena
		TWI_TX_Ptr++;
		
		TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA); //Ready TWI to receive again
		TWI_Busy = 1;
		
		break;
		
		case TWI_STX_DATA_NACK:          //Data byte in TWDR has been transmitted. NACK has been received. This is usually how Athena responds, i.e. by not responding.
		
		TWI_Status = 1;					 //Set status for a sucessful transmission
		TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA); //Ready TWI to receive again

		TWI_Busy = 1;					//Keep busy until stop condition
		break;
		
		case TWI_SRX_ADR_ACK:            //Own SLA+W has been received ACK has been returned
		TWI_RX_Ptr = 0;                                 //Set buffer pointer to first data location
		
		TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA); //Ready TWI to receive again
		
		TWI_Busy = 1;
		break;
		
		case TWI_SRX_ADR_DATA_ACK:       //Previously addressed with own SLA+W; data has been received; ACK has been returned
		
		TWI_RXBUFFER[TWI_RX_Ptr] = TWDR; //Save byte from Athena
		
		if (TWI_RXBUFFER[TWI_RX_Ptr]==0xFF)	//If byte received is 0xFF, reset the ptr for the TX buffer as Athena wants the whole buffer from the beginning
		{
			TWI_TX_Ptr = 0;
		}
		
		TWI_RX_Ptr++;
		TWI_RXBUFFER[TWI_RX_Ptr] = 0;	//Make sure to end the string with a 0!
		
		TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA); //Ready TWI to receive again 
		
		TWI_Busy = 1;
		TWI_Status = 1;
		break;
		
		case TWI_SRX_STOP_RESTART:       //A STOP condition or repeated START condition has been received while still addressed as Slave
		// Enter not addressed mode and listen to address match
		TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA); //Ready TWI to receive again
		
		TWI_Busy = 0;  //We are waiting for a new address match, so we are not busy
		break;
		
		case TWI_SRX_ADR_DATA_NACK:      //Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
		case TWI_STX_DATA_ACK_LAST_BYTE: //Last data byte in TWDR has been transmitted (TWEA = “0”); ACK has been received
		case TWI_BUS_ERROR:         //Bus error due to an illegal START or STOP condition
		TWI_Status = 0;				//Error
		TWI_Busy = 0;
		TWCR = (1<<TWSTO)|(1<<TWINT);   //Recover from TWI_BUS_ERROR, this will release the SDA and SCL pins thus enabling other devices to use the bus
		break;
		
		default:
		TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA); //Ready TWI to receive again
		TWI_Status = 0;								//Error
		TWI_Busy = 0; //Unknown status, so we wait for a new address match
	}
}