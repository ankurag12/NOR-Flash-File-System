/*
 * spiffs_hal.h
 *
 *  Created on: Sep 27, 2015
 *      Author: Robo
 */

#ifndef SPI_FFS_SPIFFS_HAL_H_
#define SPI_FFS_SPIFFS_HAL_H_

#include "spiffs_config.h"
#include "../N25Q.h"

extern FLASH_DEVICE_OBJECT flashDeviceObject;

static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst);
static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src);
static s32_t my_spiffs_erase(u32_t addr, u32_t size);
void my_spiffs_mount();
void test_spiffs();

#endif /* SPI_FFS_SPIFFS_HAL_H_ */
