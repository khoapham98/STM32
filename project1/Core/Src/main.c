#include "main.h"
#define FLASH_BASE_ADDR 0x40023C00
void flash_erase_sector(int sec_num);
void flash_program(uint8_t* addr, uint8_t val);

int main()
{


	return 0;
}

void flash_erase_sector(int sec_num)
{
	uint32_t* FLASH_SR = (uint32_t*) (FLASH_BASE_ADDR + 0x0C);
	while (((*FLASH_SR >> 16) & 1) == 1);	// check the BSY bit

	uint32_t* FLASH_CR = (uint32_t*) (FLASH_BASE_ADDR + 0x10);
	*FLASH_CR |= (1 << 1);	// set the SER bit
	*FLASH_CR |= (sec_num << 3);	// select the section as sec_num
	*FLASH_CR |= (1 << 16)	// set the STRT bit
	while (((*FLASH_SR >> 16) & 1) == 1);	// wait for BSY bit to be cleared

	uint32_t* FLASH_KEYR = (uint32_t*) (FLASH_BASE + 0x04);
}

void flash_program(uint8_t* addr, uint8_t val)
{
	uint32_t* FLASH_SR = (uint32_t*) (FLASH_BASE_ADDR + 0x0C);
	while (((*FLASH_SR >> 16) & 1) == 1);	// check the BSY bit

	uint32_t* FLASH_CR = (uint32_t*) (FLASH_BASE_ADDR + 0x10);
	*FLASH_CR |= (1 << 0);	// set the PG bit
	*addr = val;
	while (((*FLASH_SR >> 16) & 1) == 1);	// wait for BSY bit to be cleared
}
