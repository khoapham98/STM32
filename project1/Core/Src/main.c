#include "main.h"

#define GPIOE_BASE_ADDR 0x40021000
#define GPIOA_BASE_ADDR 0x40020000
#define SPI1_BASE_ADDR 0x40013000

void SPI_Init();
void master_recv(uint16_t data);
volatile uint16_t tmp = 0;

int main()
{
	HAL_Init();
	SPI_Init();

	while (1)
	{
		master_recv(0x8f);
		HAL_Delay(1000);
	}

	return 0;
}

void master_recv(uint16_t data)
{
	uint32_t* GPIOE_ODR = (uint32_t*) (GPIOE_BASE_ADDR + 0x14);
	uint16_t* SPI_DR = (uint16_t*) (SPI1_BASE_ADDR + 0x0C);
	uint16_t* SPI_SR = (uint16_t*) (SPI1_BASE_ADDR + 0x08);

	/* select slave */
	*GPIOE_ODR &= ~(1 << 3);

	/* wait until the TX buffer is empty*/
	while (((*SPI_SR >> 1) & 1) == 0);

	/* write data into DR register */
	*SPI_DR = data;

	/* wait until the data has been transmitted */
	while (((*SPI_SR >> 7) & 1) == 1);

	/* read dummy data to clear the RX buffer */
	tmp = *SPI_DR;

	/* wait until the TX buffer is empty*/
	while (((*SPI_SR >> 1) & 1) == 0);

	/* write dummy data into DR register */
	*SPI_DR = 0x00;

	/* wait until the data has been transmitted */
	while (((*SPI_SR >> 7) & 1) == 1);

	/* wait until the RX buffer is not empty */
	while ((*SPI_SR & 1) == 0);

	/* read data that is sent by slave */
	tmp = *SPI_DR;

	/* un-active slave */
	*GPIOE_ODR |= 1 << 3;
}

/*	== SPI1 ==
	- PA5: SCK
	- PA6: MISO
	- PA7: MOSI
	- PE3: SS
 */
void SPI_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	uint32_t* GPIOE_MODER = (uint32_t*) (GPIOE_BASE_ADDR + 0x00);
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE_ADDR + 0x00);
	uint32_t* GPIOA_AFRL = (uint32_t*) (GPIOA_BASE_ADDR + 0x20);
	*GPIOA_MODER &= ~(0xff << 10);	// clear bit
	*GPIOA_MODER |= (0b10 << 10) | (0b10 << 12) | (0b10 << 14);	// set PA5, PA6, PA7 at AF mode
	*GPIOE_MODER |= (0b01 << 6);	// set PE3 at OUTPUT mode
	*GPIOA_AFRL &= ~(0xfff << 20);	// clear bit
	*GPIOA_AFRL |= (5 << 20) | (5 << 24) | (5 << 28);	// select AF05

	__HAL_RCC_SPI1_CLK_ENABLE();
	uint16_t* SPI_CR1 = (uint16_t*) (SPI1_BASE_ADDR + 0x00);
	*SPI_CR1 |= 1 << 2;	// master mode
	*SPI_CR1 &= ~(0b111 << 3);	// clear bit
	*SPI_CR1 |= (0b011 << 3);	// configure Clock = 1MHz
	*SPI_CR1 |= 1 << 9;	// Software slave management enabled
	*SPI_CR1 |= 1 << 8;	// Internal slave select
	*SPI_CR1 |= 1 << 6;	// SPI enable
}
