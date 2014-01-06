/*
 * Hermes.c
 *
 * Amherst College Electronics Club - Nov 2013
 *  Author: André Lucas Antunes de Sá
 
 This program is the firmware for the Hermes Computer, one of the flight computers on board the payload of the Electronics Club's high altitude balloon project, codename Daedalus.
 Hermes is connected to the other flight computer, Athena, through the I2C bus as a slave, and to a serial radio transceiver module. The main purpose of the program is to receive 
 data from Athena and relay it to the radio, transmit its own status to Athena through the I2C and initiate emergency cut-down if command is received on the radio or if a given 
 set of requirements are met.
  
 */ 

#include "Hermes.h"

extern volatile uint8_t TWI_Busy;			//This byte is set to 1 while the TWI is in the middle of transmitting/receiving, otherwise it's 0 (Check TWI.c for more information)
extern volatile uint8_t TWI_Status;			//This byte is set to 1 every time a TWI operation is successful
extern volatile char TWI_RXBUFFER[164];
extern volatile char TWI_TXBUFFER[30];
volatile float time = 0;					//Time elapsed since initialization

ISR(TIMER1_COMPA_vect)
{
	time += 0.1;							//Timer overflows every tenth of a second
}

int main(void)
{
	char buffer[16];
	char Radio_RX = 0;
	float fvar = 0;
	uint8_t Hermes_State = 0;
	_delay_ms(1000);
	/*
	Only the 3 LSB of Hermes_State are used
	The 0th bit is assigned the same value as TWI_Status in every iteration of the main loop
	The 1st bit is set after the emergency cut-down system has been activated (relays were closed for some time)
	The 2rd bit is set when conditions are met for the emergency cut-down to be executed
	*/
	
	timerInit();
	ioInit();
	analogInit();
	USARTInit(119);
	sei();									//Enable interrupts
	
	//RadioSetup();							//Uncomment to setup radio device
	
	USARTPrint("Falling, Athena-Time, AccelX(mg), AccelY(mg), AccelZ(mg), GyroX(dps),"
	" Baro(mbar), ExtBaro(mbar), TmpOut(C), TmpIn(C), Batt(V), UTC, Lat, Long, Fix, Sat, HDOP, Alt, Heading, Speed,"
	" CPM, uSv/Hr, S/F, Athena-Status, Hermes-BatVolt(V), Hermes-Status\n\r");
	
	PORTB &= ~(1<<PORTB0);					//Turn-off green LED after initialization
	
	while(1)								//Main loop
	{
		if ((uint32_t)time%10UL==0)				//Blink RED LED every 10 seconds to show controller is functional
		{
			PORTB |= 1<<PORTB1;
			_delay_ms(100);
			PORTB &= ~(1<<PORTB1);
		}
		
		for (uint32_t i=0xFFFFFFFFUL; i!=0; --i) //If TWI is busy, wait until operation finishes in order to disable TWI soon or bail out after a while
		{
			if (TWI_Busy==0) break;
			_delay_us(10);
		}
		TWI_Busy = 0;
		TWCR = (1<<TWEN);					 //Disable TWI to make sure we don't update TWI_TXBUFFER while the TWI is enabled and could transmit
		
		////////////////////////////////////// UPDATE TWI_TXBUFFER
		
		uint8_t str_pos = 0;
		
		floatToASCII(buffer,time);
		for (uint8_t i=0; buffer[i]!=0; ++i, ++str_pos)
		{
			TWI_TXBUFFER[str_pos] = buffer[i];
		}
		TWI_TXBUFFER[str_pos] = ',';
		++str_pos;
	
		fvar = battery_voltRead();
		floatToASCII(buffer,fvar);
		for (uint8_t i=0; buffer[i]!=0; ++i, ++str_pos)
		{
			TWI_TXBUFFER[str_pos] = buffer[i];
		}
		TWI_TXBUFFER[str_pos] = ',';
		++str_pos;

		TWI_TXBUFFER[str_pos] = Radio_RX;		
		++str_pos;
		TWI_TXBUFFER[str_pos] = ',';
		++str_pos;
		
		TWI_TXBUFFER[str_pos] = Hermes_State + '0';
		++str_pos;
		TWI_TXBUFFER[str_pos] = ',';
		++str_pos;
		TWI_TXBUFFER[str_pos] = 0;
		
		/////////////////////////////////////
		
		TWIInit();							//We are ready to transmit again after TWI_TXBUFFER was updated
		
		Radio_RX = 0;
		if ((UCSR0A & (1<<RXC0)))			//Check for incoming RX from ground
		{
			Radio_RX = RadioCheckRX(Radio_RX);
		}
		
		
		if (Radio_RX!=0)			//Answer ground if HMS Command has been received
		{
			Hermes_State = RadioAnswerCMD(Radio_RX, Hermes_State);
		}
		
		
		if ((TWI_RXBUFFER[0]=='1') && (time - CUT_OFF_TIMER > 0))	//Set emergency-cut down to be executed if Athena is giving the go (not in free fall or landing) and CUT_OFF_TIMER seconds has elapsed
		{
			Hermes_State |= 1<<2;
		}
		
		
		if ((Hermes_State>>1) == 0x02)		//Turn on relays for CUT_DURATION seconds if Hermes_State is set to initiate emergency cut-down and if it hasn't previously been executed
		{
			PORTD |= (1<<PORTD5)|(1<<PORTD6);
			_delay_ms(CUT_DURATION);
			PORTD &= ~((1<<PORTD5)|(1<<PORTD6));
			USARTPrint("Cut off successful!\r\n");
			
			Hermes_State |= 1<<1;
		}
		
		Hermes_State = TWIGetState(Hermes_State);	//Update the first bit of Hermes_State to reflect status of last TWI operation
		
		/////////////////////////////////// Transmit data to ground when time is a multiple of 3
		if ((uint32_t)time%3UL == 0)
		{
			
			for (uint16_t i=0xFFFFU; i!=0; --i)	//If TWI is busy, wait until operation finishes in order to disable TWI soon or bail out after a while
			{
				if (TWI_Busy==0) break;
				_delay_us(10);
			}
			TWI_Busy = 0;
			TWCR = (1<<TWEN);						//Disable TWI to make sure we don't update TWI_TXBUFFER while the TWI is enabled and could transmit
					
			USARTPrint("HMS-GND:");
			USARTPrint((char*)TWI_RXBUFFER);		//Relay information received from Athena
			USARTWriteChar(',');
			fvar = battery_voltRead();
			floatToASCII(buffer,fvar);
			USARTPrint(buffer);
			USARTWriteChar(',');		
			USARTWriteChar(Hermes_State + '0');
			USARTPrint("\r\n");
			
			TWIInit();
		}
		
		_delay_ms(100);
	}
}

