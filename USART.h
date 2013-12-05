/*
 * USART.h
 *
 * Amherst College Electronics Club - Nov 2013
 *  Author: André Lucas Antunes de Sá
 */ 


#ifndef USART_H_
#define USART_H_

#define F_CPU 18432000

#include <avr/io.h>
#include <util/delay.h>

void USARTInit(unsigned int ubrr);
char USARTReadChar(void);
void USARTWriteChar(char data);
void USARTPrint(char *data);

#endif /* USART_H_ */