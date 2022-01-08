

#include "app_flash.h"
#include "stm32f4xx_flash.h"


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
	{
		sector = FLASH_Sector_0;
	}
	else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
	{
		sector = FLASH_Sector_1;
	}
	else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
	{
		sector = FLASH_Sector_2;
	}
	else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
	{
		sector = FLASH_Sector_3;
	}
	else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
	{
		sector = FLASH_Sector_4;
	}
	else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
	{
		sector = FLASH_Sector_5;
	}
	else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
	{
		sector = FLASH_Sector_6;
	}
	else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
	{
		sector = FLASH_Sector_7;
	}
	else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
	{
		sector = FLASH_Sector_8;
	}
	else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
	{
		sector = FLASH_Sector_9;
	}
	else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
	{
		sector = FLASH_Sector_10;
	}

#if defined (USE_STM324xG_EVAL)
	else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
	{
	sector = FLASH_Sector_11;
	}

#else /* USE_STM324x7I_EVAL or USE_STM324x9I_EVAL */

#endif /* USE_STM324x7I_EVAL or USE_STM324x9I_EVAL */
	return sector;
}



uint32_t os_get_sector_size(uint32_t sector)
{
	uint32_t sector_size = 0;
	switch(sector){
		case FLASH_Sector_0:
			sector_size = OS_FLASH_SECTOR_SIZE_16K;
			break;
		case FLASH_Sector_1:
			sector_size = OS_FLASH_SECTOR_SIZE_16K;
			break;
		case FLASH_Sector_2:
			sector_size = OS_FLASH_SECTOR_SIZE_16K;
			break;
		case FLASH_Sector_3:
			sector_size = OS_FLASH_SECTOR_SIZE_16K;
			break;
		case FLASH_Sector_4:
			sector_size = OS_FLASH_SECTOR_SIZE_64K;
			break;
		case FLASH_Sector_5:
			sector_size = OS_FLASH_SECTOR_SIZE_128K;
			break;
		case FLASH_Sector_6:
			sector_size = OS_FLASH_SECTOR_SIZE_128K;
			break;
		case FLASH_Sector_7:
			sector_size = OS_FLASH_SECTOR_SIZE_128K;
			break;
		case FLASH_Sector_8:
			sector_size = OS_FLASH_SECTOR_SIZE_128K;
			break;
		case FLASH_Sector_9:
			sector_size = OS_FLASH_SECTOR_SIZE_128K;
			break;
		case FLASH_Sector_10:
			sector_size = OS_FLASH_SECTOR_SIZE_128K;
			break;
		case FLASH_Sector_11:
			sector_size = OS_FLASH_SECTOR_SIZE_128K;
			break;

		default:
			break;
	}

	return sector_size;
}



/* NOTE: This mcu erase only one page, please check in referent manual for erase exactly address */
bool os_simple_erase_flash(uint32_t address)
{
	uint32_t m_sector = GetSector(address);
	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
				  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

	if (FLASH_EraseSector(m_sector, VoltageRange_3) != FLASH_COMPLETE){
		FLASH_Lock();
		return false;
	}

	FLASH_Lock();
	return true;
}


bool os_simple_write_flash(uint32_t address, uint8_t data[], uint16_t length)
{

	if(!data) return false;
	uint32_t m_sector = GetSector(address);

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
				  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

	if (FLASH_EraseSector(m_sector, VoltageRange_3) != FLASH_COMPLETE){
		FLASH_Lock();
		return false;
	}

	for(uint16_t i = 0; i < length; i++){

		if (FLASH_ProgramByte(address, data[i]) == FLASH_COMPLETE){
			address++;
		} else {
			FLASH_Lock();
			return false;
		}
	}

	FLASH_Lock();

	return true;
}



bool os_write_flash_sensitive(uint32_t address, uint8_t data[], uint16_t length)
{

	if(!data) return false;

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
				  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

	for(uint16_t i = 0; i < length; i++){

		if (FLASH_ProgramByte(address, data[i]) == FLASH_COMPLETE){
			address++;
		} else {
			FLASH_Unlock();
			return false;
		};
	}

	FLASH_Lock();

	return true;
}



bool os_read_flash(uint32_t address, uint8_t data[], uint16_t length)
{
	if(!data) return false;
	for(uint16_t i = 0; i < length; i++)
	{
		data[i] = *((uint8_t *)address);
		address++;
	}

	return true;
}

