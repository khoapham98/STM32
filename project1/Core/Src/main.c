#include "main.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

void UART_Init();
void button_Init();
char isPressed();
void USART_send(uint8_t data);
void USART_send_string(char* str, ...);
void DHT11_Init();
void request();
void response();
uint8_t receive_data();

uint8_t c;
int main()
{
	HAL_Init();
	UART_Init();
	button_Init();
	DHT11_Init();
	char data[5];
	while (1)
	{
		// gui xung start
		request();
		// doi xung phan hoi
		response();
		// doc 40 bit du lieu;
		for (int i = 0; i < 5; i++)
		{
			data[i] = receive_data();
		}
		// kiem tra loi
		if ((data[0] + data[1] + data[2] + data[3]) == data[4])
		{
			USART_send_string("ERROR!\n");
		}
		else
		{
			uint16_t* read = &data[0];
			USART_send_string("Humidity: %hu, ", *read);
			read = &data[2];
			USART_send_string("Temperature: %hu\n", *read);
		}
		HAL_Delay(1000);
	}
	return 0;
}

uint8_t receive_data()
{
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE + 0x00);
	*GPIOA_MODER &= ~(0b11 << 18);	// set pin PA9 as INPUT

	uint32_t* GPIOA_IDR = (uint32_t*) (GPIOA_BASE + 0x10);
	for (int i = 0; i < 8; i++) {
		while (((*GPIOA_IDR >> 9) & 1) == 0);
		HAL_Delay(30);
		if (((*GPIOA_IDR >> 9) & 1) == 1)
		{
			c = (c << 1) | 0x01;
		}
		else
		{
			c = c << 1;
		}
		while (((*GPIOA_IDR >> 9) & 1) == 1);
	}
	return c;
}

void response()
{
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE + 0x00);
	*GPIOA_MODER &= ~(0b11 << 18);	// set pin PA9 as INPUT

	uint32_t* GPIOA_IDR = (uint32_t*) (GPIOA_BASE + 0x10);
	while (((*GPIOA_IDR >> 9) & 1) == 1);
	while (((*GPIOA_IDR >> 9) & 1) == 0);
	while (((*GPIOA_IDR >> 9) & 1) == 1);
}

void request()
{
	uint32_t* GPIOA_ODR = (uint32_t*) (GPIOA_BASE + 0x14);
	*GPIOA_ODR |= (0b1 << 9);
	*GPIOA_ODR &= ~(0b1 << 9);
	HAL_Delay(20);
	*GPIOA_ODR |= (0b1 << 9);

}

void DHT11_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t* GPIOA_MODER = (uint32_t*) (GPIOA_BASE + 0x00);
	*GPIOA_MODER &= ~(0b11 << 18);
	*GPIOA_MODER |= (0b01 << 18);	// set pin PA9 as OUTPUT
}

void USART_send_string(char* str, ...)
{
	va_list list;
	va_start(list, str);
	char print_buf[128] = { 0 };
	vsprintf(print_buf, str, list);
	int len = strlen(print_buf);
	for(int i = 0; i < len; i++)
	{
		USART_send(print_buf[i]);
	}
	va_end(list);
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