void ioInit(void)
{
	DDRB |= (1<<PORTB0)|(1<<PORTB1);	//LEDs
	DDRD |= (1<<PORTD5)|(1<<PORTD6);	//Relay
	
	PORTB |= 1<<PORTB0;					//Start-up with Green Led on
}

void analogInit(void)
{
	ADMUX |= (1<<REFS0);				//Voltage Reference Selection: AVcc with External capacitor at Aref

	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	//Select clock for the ADC: Main clock/1024 (Approx 20KHz)
	ADCSRA |= (1<<ADEN);				//Enable the ADC
	ADCSRA |= (1<<ADSC);				//Start a conversion to initialize the ADC	
}

void timerInit(void)
{
	TCCR1B |= 1<<WGM12;					//Set timer to overflow once it reaches OCR1A
	TCCR1B |= (1<<CS12) | (1<<CS10);	//Set clock to be prescaled to F_CPU/1024
	OCR1AH = 1800>>8;					//Set top of the counter to 1800 (This gives as an overflow every tenth of a second)
	OCR1AL = 1800 & 0xFF;
	TIMSK1 |= 1<<OCIE1A;				//Enable the timer overflow interrupt
}

float battery_voltRead(void)
{
	char ADClow = 0;
	char ADChigh = 0;
	unsigned int result;
	float voltage = 0;
	
	ADCSRA |= 1<<ADSC;		//Do a single conversion.
	_delay_us(100);
	
	ADClow = ADCL;
	ADChigh = ADCH;
	
	result = (ADChigh<<8 | ADClow);
	voltage = (float)result*(ADCREF/1023.0)*1.7046; //The last number is for the voltage divider. Let's hope that the temperature coeff for the resistors is indeed the same and their ratio stays the same!
	
	return voltage;
}

