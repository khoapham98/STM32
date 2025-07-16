#include "main.h"

#define I2C1_BASE_ADDR 0x40005400
#define GPIOB_BASE_ADDR 0x40020400
#define ACCEL_ADDR 0x19
#define MAGNE_ADDR 0x1E
typedef enum
{
	ACK, NACK
} check_t;
uint8_t master_read(uint8_t slave_addr, uint8_t reg_addr);
uint8_t master_receive(uint8_t slave_addr, uint8_t reg_addr);
uint8_t master_transfer(uint8_t slave_addr, uint8_t reg_addr, uint8_t data);
void I2C_Init();

uint8_t data = 0;
int main()
{
	HAL_Init();
	I2C_Init();

	while (1)
	{
		data = master_read(ACCEL_ADDR, 0x20);
		data = master_read(MAGNE_ADDR, 0x60);
	}
	return 0;
}

uint8_t master_read(uint8_t slave_addr, uint8_t reg_addr)
{
	uint16_t* I2C_CR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x00);
	uint16_t* I2C_DR  = (uint16_t*) (I2C1_BASE_ADDR + 0x10);
	uint16_t* I2C_SR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x14);
	uint16_t* I2C_SR2 = (uint16_t*) (I2C1_BASE_ADDR + 0x18);

	/* wait until bus is free */
//	while (((*I2C_SR2 >> 1) & 1) == 1);

	/* send start bit */
	*I2C_CR1 |= 1 << 8;

	/* wait for start is generated */
	while ((*I2C_SR1 & 1) == 0);

	/* send slave address + write mode */
	*I2C_DR  = (slave_addr << 1);

	/* wait for slave address is transmitted */
	while (((*I2C_SR1 >> 1) & 1) == 0);
	volatile uint16_t tmp = *I2C_SR2;

	/* check ACK */
	if (((*I2C_SR1 >> 10) & 1) == 1) { return 1; }

	/* send register address */
	*I2C_DR = reg_addr;

	/* wait for register address is transmitted */
	while (((*I2C_SR1 >> 7) & 1) == 0);

	/* check ACK */
	if (((*I2C_SR1 >> 10) & 1) == 1) { return 1; }

	/* send start bit */
	*I2C_CR1 |= 1 << 8;

	/* wait for start is generated */
	while ((*I2C_SR1 & 1) == 0);

	/* send slave address + read mode */
	*I2C_DR = (slave_addr << 1) | 0x01;

	/* wait for slave address is transmitted */
	while (((*I2C_SR1 >> 1) & 1) == 0);
	tmp = *I2C_SR2;

	/* check ACK */
	if (((*I2C_SR1 >> 10) & 1) == 1) { return 1; }

	/* read data */
	volatile uint8_t data = *I2C_DR;	// Breakpoint here

	/* send stop bit */
	*I2C_CR1 |= 1 << 9;

	return data;
}

uint8_t master_receive(uint8_t slave_addr, uint8_t reg_addr)
{
	uint16_t* I2C_CR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x00);
	uint16_t* I2C_DR  = (uint16_t*) (I2C1_BASE_ADDR + 0x10);
	uint16_t* I2C_SR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x14);
	uint16_t* I2C_SR2 = (uint16_t*) (I2C1_BASE_ADDR + 0x18);
	/* send start */
	*I2C_CR1 |= 1 << 8;
	while ((*I2C_SR1 & 1) == 0);
	/* send slave address + write bit */
	*I2C_DR = slave_addr << 1;
	while (((*I2C_SR1 >> 1) & 1) == 0);
	uint16_t tmp = *I2C_SR2;
	if (((*I2C_SR1 >> 10) & 1) == 1) return NACK;
	/* send register address */
	*I2C_DR = reg_addr;
	while (((*I2C_SR1 >> 7) & 1) == 0);
	if (((*I2C_SR1 >> 10) & 1) == 1) return NACK;
	/* send start */
	*I2C_CR1 |= 1 << 8;
	while ((*I2C_SR1 & 1) == 0);
	/* send slave address + read bit */
	*I2C_DR = (slave_addr << 1) | 0x01;
	while (((*I2C_SR1 >> 1) & 1) == 0);
	tmp = *I2C_SR2;
	if (((*I2C_SR1 >> 10) & 1) == 1) return NACK;
	/* read data */
	uint8_t data = *I2C_DR;
	/* send stop */
	*I2C_CR1 |= 1 << 9;

	return data;
}

uint8_t master_transfer(uint8_t slave_addr, uint8_t reg_addr, uint8_t data)
{
	uint16_t* I2C_CR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x00);
	uint16_t* I2C_DR  = (uint16_t*) (I2C1_BASE_ADDR + 0x10);
	uint16_t* I2C_SR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x14);
	uint16_t* I2C_SR2 = (uint16_t*) (I2C1_BASE_ADDR + 0x18);
	/* send start */
	*I2C_CR1 |= 1 << 8;
	while ((*I2C_SR1 & 1) == 0);
	/* send slave address + write bit */
	*I2C_DR = slave_addr << 1;
	while (((*I2C_SR1 >> 1) & 1) == 0);
	uint8_t tmp = *I2C_SR2;
	if (((*I2C_SR1 >> 10) & 1) == 1) return NACK;
	/* send register address */
	*I2C_DR = reg_addr;
	while (((*I2C_SR1 >> 7) & 1) == 0);
	if (((*I2C_SR1 >> 10) & 1) == 1) return NACK;
	/* send data */
	*I2C_DR = data;
	while (((*I2C_SR1 >> 7) & 1) == 0);
	if (((*I2C_SR1 >> 10) & 1) == 1) return NACK;
	/* send stop */
	*I2C_CR1 |= 1 << 9;
	return ACK;
}

/* I2C1
 * PB6: SCL
 * PB9: SDA
 */
void I2C_Init()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	uint32_t* GPIOB_MODER = (uint32_t*) (GPIOB_BASE_ADDR + 0x00);
	uint32_t* GPIOB_AFRL  = (uint32_t*) (GPIOB_BASE_ADDR + 0x20);
	uint32_t* GPIOB_AFRH  = (uint32_t*) (GPIOB_BASE_ADDR + 0x24);

	*GPIOB_MODER &= ~((0b11 << (6 * 2)) | (0b11 << (9 * 2)));
	*GPIOB_MODER |= (0b10 << (6 * 2)) | (0b10 << (9 * 2));

	*GPIOB_AFRL &= ~(0xf << (6 * 4));
	*GPIOB_AFRL |= 4 << (6 * 4);
	*GPIOB_AFRH &= ~(0xf << 4);
	*GPIOB_AFRH |= 4 << 4;

	__HAL_RCC_I2C1_CLK_ENABLE();
	uint16_t* I2C_CR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x00);
	uint16_t* I2C_CR2 = (uint16_t*) (I2C1_BASE_ADDR + 0x04);
	uint16_t* I2C_CCR = (uint16_t*) (I2C1_BASE_ADDR + 0x1C);
	*I2C_CR2 |= 16 << 0;
	*I2C_CCR = 80;
	*I2C_CR1 |= 1 << 0;
}
