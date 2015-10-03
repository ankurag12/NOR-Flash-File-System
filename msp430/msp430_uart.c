/*
 * MSP430_UART.c
 *
 *  Created on: Oct 4, 2015
 *      Author: Robo
 */

#include <msp430.h>
#include "msp430_uart.h"


int msp430_uart_putc(int c)
{
	while(!(UCA1IFG & UCTXIFG));
	UCA1TXBUF = (unsigned char)c;

	return (unsigned char)c;
}


int msp430_uart_puts(unsigned char *s)
{
	unsigned int i;

	for(i = 0; s[i]; i++)
		msp430_uart_putc(s[i]);

	return i;
}

void msp430_init_uart(baud_rate baud)
{
	P5SEL = 0xC0;                             // P3.4,5 = USCI_A2 TXD/RXD
	UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**

	UCA1CTL0 = UCMODE_0;			// Uart Mode (No parity, LSB first, 8 data bits, 1 stop bit)
	UCA1CTL1 |= UCSSEL_2;			// SMCLK

	/* These registers are dependent on a 20MHz clock */
	switch (baud)
	{
		case BR_9600:
			UCA1BR0 = 130;
			UCA1BR1 = 0;
			UCA1MCTL = (UCOS16 | UCBRS_0 | UCBRF_3);
			break;
		case BR_19200:
			UCA1BR0 = 65;
			UCA1BR1 = 0;
			UCA1MCTL = (UCOS16 | UCBRS_0 | UCBRF_2);
			break;
		case BR_38400:
			UCA1BR0 = 32;
			UCA1BR1 = 0;
			UCA1MCTL = (UCOS16 | UCBRS_0 | UCBRF_9);
			break;
		case BR_57600:
			UCA1BR0 = 21;
			UCA1BR1 = 0;
			UCA1MCTL = (UCOS16 | UCBRS_0 | UCBRF_11);
			break;
		case BR_115200:
			UCA1BR0 = 10;
			UCA1BR1 = 0;
			UCA1MCTL = (UCOS16 | UCBRS_0 | UCBRF_14);
			break;
		case BR_230400:
			UCA1BR0 = 5;
			UCA1BR1 = 0;
			UCA1MCTL = (UCOS16 | UCBRS_0 | UCBRF_7);
			break;
		default:			// Default is 9600
			UCA1BR0 = 130;
			UCA1BR1 = 0;
			UCA1MCTL = (UCOS16 | UCBRS_0 | UCBRF_3);
			break;
	}


	UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA1IE |= UCRXIE;                         // Enable USCI_A2 RX interrupt

	__enable_interrupt();       // interrupts enabled
}
