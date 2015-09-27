/**********************  DRIVER FOR SPI CONTROLLER ON MSP430**********************


*******************************************************************************/
#include "Serialize.h"
#include <msp430.h>
#include <stdio.h>

//#define ENABLE_PRINT_DEBUG

#define EXT_MOD

//SPI for Ext Flash
#define USCI_UNIT	B
#define USCI_CHAN	2
// Pin definitions for this unit
#define	FLASH_SPI_SIMO                MSP430_GPIO(9,1)	// These macros not yet useable
#define	FLASH_SPI_SOMI                MSP430_GPIO(9,2)
#define	FLASH_SPI_CLK                 MSP430_GPIO(9,3)
#define	FLASH_SPI_CS                  MSP430_GPIO(9,0)
#define	FLASH_SPI_RST                  MSP430_GPIO(9,7)

#define PREEXPAND4(x, _x_, _n_, y)	EXPAND4(x, _x_, _n_, y)
#define EXPAND4(x, _x_, _n_, y)	x ##_x_ ##_n_ ##y

#define PREEXPAND3(x, _x_, _n_)	EXPAND3(x, _x_, _n_)
#define EXPAND3(x, _x_, _n_)	x ##_x_ _n_

#define MSP430_GPIO(_port, _pin) 	((_port - 1) << 8 | (1 << _pin))
#define GPIO_PORT(_gpio) ((_gpio >> 8) & 0xFF)
#define GPIO_PIN(_gpio) (_gpio & 0xFF)

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

#define SET_CS (P9OUT |= 1<<0)

#define CLEAR_CS (P9OUT &= ~(1<<0))

SPI_STATUS msp430_spi_init(void)
{

	P9DIR = 0x8B;	// P9.(0,1,3,7) as output
	P9SEL = 0x0E;	// P9.(1,2,3) as special
	P9OUT = 0x01;	// Set CS High

	__disable_interrupt();
	UCxnCTL1 |= UCSWRST;				// Put state machine in reset
	UCxnCTL0 |= UCMST+UCSYNC+UCMSB+UCCKPH;		// Master, synchronous, MSB
	UCxnCTL1 |= UCSSEL_2;                     // SMCLK
    UCxnBR0 = 1;				// f_UCxCLK = 20MHz/1 = 20MHz
    UCxnBR1 = 0;		//
	//UCxnIE = 0x00;								// All interrupts disabled

	UCxnCTL1 &= ~UCSWRST;                  		// Release state machine from reset
	__enable_interrupt();

	return RetSpiSuccess;

}

/*******************************************************************************
Function:     ConfigureSpi(SpiConfigOptions opt)
Arguments:    opt configuration options, all acceptable values are enumerated in
              SpiMasterConfigOptions, which is a typedefed enum.
Return Values:There is no return value for this function.
Description:  This function can be used to properly configure the SPI master
              before and after the transfer/receive operation
Pseudo Code:
   Step 1  : perform or skip select/deselect slave
   Step 2  : perform or skip enable/disable transfer
   Step 3  : perform or skip enable/disable receive
*******************************************************************************/

void ConfigureSpi(SpiConfigOptions opt)
{
	switch (opt)
	{
	case OpsWakeUp:
		CHECK_BSY;
		CLEAR_CS;
		break;
	case OpsInitTransfer:
		break;
	case OpsEndTransfer:
		SET_CS;
		break;
	default:
		break;
	}
}



/*******************************************************************************
Function:     Serialize(const CharStream* char_stream_send,
					CharStream* char_stream_recv,
					SpiMasterConfigOptions optBefore,
					SpiMasterConfigOptions optAfter
				)
Arguments:    char_stream_send, the char stream to be sent from the SPI master to
              the Flash memory, usually contains instruction, address, and data to be
              programmed.
              char_stream_recv, the char stream to be received from the Flash memory
              to the SPI master, usually contains data to be read from the memory.
              optBefore, configurations of the SPI master before any transfer/receive
              optAfter, configurations of the SPI after any transfer/receive
Return Values:TRUE
Description:  This function can be used to encapsulate a complete transfer/receive
              operation
Pseudo Code:
   Step 1  : perform pre-transfer configuration
   Step 2  : perform transfer/ receive
   Step 3  : perform post-transfer configuration
*******************************************************************************/
SPI_STATUS Serialize_SPI(const CharStream* char_stream_send,
                         CharStream* char_stream_recv,
                         SpiConfigOptions optBefore,
                         SpiConfigOptions optAfter
                        )
{

	uint8 *char_send, *char_recv;
	uint16 rx_len = 0, tx_len = 0;

#ifdef ENABLE_PRINT_DEBUG
	int i;
	printf("\nSEND: ");
	for(i=0; i<char_stream_send->length; i++)
		printf(" 0x%x ", char_stream_send->pChar[i]);
	printf("\n");
#endif

	tx_len = char_stream_send->length;
	char_send = char_stream_send->pChar;

	if (NULL_PTR != char_stream_recv)
	{
		rx_len = char_stream_recv->length;
		char_recv = char_stream_recv->pChar;
	}



	ConfigureSpi(optBefore);


	uint16 gie = __get_SR_register() & GIE;   // Store current GIE state

    __disable_interrupt();                      // Make this operation atomic

    // TX

    // Clock the actual data transfer and send the bytes. Note that we
    // intentionally not read out the receive buffer during frame transmission
    // in order to optimize transfer speed, however we need to take care of the
    // resulting overrun condition.
    while (tx_len--){
        while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
        UCxnTXBUF = *char_send++;                 // Write byte
    }
    while (UCxnSTAT & UCBUSY) ;                 // Wait for all TX/RX to finish

    UCxnRXBUF;                                  // Dummy read to empty RX buffer
                                                // and clear any overrun conditions

    // RX

    UCxnIFG &= ~UCRXIFG;                        // Ensure RXIFG is clear

    // Clock the actual data transfer and receive the bytes
    while (rx_len--){
        while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
        UCxnTXBUF = 0xff;                       // Write dummy byte
        while (!(UCxnIFG & UCRXIFG)) ;          // Wait for RX buffer (full)
        *char_recv++ = UCxnRXBUF;
    }

    __bis_SR_register(gie);                     // Restore original GIE state


#ifdef ENABLE_PRINT_DEBUG
	printf("\nRECV: ");
	for(i=0; i<char_stream_recv->length; i++)
		printf(" 0x%x ", char_stream_recv->pChar[i]);
	printf("\n");
#endif

	ConfigureSpi(optAfter);


	return RetSpiSuccess;
}

/*void four_byte_addr_ctl(int enable)
{
	if(enable)
		FOUR_BYTE_ENABLE;

	if(!enable)
		FOUR_BYTE_DISABLE;
}*/
