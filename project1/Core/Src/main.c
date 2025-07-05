#include "main.h"

#define WRITE 0x00
#define READ 0x01
#define GPIOB_BASE_ADDR 0x40020400
#define I2C1_BASE_ADDR 0x40005400
#define ACCEL_ADDR 0x19
#define MAGNE_ADDR 0x1E

char master_send(uint8_t slave_addr, uint8_t reg_addr, uint8_t data);
uint8_t master_read(uint8_t slave_addr, uint8_t reg_addr);
void I2C_Init();

int main()
{
	HAL_Init();
	I2C_Init();
	uint8_t x = 10;
	volatile uint8_t data = 0;

	while (1)
	{
		data = master_read(ACCEL_ADDR, 0x2F);
		master_send(ACCEL_ADDR, 0x20, x);
		x++;
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
	while (((*I2C_SR2 >> 1) & 1) == 1);

	/* send start bit */
	*I2C_CR1 |= 1 << 8;

	/* wait for start is generated */
	while ((*I2C_SR1 & 1) == 0);

	/* send slave address + write mode */
	*I2C_DR  = (slave_addr << 1) | WRITE;

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
	*I2C_DR = (slave_addr << 1) | READ;

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

char master_send(uint8_t slave_addr, uint8_t reg_addr, uint8_t data)
{
	uint16_t* I2C_CR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x00);
	uint16_t* I2C_DR  = (uint16_t*) (I2C1_BASE_ADDR + 0x10);
	uint16_t* I2C_SR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x14);
	uint16_t* I2C_SR2 = (uint16_t*) (I2C1_BASE_ADDR + 0x18);

	/* wait until bus is free */
	while (((*I2C_SR2 >> 1) & 1) == 1);

	/* send start bit */
	*I2C_CR1 |= 1 << 8;

	/* wait for start is generated */
	while ((*I2C_SR1 & 1) == 0);

	/* send slave address */
	*I2C_DR = (slave_addr << 1) | WRITE;

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

	/* send data */
	*I2C_DR = data;

	/* wait for data is transmitted */
	while (((*I2C_SR1 >> 7) & 1) == 0);

	/* check ACK */
	if (((*I2C_SR1 >> 10) & 1) == 1) { return 1; }

	/* send stop bit */
	*I2C_CR1 |= 1 << 9;

	return 0;
}

/*
	PB6: SCL
	PB9: SDA
 */
void I2C_Init()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	uint32_t* GPIOB_MODER = (uint32_t*) (GPIOB_BASE_ADDR + 0x00);
	uint32_t* GPIOB_AFRL = (uint32_t*) (GPIOB_BASE_ADDR + 0x20);
	uint32_t* GPIOB_AFRH = (uint32_t*) (GPIOB_BASE_ADDR + 0x24);
	*GPIOB_MODER &= ~((0b11 << (6 * 2)) | (0b11 << (9 * 2)));
	*GPIOB_MODER |= (0b10 << (6 * 2)) | (0b10 << (9 * 2));
	*GPIOB_AFRL &= ~(0xf << 24);
	*GPIOB_AFRL |= 4 << 24;
	*GPIOB_AFRH &= ~(0xf << 4);
	*GPIOB_AFRH |= 4 << 4;

	__HAL_RCC_I2C1_CLK_ENABLE();
	uint16_t* I2C_CR1 = (uint16_t*) (I2C1_BASE_ADDR + 0x00);
	uint16_t* I2C_CR2 = (uint16_t*) (I2C1_BASE_ADDR + 0x04);
	uint16_t* I2C_CCR = (uint16_t*) (I2C1_BASE_ADDR + 0x1C);
	*I2C_CR2 |= 16;
	*I2C_CCR = 80;
	*I2C_CR1 |= 1 << 0;
}

