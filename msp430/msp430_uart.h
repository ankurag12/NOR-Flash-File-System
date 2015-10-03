/*
 * MSP430_UART.h
 *
 *  Created on: Oct 4, 2015
 *      Author: Robo
 */

#ifndef MSP430_UART_H_
#define MSP430_UART_H_


//#define USCI_UNIT	A
//#define	USCI_CHAN	1
// Pin definitions for this unit. (Just for information, actual configuration has been hard-coded)
#define	UART_TX                 MSP430_GPIO(5,6)
#define	UART_RX                 MSP430_GPIO(5,7)


typedef enum {
	BR_9600,
	BR_19200,
	BR_38400,
	BR_57600,
	BR_115200,
	BR_230400,
} baud_rate;

int msp430_uart_putc(int c);
int msp430_uart_puts(unsigned char *s);
void msp430_init_uart(baud_rate baud);


#endif /* MSP430_UART_H_ */
