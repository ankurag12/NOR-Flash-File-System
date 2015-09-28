/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "../N25Q.h"
#include "../Serialize.h"
#include "ffconf.h"

/* Definitions of physical drive number for each drive */
#define ATA		0	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */

static DSTATUS stat = (STA_NOINIT|STA_NODISK);
FLASH_DEVICE_OBJECT flashDeviceObject;			// STATIC OR NOT? THAT IS THE QUESTION

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{

	msp430_spi_init();		// Initialize SPI Communication (See Serialize.c)

	if (Driver_Init(&flashDeviceObject)==Flash_Success)		/* initialize the flash driver */
	{
		stat &= ~(STA_NOINIT|STA_NODISK);

		NMX_uint8 ucSR;
		flashDeviceObject.GenOp.ReadStatusRegister(&ucSR);
		//if(ucSR & SPI_FLASH_WEL)
		//	stat &= ~STA_PROTECT;
		return stat;
	}

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	//DRESULT res;
	DSTATUS s;

	s = disk_status(pdrv);
    if (s & STA_NOINIT)
        return ( RES_NOTRDY) ;

    if (!count)
        return ( RES_PARERR) ;

	ParameterType para;

	para.Read.udAddr = sector*_MAX_SS;
	para.Read.pArray = buff;
	para.Read.udNrOfElementsToRead = count*_MAX_SS;
	if(flashDeviceObject.GenOp.DataRead(Read, &para)==Flash_Success)
		return RES_OK;

	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	//DRESULT res;
    DSTATUS s;
    UINT k=0, i=0;


    s = disk_status(pdrv);
    if (s & STA_NOINIT)
        return ( RES_NOTRDY) ;

    if (s & STA_PROTECT)
        return ( RES_WRPRT) ;

    if (!count)
        return ( RES_PARERR) ;


    ParameterType para;
	while(count>0) {
		if(flashDeviceObject.GenOp.SubSectorErase(sector+k)!=Flash_Success)
			return RES_ERROR;
		for(i=0; i<PAGES_PER_SUBSECTOR; i++) {
			para.PageProgram.udAddr = sector*_MAX_SS + i*BYTES_PER_PAGE;
			para.PageProgram.pArray = (void*)(buff+i*BYTES_PER_PAGE);
			para.PageProgram.udNrOfElementsInArray = BYTES_PER_PAGE;
			if(flashDeviceObject.GenOp.DataProgram(PageProgram, &para)!=Flash_Success)
				return RES_ERROR;
		}
		count--;
		k++;
	}

	return (count ? RES_ERROR : RES_OK);
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{

	switch(cmd) {
	case CTRL_SYNC:
		break;
	case GET_SECTOR_COUNT:
		*((DWORD*)buff) = (DWORD)(4096);
		break;
	case GET_SECTOR_SIZE:
		*((WORD*)buff) = (WORD)(4096);
		break;
	case GET_BLOCK_SIZE:
		*((DWORD*)buff) = (DWORD)(4096);
		break;
	case CTRL_TRIM:
		break;
	default:
		break;

	}

	return RES_OK;
}
#endif
