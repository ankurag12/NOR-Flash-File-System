//

#include <msp430.h>
#include <stdio.h>
#include <string.h>

//#include "SPI_FFS/spiffs.h"
//#include "SPI_FFS/spiffs_hal.h"
//#include "FAT_FS/ff.h"
#include "FFIS/FlashFileIndexSystem.h"
#include "FFIS/FFIS_HAL.h"
#include "msp430/msp430_uart.h"
#include "msp430/msp430_init.h"

//#include "FAT_FS/diskio.h"
//FATFS FatFs;

static FlashHW flashHWobj;


int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	msp430_init_hardware();

	printf("Test \r\n");

	msp430_init_uart(BR_9600);

	msp430_uart_puts("\r \n Hello World \r \n");

	if(!FFIS_init(&flashHWobj))
		printf("FFIS Initialized. \r\n");
	else
		printf("Error initializing FFIS \r\n");


	int bw, br;
	fileIndexEntry newEntry;
	FFISretVal ret;


	FILE *inFile;
	long lSize;
	int res;

	uint8_t buffer[512];

	// Enter desired (source) filename here
	inFile = fopen("waveform.wbf", "rb");

	// obtain file size:
	fseek (inFile , 0 , SEEK_END);
	lSize = ftell (inFile);
	rewind (inFile);

	//Enter desired (destination) file ID here
	if(ret = fileCheckOut(&flashHWobj, 3, &newEntry, WRITE))
		printf("Error (%d) in checking out file in WRITE mode \r\n", ret);

	while(lSize>0)
	{
		res = fread (buffer, sizeof(uint8_t), sizeof(buffer), inFile);
		if(ret = fileWrite(&flashHWobj, &newEntry, buffer, res, &bw))
			printf("Error (%d) in writing file \r\n", ret);
		lSize -= res;
	}

	fclose(inFile);
	if(ret = fileCheckIn(&flashHWobj, &newEntry))
		printf("Error (%d) in checking in file \r\n", ret);

	printf("Flash file write complete \r\n");

// Uncomment this to verify if the file was written correctly

	FILE *outFile;

	// Enter verification (destination) filename here
	outFile = fopen("flash.bin", "wb");
	fileIndexEntry newEntry2;

	//Enter desired (source) file ID here
	if(ret = fileCheckOut(&flashHWobj, 3, &newEntry2, READ))
		printf("Error (%d) in checking out file in READ mode \r\n", ret);

	lSize = newEntry2.fileSize;

	while(lSize>0)
	{
		if(ret = fileRead(&flashHWobj, &newEntry2, buffer, sizeof(buffer), &br)) {
			printf("Error (%d) in reading file \r\n", ret);
			return -1;
		}
		fwrite(buffer, sizeof(uint8_t), br, outFile);
		lSize -= br;

	}

	fclose(outFile);
	if(ret = fileCheckIn(&flashHWobj, &newEntry2))
		printf("Error (%d) in checking in file \r\n", ret);

	printf("Verification file write complete \r\n");

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

