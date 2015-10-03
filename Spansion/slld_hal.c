/* slld_hal_example.c - SLLD Hardware Abstraction Layer example Code */
 
/**************************************************************************
* Copyright 2011 Spansion LLC. All Rights Reserved. 
*
* This software is owned and published by: 
* Spansion LLC, 915 DeGuigne Drive, Sunnyvale, CA 94088 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software constitutes driver source code for use in programming Spansion's 
* Flash memory components. This software is licensed by Spansion to be adapted only 
* for use in systems utilizing Spansion's Flash memories. Spansion is not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein.  Spansion is providing this source code "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the source code herein.  
*
* Spansion MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE, 
* REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED 
* USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY, 
* FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  Spansion WILL 
* HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR 
* OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE, 
* INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL, 
* SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS, 
* EVEN IF Spansion HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Copyright notice must be included with 
* this software, whether used in part or whole, at all times.  
*/

#include <stdio.h>
#include "slld_targetspecific.h"
#include "slld.h"
#include "slld_hal.h"
#include <msp430.h>
#include <stdint.h>
#include "../msp430/msp430_spi.h"



// ***************************************************************************
//  FLASH_READ - HAL read function
//
//  input : device_num            device number to which operation will be done
//          command               write a single command byte to flash
//          sys_addr              system address to be used
//          data_buffer           Pointer to the data buffer where to store the read data
//          Number_Of_Read_Bytes  number of bytes to be read
//
//  return value : status of the operation - FAIL or SUCCESS
// ***************************************************************************

SLLD_STATUS FLASH_READ
(
BYTE     command,                        /* write a single command byte to flash */
ADDRESS  sys_addr,                       /* system address to be used */
BYTE    *data_buffer,                    /* Pointer to the data buffer containing data to be written */
int      Number_Of_Read_Bytes            /* number of bytes to be read */
)
{
    SLLD_STATUS status = SLLD_OK;
    int data_cycle;
	//int Number_Of_Dummy_Bytes = 0;
    int i;

    // Select the device
	CHECK_BSY;
	CLEAR_CS;

	WORD gie = __get_SR_register() & GIE;   // Store current GIE state

    __disable_interrupt();                      // Make this operation atomic
    
    while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
    // Write the command
	UCxnTXBUF = command;

    // Write the address
    if (sys_addr != ADDRESS_NOT_USED)
    {
        switch (command)
        {
            case SPI_DUALIO_HPRD_CMD:
            case SPI_DUALIO_HPRD_4B_CMD:
            {
            	for (i=16; i>=0; i-=8) {
            		 while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
            		 UCxnTXBUF = (BYTE)((sys_addr >> i) & 0x000000FF);		// Change UCxnTXBUF as required by mode
            	}

                break;
            }
            case SPI_QUADIO_HPRD_CMD:
            case SPI_QUADIO_HPRD_4B_CMD:
            {
            	for (i=16; i>=0; i-=8) {
            		 while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
            		 UCxnTXBUF = (BYTE)((sys_addr >> i) & 0x000000FF);	// Change UCxnTXBUF as required by mode
            	}
                break;
            }
            default:
            {
            	for (i=16; i>=0; i-=8) {
            		 while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
            		 UCxnTXBUF = (BYTE)((sys_addr >> i) & 0x000000FF);	// Change UCxnTXBUF as required by mode
            	}
                break;
            }
        }
    }

    UCxnIFG &= ~UCRXIFG;                        // Ensure RXIFG is clear


    // Read the data
    if (Number_Of_Read_Bytes != 0)
    {
        switch (command)
        {
            case SPI_DUALIO_RD_CMD:
            case SPI_DUALIO_RD_4B_CMD:
            case SPI_DUALIO_HPRD_CMD:
            case SPI_DUALIO_HPRD_4B_CMD:
            {
                // Read the data using the relevant mode


                for (data_cycle = 0; data_cycle < Number_Of_Read_Bytes; data_cycle++) {
                    while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
                    UCxnTXBUF = 0xff;                       // Write dummy byte
                	while (!(UCxnIFG & UCRXIFG)) ;          	// Wait for RX buffer (full)
                	*(data_buffer + data_cycle) = UCxnRXBUF;	// Change UCxnRXBUF as required by mode
                }
                break;
            }
            case SPI_QUADIO_RD_CMD:
            case SPI_QUADIO_RD_4B_CMD:
            case SPI_QUADIO_HPRD_CMD:
            case SPI_QUADIO_HPRD_4B_CMD:
            {
                // Read the data using the relevant mode
                for (data_cycle = 0; data_cycle < Number_Of_Read_Bytes; data_cycle++) {
                    while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
                    UCxnTXBUF = 0xff;                       // Write dummy byte
                	while (!(UCxnIFG & UCRXIFG)) ;          // Wait for RX buffer (full)
                	*(data_buffer + data_cycle) = UCxnRXBUF;// Change UCxnRXBUF as required by mode
                }
                break;
            }
            default:
            {
                // Read the data using the relevant mode
                for (data_cycle = 0; data_cycle < Number_Of_Read_Bytes; data_cycle++) {
                    while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
                    UCxnTXBUF = 0xff;                       // Write dummy byte
                	while (!(UCxnIFG & UCRXIFG)) ;          // Wait for RX buffer (full)
                	*(data_buffer + data_cycle) = UCxnRXBUF;// Change UCxnRXBUF as required by mode
                }
                break;
            }
        }
    }

    // Deselect the device
	SET_CS;

    __bis_SR_register(gie);                     // Restore original GIE state
    return(status);
}


