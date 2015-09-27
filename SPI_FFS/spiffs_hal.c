#include "spiffs_hal.h"
#include <stdio.h>
#include <string.h>
#include "../N25Q.h"
#include "spiffs.h"

#define LOG_PAGE_SIZE       256
#define PHYSICAL_SECTOR_SIZE 65536
#define FLASH_PHYSICAL_SIZE 1048576L	//1*1024*1024

static spiffs fs;
static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*4];

static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst) {
	ParameterType para;
	ReturnType ret;
	para.Read.udAddr = addr;
	para.Read.pArray = dst;
	para.Read.udNrOfElementsToRead = size;
	if(flashDeviceObject.GenOp.DataRead(Read, &para)==Flash_Success)
		return SPIFFS_OK;
	return -1;
}

static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src) {
	ParameterType para;
	ReturnType ret;
	para.PageProgram.udAddr = addr;
	para.PageProgram.pArray = src;
	para.PageProgram.udNrOfElementsInArray = size;
	if(flashDeviceObject.GenOp.DataProgram(PageProgram, &para)==Flash_Success)
		return SPIFFS_OK;
	return -1;
}

static s32_t my_spiffs_erase(u32_t addr, u32_t size) {
	while(size>0) {
		if(flashDeviceObject.GenOp.SectorErase(addr)!=Flash_Success)
			return -1;
		addr=addr+PHYSICAL_SECTOR_SIZE;
		size-=PHYSICAL_SECTOR_SIZE;
	}

	return SPIFFS_OK;
}

void my_spiffs_mount() {
  spiffs_config cfg;
  cfg.phys_size = FLASH_PHYSICAL_SIZE; // use all spi flash
  cfg.phys_addr = 0; // start spiffs at start of spi flash
  cfg.phys_erase_block = PHYSICAL_SECTOR_SIZE; // according to datasheet
  cfg.log_block_size = PHYSICAL_SECTOR_SIZE; // let us not complicate things
  cfg.log_page_size = LOG_PAGE_SIZE; // as we said

  cfg.hal_read_f = my_spiffs_read;
  cfg.hal_write_f = my_spiffs_write;
  cfg.hal_erase_f = my_spiffs_erase;

  int res = SPIFFS_mount(&fs,
    &cfg,
    spiffs_work_buf,
    spiffs_fds,
    sizeof(spiffs_fds),
    spiffs_cache_buf,
    sizeof(spiffs_cache_buf),
    0);
    printf("mount res: %i \n", res);

}

void test_spiffs() {
  char buf[12];

  my_spiffs_mount();

  spiffs_file fd = SPIFFS_open(&fs, "my_file", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
  if (SPIFFS_write(&fs, fd, (u8_t *)"Hello world", 12) < 0)
	  printf("errno %i\n", SPIFFS_errno(&fs));
  SPIFFS_close(&fs, fd);

  fd = SPIFFS_open(&fs, "my_file", SPIFFS_RDWR, 0);
  if (SPIFFS_read(&fs, fd, (u8_t *)buf, 12) < 0)
	  printf("errno %i\n", SPIFFS_errno(&fs));
  SPIFFS_close(&fs, fd);

  printf("--> %s <--\n", buf);
}


