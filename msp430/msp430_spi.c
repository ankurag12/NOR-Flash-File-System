/*
 * msp430_spi.c
 *
 *  Created on: Oct 4, 2015
 *      Author: Robo
 */
#include "msp430_spi.h"
#include <msp430.h>

void msp430_spi_init(void)
{

	P9DIR = 0x8B;	// P9.(0,1,3,7) as output		// HARD CODED
	P9SEL = 0x0E;	// P9.(1,2,3) as special		// HARD CODED
	P9OUT = 0x01;	// Set CS High					// HARD CODED

	__disable_interrupt();
	UCxnCTL1 |= UCSWRST;				// Put state machine in reset
	UCxnCTL0 |= UCMST+UCSYNC+UCMSB+UCCKPH;		// Master, synchronous, MSB
	UCxnCTL1 |= UCSSEL_2;                     // SMCLK
    UCxnBR0 = 1;				// f_UCxCLK = 20MHz/1 = 20MHz
    UCxnBR1 = 0;		//
	//UCxnIE = 0x00;								// All interrupts disabled

	UCxnCTL1 &= ~UCSWRST;                  		// Release state machine from reset
	__enable_interrupt();


}
