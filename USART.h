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
#include <inttypes.h>

void USARTInit(const uint16_t ubrr);
char USARTReadChar(void);
void USARTWriteChar(const char data);
void USARTPrint(const char *data);

#endif /* USART_H_ */