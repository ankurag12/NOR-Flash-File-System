/*
 * msp430_spi.h
 *
 *  Created on: Oct 4, 2015
 *      Author: Robo
 */

#ifndef MSP430_SPI_H_
#define MSP430_SPI_H_

//SPI for Ext Flash
#define USCI_UNIT	B
#define USCI_CHAN	2
// Pin definitions for this unit
//#define	FLASH_SPI_SIMO                MSP430_GPIO(9,1)	// These macros not yet useable
//#define	FLASH_SPI_SOMI                MSP430_GPIO(9,2)
//#define	FLASH_SPI_CLK                 MSP430_GPIO(9,3)
//#define	FLASH_SPI_CS                  MSP430_GPIO(9,0)
//#define	FLASH_SPI_RST                  MSP430_GPIO(9,7)

#define PREEXPAND4(x, _x_, _n_, y)	EXPAND4(x, _x_, _n_, y)
#define EXPAND4(x, _x_, _n_, y)	x ##_x_ ##_n_ ##y

#define PREEXPAND3(x, _x_, _n_)	EXPAND3(x, _x_, _n_)
#define EXPAND3(x, _x_, _n_)	x ##_x_ _n_

//#define MSP430_GPIO(_port, _pin) 	((_port - 1) << 8 | (1 << _pin))
//#define GPIO_PORT(_gpio) ((_gpio >> 8) & 0xFF)
//#define GPIO_PIN(_gpio) (_gpio & 0xFF)

#define	UCxnCTL0	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, CTL0)
#define	UCxnCTL1	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, CTL1)
#define UCxnBR0		PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, BR0)
#define	UCxnBR1		PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, BR1)
#define	UCxnIE		PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, IE)
#define	UCxnIFG		PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, IFG)
#define	UCxnTXBUF	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, TXBUF)
#define	UCxnRXBUF	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, RXBUF)
#define	UCxnSTAT	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, STAT)

//#define	UCxnI2COA	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, I2COA)
//#define	UCxnI2CSA  	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, I2CSA)

#define	UCxnMCTL  	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, MCTL)
#define	UCxnIV  	PREEXPAND4(UC, USCI_UNIT, USCI_CHAN, IV)

#define	USCI_xn_VECTOR  	PREEXPAND4(USCI_, USCI_UNIT, USCI_CHAN, _VECTOR)


//#define SET_HIGH(_port, _pin) 		(PREEXPAND3(P, _port, OUT) |= (1 << (_pin)))
//#define SET_LOW(_port, _pin) 		(PREEXPAND3(P, _port, OUT) &= ~(1 << (_pin)))
//#define SET_OUTPUT(_port, _pin) 	(PREEXPAND3(P, _port, DIR) |= (1 << (_pin)))
//#define SET_SPECIAL(_port, _pin) 		(PREEXPAND3(P, _port, SEL) |= (1 << (_pin)))

#define CHECK_BSY \
   while (UCxnSTAT & UCBUSY);

#define SET_CS (P9OUT |= 1<<0)			// HARD CODED

#define CLEAR_CS (P9OUT &= ~(1<<0))		// HARD CODED


void msp430_spi_init(void);

#endif /* MSP430_SPI_H_ */
