/*
 * Radio.h
 *
 * Amherst College Electronics Club - Nov 2013
 *  Author: Matthew Kurek
 */ 


#ifndef RADIO_H_
#define RADIO_H_

#include "USART.h"

void RadioSetup(void);
void RadioSendCmd(const char *data);	//Sends an AT command with its parameters
void RadioSetPower(const uint8_t p);		//Change power (0-4 where 4 corresponds to 1W)
void RadioTransmit(const char *data);	//Generic transmission command.

#endif /* RADIO_H_ */