char RadioAnswerCMD(char Radio_RX, char Hermes_State)
{
	if (Radio_RX == 'C')	//If last char received was a 'C', confirm emergency cut-down through radio
	{
		USARTPrint("Are you sure?(Type Y)\r\n");
		
		for (unsigned int i=1000; i!=0; i--)	//Wait until next character is received or bail out
		{
			if ((UCSR0A & (1<<RXC0))) break;
			_delay_ms(10);
		}
		
		Radio_RX = UDR0;
		
		if ( Radio_RX == 'Y')
		{
			USARTPrint("Last Chance?(Type D)\r\n");
			
			for (unsigned int i=1000; i!=0; i--)	//Wait until next character is received or bail out
			{
				if ((UCSR0A & (1<<RXC0))) break;
				_delay_ms(10);
			}
			
			Radio_RX = UDR0;
			
			if (Radio_RX == 'D')
			{
				Hermes_State |= 1<<2;
			}
		}
	}
	else
	{
		USARTPrint("Hello!\r\n");	//Send Hello if HMS + any char but capital C was received
	}
	
	return Hermes_State;
}

char RadioCheckRX(char Radio_RX)
{
	char data = 0;	
	
	for (unsigned int i=1000; i!=0; i--)
	{
		data=USARTReadChar();
		if (data == 'H')
		{
			USARTWriteChar(data);
			data = USARTReadChar();
			if (data == 'M')
			{
				USARTWriteChar(data);
				data = USARTReadChar();
				if (data == 'S' )
				{
					USARTWriteChar(data);
					Radio_RX = USARTReadChar();
					USARTPrint("CMD:");
					USARTWriteChar(Radio_RX);
					USARTPrint("\r\n");
				}
			}
		}
		
		_delay_ms(10);
	}
	
	return Radio_RX;
}

char TWIGetState(char Hermes_State)
{
	if (TWI_Busy == 0)
	{
		if (TWI_Status)		//Check if last TWI operation was successful
		{
			Hermes_State |= 1<<0;
			TWI_Status = 0;
		}
		else
		{
			Hermes_State &= ~(1<<0);
			TWI_Status = 0;
			TWCR =   (1<<TWSTO)|(1<<TWINT);   //Recover from error by releasing SDA and SCL pins and thus enabling other devices to use the bus
			TWIInit();
		}
	}
	
	return Hermes_State;
}

void numberToASCII(char * str, int32_t number) //Prints the value of a variable number into a a string of ASCII characters (My own simplified version of the same feature in the printf function)
{
	uint32_t tens = 1000000000UL;
	uint8_t digits[10];
	uint8_t i = 0;
	uint8_t j = 0;

	if (number < 0)
	{
		number *= -1;
		str[j]='-';
		++j;
	}
	
	while (i<10U)
	{
		digits[i] = number/tens;
		number = number%tens;
		tens /= 10U;
		++i;
	}

	for (i=0; i<10U; ++i)
	{
		if (digits[i]!=0)
		{
			break;
		}
	}
	
	if (i!=10U)
	{
		for (; i<10U; ++i, ++j)
		{
			str[j] = digits[i] + '0';
		}
	}
	else
	{
		str[j]='0';
		++str;
	}
	
	str[j]=0;
	
}

/*
Same as above but for float numbers
Up to 3 decimal place
Be careful with numbers larger than 100000
*/

void floatToASCII(char * str, float number) //This function can be greatly optimized for speed. Using an array for the tens variable would greatly improve it at the cost of some memory.
{
	float tens = 1000000000.0;
	float rational = 0.0;
	uint8_t digits[13];
	uint8_t i = 0;
	uint8_t j = 0;
	
	if (number < 0.0)
	{
		number *= -1.0;
		str[j]='-';
		++j;
	}
	
	rational = number - (int32_t)number;
	
	while (i<10U)
	{
		digits[i] = number/tens;
		number = (int32_t)number%(int32_t)tens;
		tens /= 10.0;
		++i;
	}

	for (i=0; i<10U; ++i)
	{
		if (digits[i]!=0)
		{
			break;
		}
	}
	
	if (i!=10U)
	{
		for (; i<10U; ++i, ++j)
		{
			str[j] = digits[i] + '0';
		}
	}
	else
	{
		str[j]='0';
		++str;
	}
	//////////// Up to here the result was the same as the function above but below we also take care of digits to the right of the decimal point
	
	str[j] = '.';
	++str;
	
	i = 10U;
	
	while (i<13U)
	{
		tens = 10.0;
		rational *= tens;
		digits[i] = rational;
		rational -= digits[i];
		tens *= 10.0;
		++i;
	}

	for (i=12U; i>9U; --i)
	{
		if (digits[i]!=0)
		{
			break;
		}
	}
	
	if (i!=9U)
	{
		for (uint8_t k=10U; k<(i+1); ++k,++j)
		{
			str[j] = digits[k] + '0';
		}
	}
	else
	{
		str[j]='0';
		++str;
	}
	
	str[j]=0;
	
}