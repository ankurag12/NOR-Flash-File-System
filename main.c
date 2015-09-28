//

#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include "Serialize.h"
#include "N25Q.h"
#include "SPI_FFS/spiffs.h"
#include "SPI_FFS/spiffs_hal.h"
#include "FAT_FS/ff.h"

//#include "FAT_FS/diskio.h"

#define USCI_UNIT	A
#define	USCI_CHAN	1
// Pin definitions for this unit.
#define	UART_TX                 MSP430_GPIO(5,6)
#define	UART_RX                 MSP430_GPIO(5,7)

//FLASH_DEVICE_OBJECT flashDeviceObject;

FATFS FatFs;


int msp430_uart_putc(int c)
{
	while(!(UCA1IFG & UCTXIFG));
	UCA1TXBUF = (unsigned char)c;

	return (unsigned char)c;
}


int msp430_uart_puts(const char *s)
{
	unsigned int i;

	for(i = 0; s[i]; i++)
		msp430_uart_putc(s[i]);

	return i;
}

void msp430_init_uart()
{
	P5SEL = 0xC0;                             // P3.4,5 = USCI_A2 TXD/RXD
	UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA1CTL1 |= UCSSEL_1;                     // CLK = ACLK
	UCA1BR0 = 0x03;                              // 32kHz/9600=3.41 (see User's Guide)
	UCA1BR1 = 0x00;                              //
	UCA1MCTL |= UCBRS_3 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA1IE |= UCRXIE;                         // Enable USCI_A2 RX interrupt

	__enable_interrupt();       // interrupts enabled
}



int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	printf("Test \r\n");



	ParameterType para;
	ReturnType ret;

	NMX_uint8 rbuffer[17]; /* read buffer */
	memset(rbuffer,0,17);
	NMX_uint8 wbuffer[17] = "Does this work? \0"; /* write buffer */

	/*ret = Driver_Init(&flashDeviceObject);  initialize the flash driver
	if (ret == Flash_WrongType)
	{
		msp430_uart_puts("Sorry, no device detected.\n");
		return -1;
	}

	if(flashDeviceObject.GenOp.SectorErase(0)!=Flash_Success)
		msp430_uart_puts("Error erasing sector\r\n");

	para.PageProgram.udAddr = 0;
	para.PageProgram.pArray = wbuffer;
	para.PageProgram.udNrOfElementsInArray = sizeof(wbuffer);
	if(flashDeviceObject.GenOp.DataProgram(PageProgram, &para)!=Flash_Success)
		printf("Error writing to Flash");

	para.Read.udAddr = 0;
	para.Read.pArray = rbuffer;
	para.Read.udNrOfElementsToRead = sizeof(rbuffer)-1;
	if(flashDeviceObject.GenOp.DataRead(Read, &para)!=Flash_Success)
		printf("Error reading Flash");
	rbuffer[16]=0;
	printf("Reading from Flash: %s \r\n", rbuffer);*/

	//test_spiffs();


	msp430_init_uart();
	msp430_uart_puts("\r \n Hello World \r \n");

/*
	BYTE buff[100];
	printf("Disk Status: %d \r\n",disk_initialize(0));

	printf("Disk write status: %d \r\n", disk_write(0, wbuffer, 0, 1));

	printf("Disk read status: %d \r\n", disk_read(0, buff, 0, 1));

	printf("Data on the disk: %s \r\n", buff);
	while(1);

*/



	FIL fil;
	FRESULT fr;
	UINT br, bw;

	if(fr = f_mount(&FatFs, "", 1))
		printf("Error in mounting volume: %d \r\n", fr);

	if(fr = f_mkfs("", 1, 0))
		printf("Error in creating the FAT volume: %d \r\n", fr);

	if(fr = f_open(&fil, "test.txt", FA_WRITE | FA_CREATE_ALWAYS))
		printf("Some error in creating file: %d \r\n", fr);

	if(fr = f_write(&fil, wbuffer, 15, &bw))
		printf("Some error in writing: %d \r\n", fr);

	f_close(&fil);

	if(fr = f_open(&fil, "test.txt", FA_READ))
		printf("Some error in opening file: %d \r\n", fr);

	if(fr = f_read(&fil, rbuffer, 15, &br))
		printf("Some error in reading file: %d  \r\n", fr);

	f_close(&fil);

	printf("Data read : %s \r\n", rbuffer);

	f_mount(0, "", 0);


/*
	char str[100];
	memset(str,0,sizeof(str));

	rbuffer[16]=0;
	sprintf(str, "Reading from flash: %s \r\n", rbuffer);
	msp430_uart_puts(str);
*/

	__bis_SR_register(LPM3_bits + GIE);       // Enter LPM0, interrupts enabled


  return 0;
}

// Echo back RXed character, confirm TX buffer is ready first
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA1IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
    while (!(UCA1IFG&UCTXIFG));             // USCI_A1 TX buffer ready?
    UCA1TXBUF = UCA1RXBUF;                  // TX -> RXed character
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}