// ***************************************************************************
//  FLASH_WRITE - HAL write function
//
//  input : device_num               device number to which operation will be done
//          command                  write a single command byte to flash
//          sys_addr                 system address to be used
//          data_buffer              Pointer to the data buffer where to store the written data
//          Number_Of_Written_Bytes  number of bytes to be written
//
//  return value : status of the operation - FAIL or SUCCESS
// ***************************************************************************
SLLD_STATUS FLASH_WRITE
(
BYTE     command,                        /* write a single command byte to flash */
ADDRESS  sys_addr,                       /* system address to be used */
BYTE    *data_buffer,                    /* Pointer to the data buffer containing data to be written */
int      Number_Of_Written_Bytes         /* number of bytes to be written */
)
{
    SLLD_STATUS status = SLLD_OK;
    int data_cycle;
    int i;

    // Select the device
	CHECK_BSY;
	CLEAR_CS;

	WORD gie = __get_SR_register() & GIE;   // Store current GIE state

    __disable_interrupt();                      // Make this operation atomic
    
    while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
    // Write the command
	UCxnTXBUF = command;

    // Write the address
    if (sys_addr != ADDRESS_NOT_USED)
    {
    	for (i=16; i>=0; i-=8) {
    		 while (!(UCxnIFG & UCTXIFG)) ;          				// Wait while not ready for TX
    		 UCxnTXBUF = (BYTE)((sys_addr >> i) & 0x000000FF);		// Change UCxnTXBUF as required by mode
    	}
    }

    // Write the data
    if (Number_Of_Written_Bytes != 0)
    {
        switch (command)
        {
            case SPI_QPP_CMD:
            case SPI_QPP_4B_CMD:
            {
                // Write the data using the relevant mode
                for (data_cycle = 0; data_cycle < Number_Of_Written_Bytes; data_cycle++) {
                    while (!(UCxnIFG & UCTXIFG)) ;          			// Wait while not ready for TX
                	UCxnTXBUF = *(data_buffer + data_cycle);			// Change UCxnTXBUF as required by mode
                }
                break;
            }
            default:
            {
                // Write the data using the relevant mode
                for (data_cycle = 0; data_cycle < Number_Of_Written_Bytes; data_cycle++) {
                    while (!(UCxnIFG & UCTXIFG)) ;          			// Wait while not ready for TX
                	UCxnTXBUF = *(data_buffer + data_cycle);			// Change UCxnTXBUF as required by mode
                }
                break;
            }
        }
    }

    while (UCxnSTAT & UCBUSY) ;                 // Wait for all TX/RX to finish

    UCxnRXBUF;                                  // Dummy read to empty RX buffer
                                                // and clear any overrun conditions
    // Deselect the device
	SET_CS;

    __bis_SR_register(gie);                     // Restore original GIE state

    return(status);
}


/*****************************************************************************/
