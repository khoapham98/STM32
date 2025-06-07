#include "main.h"
#include <string.h>

#define USART1_BASE_ADDR 0x40011000
#define GPIOB_BASE_ADDR 0x40020400


void USART_Init();
void USART_send_char(char data);
void USART_send_string(char* str);


char str[20] = {0};
int _index = 0;

void USART1_IRQHandler()
{
	uint32_t* USART1_DR = (uint32_t*) (USART1_BASE_ADDR + 0x04);
	str[_index++] = *USART1_DR;
}

int main()
{
	HAL_Init();
	USART_Init();

	while(1)
	{

	}

	return 0;
}


void USART_send_char(char data)
{
	uint32_t* USART1_DR = (uint32_t*) (USART1_BASE_ADDR + 0x04);
	*USART1_DR = data;	// write data to DR register

	uint32_t* USART1_SR = (uint32_t*) (USART1_BASE_ADDR + 0x00);
	while (((*USART1_SR >> 7) & 1) == 0);	// wait until the data has been transferred
}

void USART_send_string(char* str)
{
	int size = strlen(str);
	for (int i = 0; i < size; i++)
	{
		USART_send_char(str[i]);
	}
}

void USART_Init()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	uint32_t* GPIOB_MODER = (uint32_t*) (GPIOB_BASE_ADDR + 0x00);
	uint32_t* GPIOB_AFRL = (uint32_t*) (GPIOB_BASE_ADDR + 0x20);
	*GPIOB_MODER &= ~(0b1111 << 12); // clear bit pin PB6 and PB7
	*GPIOB_MODER |= (0b1010 << 12);  // set PB6 and PB7 in AF mode
	*GPIOB_AFRL &= ~(0xff << 24);	// clear bit pin AFRL6 and AFRL7
	*GPIOB_AFRL |= (0b0111 << 24) | (0b0111 << 28); // select AF7 for AFRL6 and AFRL7

	__HAL_RCC_USART1_CLK_ENABLE();
	uint32_t* USART1_CR1 = (uint32_t*) (USART1_BASE_ADDR + 0x0C);
	uint32_t* USART1_BRR = (uint32_t*) (USART1_BASE_ADDR + 0x08);
	*USART1_CR1 |= (0b1 << 12); // set word length
	*USART1_CR1 |= (0b1 << 10); // enable parity bit
	*USART1_CR1 &= ~(0b1 << 9); // select Even parity
	*USART1_CR1 |= (0b11 << 2); // enable transmitter & receiver
	*USART1_CR1 |= (0b1 << 13); // enable USART1
	*USART1_BRR = (104 << 4) | (3 << 0); // set baud rate at 9600bps
	*USART1_CR1 |= (0b1 << 5); // generate interrupt

	uint32_t* NVIC_ISER1 = (uint32_t*) 0xE000E104;
	*NVIC_ISER1 |= (0b1 << 5);
}
