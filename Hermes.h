/*
 * Hermes.h
 *
 * Amherst College Electronics Club - Nov 2013
 *  Author: André Lucas Antunes de Sá
 */ 


#ifndef HERMES_H_
#define HERMES_H_

#define F_CPU 18432000
#define ADCREF 5.04				//Average Vcc with everything on
#define CUT_OFF_TIMER 10800.0	//3 hours of flight
#define CUT_DURATION 15000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "Radio.h"
#include "USART.h"
#include "TWI.h"

void ioInit(void);
void analogInit(void);
void timerInit(void);

float battery_voltRead(void);
char RadioAnswerCMD(char RadioRX, char Hermes_State);
char RadioCheckRX(char Radio_RX);
char TWIGetState(char Hermes_State);
void numberToASCII(char *str, unsigned long number);
void floatToASCII(char *str, float number); //Up to 3 decimal places and be careful with numbers larger than 100000

/*Pin Definitions

Relay - PD5 and PD6
Batt - ADC0
Green Led - PB0
Red Led - PB1

*/

#endif /* HERMES_H_ */