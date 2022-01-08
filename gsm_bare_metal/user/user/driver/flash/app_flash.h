/*
 * os_flash.h
 *
 *  Created on: Jul 12, 2020
 *      Author:
 */

#ifndef DRIVER_OS_FLASH_H_
#define DRIVER_OS_FLASH_H_

#include <stdint.h>
#include <stdbool.h>

#define OS_FLASH_SECTOR_SIZE_16K			(16*1024)
#define OS_FLASH_SECTOR_SIZE_64K			(64*1024)
#define OS_FLASH_SECTOR_SIZE_128K			(128*1024)

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes   */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 Kbytes */

/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address);

uint32_t os_get_sector_size(uint32_t sector);

/* NOTE: This mcu erase only one page, please check in referent manual for erase exactly address */
bool os_simple_erase_flash(uint32_t address);

bool os_simple_write_flash(uint32_t address, uint8_t data[], uint16_t length);

bool os_write_flash_sensitive(uint32_t address, uint8_t data[], uint16_t length);

bool os_read_flash(uint32_t address, uint8_t data[], uint16_t length);

#endif /* DRIVER_OS_FLASH_H_ */
