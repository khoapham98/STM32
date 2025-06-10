#include "main.h"
#include <string.h>

void UART_Init();
void button_Init();
char isPressed();
void USART_send(uint8_t data);
void USART_send_string(char* str);

int main()
{
	HAL_Init();
	UART_Init();
	button_Init();
	int press = 0;
	while (1)
	{
		while (isPressed())
		{
			USART_send_string("The button is being pressed!\n");
			HAL_Delay(500);
			press = 0;
		}
		if (press == 0)
		{
			USART_send_string("Waiting");
			press = 1;
		}
		USART_send('.');
		HAL_Delay(1000);
	}
	return 0;
}

void USART_send_string(char* str)
{
	int size = strlen(str);
	for (int i = 0; i < size; i++) {
		USART_send(str[i]);
	}
}

void USART_send(uint8_t data)
{
	uint32_t* USART6_DR = (uint32_t*) (USART6_BASE + 0x04);
	uint32_t* USART6_SR = (uint32_t*) (USART6_BASE + 0x00);
	*USART6_DR = data;
	while (((*USART6_SR >> 7) & 1) == 0);
}

char isPressed()
{
	uint32_t* GPIOA_IDR = (uint32_t*) (GPIOA_BASE + 0x10);
	if ((*GPIOA_IDR & 1) == 1) { return 1;}
	return 0;
}

void button_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE + 0x00);
	*GPIOA_MODER &= ~(0b11 << 0);
}

void UART_Init()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	uint32_t* GPIOC_MODER = (uint32_t*) (GPIOC_BASE + 0x00);
	uint32_t* GPIOC_AFRL = (uint32_t*) (GPIOC_BASE + 0x20);
	*GPIOC_MODER &= ~(0b1111 << 12);
	*GPIOC_MODER |= (0b1010 << 12);
	*GPIOC_AFRL &= ~(0xff << 24);
	*GPIOC_AFRL = (0b1000 << 24) | (0b1000 << 28);

	__HAL_RCC_USART6_CLK_ENABLE();
	uint32_t* USART6_CR1 = (uint32_t*) (USART6_BASE + 0x0C);
	uint32_t* USART6_BRR = (uint32_t*) (USART6_BASE + 0x08);
	*USART6_CR1 &= ~(0b1 << 12); // set word length = 8 data bits
	*USART6_CR1 &= ~(0b1 << 10); // disable parity control
	*USART6_CR1 |= (0b11 << 2);  // enable transmitter and receiver
	*USART6_BRR = (104 << 4) | (3 << 0); // set baud rate = 9600bps
	*USART6_CR1 |= (0b1 << 13);  // enable USART
}
