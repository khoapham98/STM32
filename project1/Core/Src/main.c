#include "main.h"

#define GPIOA_BASE_ADDR 0x40020000
#define SPI1_BASE_ADDR 0x40013000

void SPI_Init();

int main()
{
	while (1)
	{

	}

	return 0;
}

/*	== SPI1 ==
	- PA5: SCK
	- PA6: MISO
	- PA7: MOSI
	- PA8: SS
 */
void SPI_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE_ADDR + 0x00);
	uint32_t* GPIOA_AFRL = (uint32_t*) (GPIOA_BASE_ADDR + 0x20);
	*GPIOA_MODER &= ~(0xff << 10);	// clear bit
	*GPIOA_MODER |= (0b10 << 10) | (0b10 << 12) | (0b10 << 14);
	*GPIOA_MODER |= (0b01 << 16);
	*GPIOA_AFRL &= ~(0xfff << 20);	// clear bit
	*GPIOA_AFRL |= (5 << 20) | (5 << 24) | (5 << 28);	// select AF05

	__HAL_RCC_SPI1_CLK_ENABLE();
	uint32_t* SPI_CR1 = (uint32_t*) (SPI1_BASE_ADDR + 0x00);
	*SPI_CR1 |= 1 << 2;	// master mode
	*SPI_CR1 &= ~(0b111 << 3);	// clear bit
	*SPI_CR1 |= (0b011 << 3);	// configure Clock = 1MHz
	*SPI_CR1 |= 1 << 9;	// Software slave management enabled
	*SPI_CR1 |= 1 << 8;	// Internal slave select
	*SPI_CR1 |= 1 << 6;	// SPI enable
}
