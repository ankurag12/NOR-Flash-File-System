/**********************  DRIVER FOR SPI CONTROLLER ON MSP430**********************


*******************************************************************************/
#include "Serialize.h"
#include <msp430.h>
#include <stdio.h>
#include "../msp430/msp430_spi.h"

//#define ENABLE_PRINT_DEBUG

#define EXT_MOD

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
