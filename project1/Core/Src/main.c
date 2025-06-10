#include "main.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#define USART1_BASE_ADDR 0x40011000
#define GPIOB_BASE_ADDR 0x40020400
#define GPIOD_BASE_ADDR 0x40020C00

typedef enum{
	GREEN, ORANGE, RED, BLUE
} color_t;

typedef enum{
	OFF, ON
} state_t;

char str[20] = {0};
int _index = 0;
char* c[4] = { "Green", "Orange", "Red", "Blue" };
char* s[2] = { "OFF", "ON" };

void USART_Init();
void USART_send_char(char data);
void USART_send_string(char* str, ...);
void LED_Init();
void ctrl_LED(color_t color, state_t state);
color_t get_color();
state_t get_state();
void clear_string();
char is_done();
void USART1_IRQHandler();

int main()
{
	HAL_Init();
	USART_Init();
	LED_Init();

	while(1)
	{
		if (is_done())
		{
			color_t color = get_color();
			state_t state = get_state();
			ctrl_LED(color, state);
			USART_send_string("%s led is %s\n", c[color], s[state]);
			clear_string();
		}
	}
	return 0;
}

void USART1_IRQHandler()
{
	uint32_t* USART1_DR = (uint32_t*) (USART1_BASE_ADDR + 0x04);
	str[_index++] = *USART1_DR;
}

color_t get_color()
{
	if (strstr(str, "green") != NULL) { return GREEN;}
	else if (strstr(str, "orange") != NULL) { return ORANGE; }
	else if (strstr(str, "red") != NULL) { return RED; }
	else {return BLUE;}
}

state_t get_state()
{
	if (strstr(str, "on") != NULL) { return ON; }
	else { return OFF; }
}

void clear_string()
{
	memset(str, 0, 20);
	_index = 0;
}

char is_done()
{
	for (int i = 0; i < 20; i++)
	{
		if (str[i] == '\n') { return 1; }
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

void USART_send_string(char* str, ...)
{
	va_list list;
	va_start(list, str);
	char print_buf[128] = {0};
	vsprintf(print_buf, str, list);
	int size = strlen(print_buf);
	for (int i = 0; i < size; i++)
	{
		USART_send_char(print_buf[i]);
	}
	va_end(list);
}

void ctrl_LED(color_t color, state_t state)
{
	uint32_t* GPIOD_ODR = (uint32_t*) (GPIOD_BASE_ADDR + 0x14);
	 if (state == ON)
	 {
		 *GPIOD_ODR |= (0b1 << (12 + color));
	 }
	 else
	 {
		 *GPIOD_ODR &= ~(0b1 << (12 + color));
	 }
}

void LED_Init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER = (uint32_t*) (GPIOD_BASE_ADDR + 0x00);
	*GPIOD_MODER &= ~(0xff << 24);		// clear pin PD12, PD13, PD14, PD15
	*GPIOD_MODER |= (0b01010101 << 24);	// set PD12, PD13, PD14, PD15 as OUTPUT
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
