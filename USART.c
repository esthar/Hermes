/*
 * USART.c
 *
 * Amherst College Electronics Club - Nov 2013
 *  Author: André Lucas Antunes de Sá
 */ 

#include "USART.h"

void USARTInit(unsigned int ubrr)
{

  
	UBRR0L = ubrr;		//Set Baud Rate
	UBRR0H = (ubrr>>8);

	/*Set Frame Format. This is the default for Asynchronous serial


	>> Asynchronous mode
	>> No Parity
	>> 1 StopBit

	>> char size 8
	*/
   
	UCSR0C= (1<<UCSZ00)|(1<<UCSZ01); //8-bit frame


	UCSR0B=(1<<RXEN0)|(1<<TXEN0); //Enable TX and RX
}

char USARTReadChar()
{
   //Wait until data is available or bail out

 	char i = 255;

 	while(!(UCSR0A & (1<<RXC0))){
	 	if(i==0)
	 	{
		 	break;
	 	}
	 	i--;
	 	_delay_us(10);
 	}

   //USART has gotten data from device and is available on the buffer

   return UDR0;
}

void USARTWriteChar(char data)
{
   //Wait until the transmitter is ready or bail out

 	char i = 255;

 	while(!(UCSR0A & (1<<UDRE0))){
	 	if(i==0)
	 	{
		 	break;
	 	}
	 	i--;
	 	_delay_us(10);
 	}

   //Write the data to USART buffer

   UDR0=data;
}

void USARTPrint(char *data)			//Sends a series of characters through the serial until the string ends with a termination character, i.e. a 0
{
	for (unsigned int i=0;data[i]!=0;i++)
	{
		while(!(UCSR0A & (1<<UDRE0)));
		UDR0=data[i];
	}
}